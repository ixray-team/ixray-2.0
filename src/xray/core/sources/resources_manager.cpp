#include "pch.h"

#include "resources_manager.h"
#include "resources_device_manager.h"
#include "resources_hdd_manager.h"
#include "resources_macros.h"
#include "resources_allocators.h"
#include "resources_callbacks.h"
#include "resources_fs_impl.h"
#include "fs_watcher.h"

#include <xray/resources_resource_link.h>
#include <xray/resources_queries_result.h>

#include <xray/os_preinclude.h>
#define _WIN32_WINNT 500
#include <xray/os_include.h>

namespace xray {
namespace resources {

using namespace		fs;

uninitialized_reference<resources_manager>		g_resources_manager;
uninitialized_reference<ppmd_compressor>		g_resource_compressor;

//----------------------------------------------------------
// initialization / deinitialization
//----------------------------------------------------------

resources_manager::resources_manager ()
{
	XRAY_CONSTRUCT_REFERENCE				(m_compressor, ppmd_compressor) (&memory::g_cook_allocator, 1);
	m_tls_key_thread_local_data			=	threading::tls_create_key();

	m_resources_thread_exit				=	false;
	m_resources_thread_exited			=	false;
	m_cooker_thread_exit				=	false;
	m_cooker_thread_exited				=	false;
	m_do_mount_mounts_path				=	false;

	m_dispatching_delay_delete_unmanaged_resources_flag	=	0;
	m_num_delay_delete_unmanaged_resources	=	0;

	device_manager * const hdd_manager		=	RES_NEW(class hdd_manager)("resources HDD", 512);
	m_device_managers.push_back					(hdd_manager);

	m_num_unmanaged_resources				=	0;
	m_count_of_pending_mount_operations		=	0;
	m_num_current_fs_ops					=	0;
	m_pending_queries_count					=	0;
	m_num_cook_registrators					=	0;
	m_count_of_pending_query_with_fat_it	=	0;
	m_count_of_pending_helper_query_for_mount	=	0;
	m_last_unique_id_for_queries			=	0;

	for ( u32 i=0; i<last_resource_class; ++i )
		m_cooks_registry.push_back			(NULL);

	if ( !threading::g_debug_single_thread )
	{
		m_resources_thread_id	=	threading::spawn	(boost::bind(&resources_manager::resources_thread_proc, this), 
														 "resources manager", 
														 "res-man", 
														 0,
														 2 % threading::core_count(),
														 threading::tasks_unaware);

		m_cooker_thread_id		=	threading::spawn	(boost::bind(&resources_manager::cooker_thread_proc, this), 
														 "resources cooker",
														 "res-cook", 
														 0,
														 2 % threading::core_count(),
														 threading::tasks_unaware);
	}
	else
	{
		m_resources_thread_id			=	threading::current_thread_id();
		m_cooker_thread_id				=	threading::current_thread_id();
	}

	register_cooks							();
}

resources_manager::~resources_manager ()
{
	if ( m_num_unmanaged_resources )
	{
		LOGI_ERROR							("resources",
											 "%d unmanaged resource(s) or user buffers LEAKED!!!", 
											 m_num_unmanaged_resources);
		
		unmanaged_resource * it_resource	=	m_unmanaged_resources.pop_all_and_clear();
		while ( it_resource )
		{
			LOGI_ERROR						("resources", "LEAKED: %s", it_resource->log_string().c_str());
			it_resource					=	m_unmanaged_resources.get_next_of_object(it_resource);
		}
	}

	if ( !threading::g_debug_single_thread )
	{
		m_resources_thread_exit			=	true;
		wakeup_resources_thread				();
		while ( !m_resources_thread_exited )
			threading::yield				(1);

		m_cooker_thread_exit			=	true;
	
		wakeup_cooker_thread				();
		while ( !m_cooker_thread_exited )
			threading::yield				(1);

		memory::g_resources_helper_allocator.user_current_thread_id( );
		memory::g_resources_unmanaged_allocator.user_current_thread_id( );
	}

	for ( thread_local_data_container::iterator	it	=	m_thread_local_data.begin();
												it	!=	m_thread_local_data.end();
												++it )
	{
		thread_local_data * const tls	=	(*it).second;
		tls->delete_this					();
	}

	m_thread_local_data.clear				();
	
	for ( device_managers::iterator		it	=	m_device_managers.begin();
										it	!=	m_device_managers.end();
										++it )
	{
		device_manager*	 manager		=	*it;
		RES_DELETE							(manager);
	}

	fs::set_allocator_thread_id				(threading::current_thread_id());

	XRAY_DESTROY_REFERENCE					(m_compressor);

	threading::tls_delete_key				(m_tls_key_thread_local_data);
}

//----------------------------------------------------------
// general functions
//----------------------------------------------------------

void   resources_manager::push_delayed_delete_resource (managed_resource * res)
{
	m_delayed_delete_resources.push_back	(res);
	wakeup_resources_thread					();
}

void   resources_manager::push_delayed_delete_unmanaged_resource (unmanaged_resource * resource)
{
#ifndef MASTER_GOLD
	register_delay_delete_unmanaged			(resource);	
#endif // #ifndef MASTER_GOLD

	threading::interlocked_increment		(m_num_delay_delete_unmanaged_resources);
	m_delayed_delete_unmanaged_resources.push_back (resource);
	wakeup_resources_thread					();
}

void   resources_manager::delete_delayed_resources ()
{
	managed_resource * it_dying_resource =	m_delayed_delete_resources.pop_all_and_clear();
	while ( it_dying_resource )
	{
		managed_resource * const next_it =	it_dying_resource->get_next_delay_delete();

		free_managed_resource				(it_dying_resource);
		
		it_dying_resource				=	next_it;
	}
}

void   resources_manager::dispatch_delete_delayed_unmanaged_resources ()
{
	threading::interlocked_exchange			(m_dispatching_delay_delete_unmanaged_resources_flag, 1);

	while ( !m_delayed_delete_unmanaged_resources.empty() )
	{
		// caution: we dont pop unmanaged_resource yet, so m_delayed_delete_unmanaged_resources
		// stays non-empty while unmanaged_resource is not transfered to thread_local_data
		// this is needed for finalize_thread_usage to work correctly
		unmanaged_resource * dying_resource	=	m_delayed_delete_unmanaged_resources.front();
		R_ASSERT							(dying_resource->get_class_id() != raw_data_class);
		cook_base * const cook			=	find_cook(dying_resource->get_class_id());
		R_ASSERT_U							(cook);
		R_ASSERT							(dying_resource->get_deleter_object() == cook);

		u32 const deallocate_thread_id	=	dying_resource->deallocate_thread_id();
		if ( deallocate_thread_id == threading::current_thread_id() )
		{
			m_delayed_delete_unmanaged_resources.pop_front();
			delete_delayed_unmanaged_resource	(dying_resource);
			continue;
		}

		thread_local_data *	thread_data	=	get_thread_local_data	(deallocate_thread_id, true);
		m_delayed_delete_unmanaged_resources.pop_front();

		thread_data->delayed_delete_unmanaged_resources.push_back	(dying_resource);

		if ( deallocate_thread_id == m_cooker_thread_id )
			wakeup_cooker_thread			();
	}

	threading::interlocked_exchange			(m_dispatching_delay_delete_unmanaged_resources_flag, 0);
}

void   resources_manager::execute_fs_task (fs_task * task)
{
	bool const is_mount_task		=	task->is_mount_task();
	if ( is_mount_task )
	{
		while ( m_num_delay_delete_unmanaged_resources )
		{
			dispatch_delete_delayed_unmanaged_resources	();

			if ( threading::g_debug_single_thread )
				resources::dispatch_callbacks	();
		}

		get_execute_task_mutex().lock	();
	}

	task->result					=	task->execute	();

#pragma message (XRAY_TODO("fix it"))
	if ( !task->mount_callback && !task->fs_iterator_callback )
	{
		if ( task->allocator == & memory::g_mt_allocator )
			XRAY_DELETE_IMPL				(task->allocator, task);
	}
	else
	{
		thread_local_data * const local_data	=	get_thread_local_data(task->thread_id, true);
		local_data->ready_fs_tasks.push_back	(task);

		if ( watcher_enabled() && task->thread_id == fs::watcher_thread_id() )
			fs::wakeup_watcher_thread	();
	}

	if ( is_mount_task )
	{
		get_execute_task_mutex().unlock	();
		change_count_of_pending_mount_operations(-1);
	}
}

void   resources_manager::execute_fs_tasks (fs_task * task)
{
	if ( !task )
		return;

	if ( m_count_of_pending_mount_operations )
	{
		while ( m_num_current_fs_ops )
		{
			;	// wait until all concurrent fs operations are ended
		}
	}

	while ( task )
	{
		fs_task * const next_task		=	task->next;

		if ( task->type == fs_task::type_mount_composite )
		{
			fs_task * const children_tasks	=	task->children.pop_all_and_clear();
			execute_fs_tasks				(children_tasks);
		}

		execute_fs_task						(task);

		task							=	next_task;
	}
}

//----------------------------------------------------------
// main functions
//----------------------------------------------------------

void   resources_manager::resources_tick_impl ()
{
	if ( m_do_mount_mounts_path )
	{
		do_mount_mounts_path				();
		m_do_mount_mounts_path			=	false;
	}

	m_out_of_memory_functionality.tick		();

	bool const do_init_new_queries_with_unlocked_fat_it	=	!m_count_of_pending_mount_operations && !m_count_of_pending_helper_query_for_mount;
	
	bool const has_fs_tasks				=	!m_fs_tasks.empty();
	bool const do_execute_fs_tasks		=	!m_count_of_pending_query_with_fat_it && !m_count_of_pending_helper_query_for_mount;

	query_result * queries_with_unlocked_fat_it		=	do_init_new_queries_with_unlocked_fat_it ? 
														m_new_queries_with_unlocked_fat_it.pop_all_and_clear() : NULL;
	query_result * const queries_with_locked_fat_it	=	m_new_queries_with_locked_fat_it.pop_all_and_clear();

	query_result * const new_new_helper_queries_for_mount_list	=	m_new_queries_for_mount.pop_all_and_clear();

	fs_task * const fs_tasks			=	do_execute_fs_tasks ? m_fs_tasks.pop_all_and_clear() : NULL;
	
	delete_delayed_resources				();

	dispatch_delete_delayed_unmanaged_resources	();

	if ( do_execute_fs_tasks )
		execute_fs_tasks 					(fs_tasks);

	if ( do_init_new_queries_with_unlocked_fat_it && !m_new_queries_waiting_for_cook_register.empty() && !cooks_are_registering() )
	{
		query_result * const new_queries_waiting_for_cook_register	=	m_new_queries_waiting_for_cook_register.pop_all_and_clear();
		if ( !queries_with_unlocked_fat_it )
			queries_with_unlocked_fat_it				=	new_queries_waiting_for_cook_register;
		else
			new_queries_list::set_next_of_object	(queries_with_unlocked_fat_it, new_queries_waiting_for_cook_register);
	}

	init_new_queries						(new_new_helper_queries_for_mount_list);

	init_new_queries						(queries_with_locked_fat_it);

	if ( do_init_new_queries_with_unlocked_fat_it )
		init_new_queries					(queries_with_unlocked_fat_it);

	dispatch_callbacks						(false);

	m_allocate_resource_functionality.tick	(false); // must be before dispatch_allocated_raw_resources to work synchronously
	dispatch_allocated_raw_resources		();

	std::for_each							(m_device_managers.begin(), 
											 m_device_managers.end(), 
											 std::mem_fun(& device_manager::update));
	
	dispatch_created_resources				();
	dispatch_decompressed_resources			();

	save_generated_resources				();

	delete_name_registry_entries			();

	dispatch_query_finished_callbacks			();

	if ( m_flush_timer.get_elapsed_ms() >= 1000 )
	{
		fs::flush_replications				();
		m_flush_timer.start					();
	}

	// if one or both functionalities where turned off - continue in next tick
	if ( !do_init_new_queries_with_unlocked_fat_it || (!do_execute_fs_tasks && has_fs_tasks) )
		wakeup_resources_thread				();
}

void   resources_manager::resources_thread_proc ()
{
	fs::set_allocator_thread_id				(threading::current_thread_id());
	ASSERT									(!threading::g_debug_single_thread);
	memory::g_resources_helper_allocator.user_current_thread_id();
	memory::g_resources_unmanaged_allocator.user_current_thread_id();

	m_flush_timer.start						();
	while ( !m_resources_thread_exit )
	{
		m_resources_wakeup_event.wait		(threading::event::wait_time_infinite);
		resources_tick_impl					();
	}

	fs::flush_replications					();
	m_resources_thread_exited			=	true;
}

void   resources_manager::wait_and_dispatch_callbacks (bool const call_from_main_thread, bool finalizing_thread)
{
	if ( !call_from_main_thread && threading::core_count() == 1 ) 
		return;

	while ( !m_wait_and_dispatch_callbacks_mutex.try_lock() )
	{
		if ( threading::g_debug_single_thread )
			resources::tick						();

		dispatch_callbacks						(true);
	}
	
	while ( !thread_can_exit() )
	{
		if ( threading::g_debug_single_thread )
			resources::tick					();

		dispatch_callbacks					(finalizing_thread);
	}

	m_wait_and_dispatch_callbacks_mutex.unlock	();
}


} // namespace resources
} // namespace xray
