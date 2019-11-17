////////////////////////////////////////////////////////////////////////////
//	Created		: 14.06.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_manager.h"
#include "resources_device_manager.h"
#include "resources_callbacks.h"
#include "fs_file_system.h"

#include "configs_binary_config_cook.h"
#include "configs_lua_config_cook.h"
#include "configs_ltx_config_cook.h"

namespace xray {
namespace resources {

using namespace fs;

void   resources_manager::register_cooks ()
{
	get_binary_config_cook_impl				();
	get_binary_config_cook					();

	static core::configs::ltx_config_cook	ltx_cook;
	register_cook							(& ltx_cook);

	get_lua_config_cook						();
}

void   resources_manager::do_mount_mounts_path ()
{
	fs::g_fat->mount_disk						("mounts", m_mounts_path.c_str());
	m_mounts_initialized_event.set			(true);
}

xray::core::configs::lua_config_cook& resources_manager::get_lua_config_cook	()
{
	static xray::core::configs::lua_config_cook	cook;
	static bool initialized					= false;
	if ( !initialized )
	{
		initialized							= true;
		register_cook						(&cook);
	}

	return									cook;
}

xray::core::configs::binary_config_cook & resources_manager::get_binary_config_cook	()
{
	static xray::core::configs::binary_config_cook	cook;
	static bool initialized					= false;
	if ( !initialized ) 
	{
		initialized							= true;
		register_cook						(&cook);
	}

	return									cook;
}

xray::core::configs::binary_config_cook_impl & resources_manager::get_binary_config_cook_impl	()
{
	static xray::core::configs::binary_config_cook_impl	cook;
	static bool initialized					= false;
	if ( !initialized ) 
	{
		initialized							= true;
		register_cook						(&cook);
	}

	return									cook;
}

threading::event &   resources_manager::get_resources_wakeup_event () 
{ 
	return									m_resources_wakeup_event; 
}

threading::mutex &   resources_manager::get_execute_task_mutex () 
{ 
	return									m_execute_task_mutex; 
}

threading::mutex &   resources_manager::get_mount_transaction_mutex () 
{ 
	return									m_mount_transaction_mutex; 
}

bool   resources_manager::need_replication (pcstr const file_path)
{
	XRAY_UNREFERENCED_PARAMETER				(file_path);
	//^ if ( is_hdd path ... )
	return									false;
}

device_manager *   resources_manager::find_capable_device_manager (pcstr file_path)
{
	for ( device_managers::iterator		it	=	m_device_managers.begin();
										it	!=	m_device_managers.end();
										++it )
	{
		device_manager*	manager			=	*it;
		if ( manager->can_handle_query		(file_path) )
			return							manager;
	}

	return									NULL;
}

void   resources_manager::add_fs_task (fs_task * task)
{
	m_fs_tasks.push_back					(task);
	wakeup_resources_thread					();
}

void   resources_manager::finalize_thread_usage (bool const call_from_main_thread)
{
	this->wait_and_dispatch_callbacks			(call_from_main_thread, true);
}

void   resources_manager::unregister_unmanaged_resource (unmanaged_resource* const res)
{
	m_unmanaged_resources.erase					(res);
	threading::interlocked_decrement			(m_num_unmanaged_resources);
}

void   resources_manager::register_unmanaged_resource (unmanaged_resource* const res)
{
	m_unmanaged_resources.push_back				(res);
	threading::interlocked_increment			(m_num_unmanaged_resources);
}

void   resources_manager::wakeup_resources_thread ()
{
	m_resources_wakeup_event.set				(true);
}

void   resources_manager::wakeup_cooker_thread ()
{
	m_cooker_wakeup_event.set					(true);
}

void   resources_manager::push_generated_resource_to_save (query_result * in_query)
{
	R_ASSERT										(in_query->has_flag(query_result::flag_need_saving_of_generated_resource));
	in_query->unset_flag							(query_result::flag_need_saving_of_generated_resource);
	m_generated_resources_to_save_list.push_back	(in_query);
	wakeup_resources_thread							();
}

void   resources_manager::save_generated_resource (query_result * in_query)
{
	path_string		disk_path;
	bool const selected_disk_path		=	in_query->select_disk_path_from_request_path(& disk_path);
	device_manager * const manager		=	selected_disk_path ? 
											find_capable_device_manager(disk_path.c_str()) : NULL;
	if ( !manager )
	{
		LOGI_WARNING						("resources:resource", "cannot save generated resource", detail::make_query_logging_string(in_query).c_str());
		in_query->finish_query				(in_query->get_create_resource_result());
		return;
	}

	mutable_buffer generated_data		=	in_query->grab_generated_data_to_save();

	query_result * const save_query		=	RES_NEW(query_result)(0, NULL, 0, 0);
	save_query->init_save					(in_query->get_requested_path(), 
											 generated_data, 
											 in_query, 
											 0);
	
	manager->push_query						(save_query);
	wakeup_resources_thread					();
}

void   resources_manager::save_generated_resources ()
{
	for ( query_result *	it_query	=	m_generated_resources_to_save_list.pop_all_and_clear();
							it_query;
							it_query	=	m_generated_resources_to_save_list.get_next_of_object(it_query) )
	{
		save_generated_resource				(it_query);
	}
}

void   resources_manager::change_count_of_pending_query_with_fat_it (long change)
{
	R_ASSERT								(change == +1 || change == -1);
	if ( change == -1 )
		R_ASSERT							(m_count_of_pending_query_with_fat_it > 0);
	threading::interlocked_exchange_add		(m_count_of_pending_query_with_fat_it, change);
}

void   resources_manager::change_count_of_pending_helper_query_for_mount (long change)
{
	R_ASSERT								(change == +1 || change == -1);
	if ( change == -1 )
		R_ASSERT							(m_count_of_pending_helper_query_for_mount > 0);
	threading::interlocked_exchange_add		(m_count_of_pending_helper_query_for_mount, change);
}

void   resources_manager::change_count_of_pending_mount_operations (long change)
{
	threading::mutex_raii	raii			(m_mutex_mount_task_add);
	R_ASSERT								(change == +1 || change == -1);
	if ( change == -1 )
		R_ASSERT							(m_count_of_pending_mount_operations > 0);
	threading::interlocked_exchange_add		(m_count_of_pending_mount_operations, change);
}

void   resources_manager::push_name_registry_to_delete (name_registry_entry * entry)
{ 
	m_name_registry.erase					(entry);
	m_name_registry_delete_queue.push_back	(entry);
	wakeup_resources_thread					();
}

void   resources_manager::delete_name_registry_entries ()
{
	name_registry_entry * it_entry		=	m_name_registry_delete_queue.pop_all_and_clear();
	while ( it_entry )
	{
		name_registry_entry * it_next	=	m_name_registry_delete_queue.get_next_of_object(it_entry);

		RES_FREE							(it_entry);

		it_entry						=	it_next;
	}
}

void   resources_manager::push_to_call_query_finished_callback (resource_base * resource)
{
	if ( m_query_finished_callback )
	{
		resource->cast_base_of_intrusive_base()->pin_reference_count_for_query_finished_callback();
		m_query_finished_callback_list.push_back	(resource);
		wakeup_resources_thread					();
	}
}

void   resources_manager::dispatch_query_finished_callbacks ()
{
	resource_base * it_resource				=	m_query_finished_callback_list.pop_all_and_clear();
	if ( it_resource )
		ASSERT									(m_query_finished_callback);
	while ( it_resource )
	{
		resource_base * const next			=	m_query_finished_callback_list.get_next_of_object(it_resource);

		m_query_finished_callback					(it_resource);

		it_resource->cast_base_of_intrusive_base()->unpin_reference_count_for_query_finished_callback();

		it_resource							=	next;
	}
}

thread_local_data*   resources_manager::get_thread_local_data (u32 const thread_id, bool create_if_not_exist)
{
	thread_local_data *	local_data		=	NULL;
	
	if ( threading::current_thread_id() == thread_id )
	{
		local_data						=	(thread_local_data *)threading::tls_get_value(m_tls_key_thread_local_data);
		if ( local_data )
			return							local_data;
	}

	threading::mutex_raii	raii			(m_thread_local_data_mutex);
	thread_local_data_container::iterator it	=	m_thread_local_data.find(thread_id);

	if ( it != m_thread_local_data.end() )
	{
		local_data						=	it->second;
	}
	else
	{
		if ( create_if_not_exist )
		{
			if ( threading::current_thread_id() == m_resources_thread_id )
				local_data					=	RES_NEW(thread_local_data)(& memory::g_resources_helper_allocator);
			else
				local_data					=	MT_NEW(thread_local_data)(& memory::g_mt_allocator);

			m_thread_local_data.insert		( std::make_pair(thread_id, local_data) );
		}
	}

	if ( threading::current_thread_id() == thread_id )
		threading::tls_set_value			(m_tls_key_thread_local_data, local_data);

	return									local_data;
}

void   resources_manager::on_query_finished (queries_result * query)
{
	thread_local_data *	local_data		=	get_thread_local_data(query->m_thread_id, true);
	local_data->finished_queries.push_back	(query);

	if ( query->m_thread_id == m_cooker_thread_id )
		wakeup_cooker_thread				();
	if ( query->m_thread_id == m_resources_thread_id )
		wakeup_resources_thread				();
}

//----------------------------------------------------------
// replications
//----------------------------------------------------------

bool   resources_manager::replicate_resource (fat_iterator			fat_it, 
											  managed_resource_ptr	resource,
											  query_callback		callback, 
											  int const				priority)
{
	XRAY_UNREFERENCED_PARAMETER				(callback);
	path_string								file_path, replicate_path;
	g_fat->get_disk_path					(fat_it, file_path);
	g_fat->replicate_path					(file_path.c_str(), replicate_path);

	device_manager*	manager				=	find_capable_device_manager(replicate_path.c_str());
	R_ASSERT								(manager);
	if ( !manager )
		return								false;

	// note: data must live till store_callback is called
	query_result* const	 replication_query	=	RES_NEW(query_result)(0, NULL, 0, 0);

	replication_query->init_replication		(fat_it_to_wrapper(fat_it), 
											 resource, 
											 priority);
	
	manager->push_query						(replication_query);
	wakeup_resources_thread					();
	return									true;
}

} // namespace resources
} // namespace xray


