////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_manager.h"
#include "resources_device_manager.h"
#include "resources_hdd_manager.h"
#include "resources_macros.h"
#include "resources_managed_allocator.h"
#include "resources_callbacks.h"
#include "resources_fs_impl.h"
#include <xray/resources_queries_result.h>

#include <xray/os_preinclude.h>
#define _WIN32_WINNT 500
#include <xray/os_include.h>

namespace xray {
namespace resources {

using namespace		fs;

void   resources_manager::start_query_transaction ()
{
	thread_local_data *	const local_data	=	get_thread_local_data(threading::current_thread_id(), true);
	ASSERT										(!local_data->in_transaction);

	local_data->in_transaction				=	true;
}

void   resources_manager::end_query_transaction ()
{
	thread_local_data *	const local_data	=	get_thread_local_data(threading::current_thread_id(), false);
	ASSERT										(local_data && local_data->in_transaction);
	local_data->in_transaction				=	false;

	query_result * to_init					=	local_data->to_init_by_transaction.pop_all_and_clear();
	while ( to_init )
	{
		query_result * const next			=	local_data->to_init_by_transaction.get_next_of_object(to_init);
		push_new_query							(to_init, query_type_normal);

		to_init								=	next;
	}

	wakeup_resources_thread						();
}

long   resources_manager::query_resources_impl   (request   				const requests[],
												  creation_request			const requests_create[],
										  		  u32						const requests_count,
										  		  query_callback const &		  callback,
										  		  memory::base_allocator *	const allocator,
												  user_data_variant const *	const user_data[],
										  		  int						const priority,
												  query_result_for_cook *	const parent,
												  query_type_enum			const query_type)
{
	R_ASSERT								(requests || requests_create || !requests_count);
	u32* strings_lengths_with_zero		=	(u32 *)ALLOCA(sizeof(u32) * requests_count);
	u32	 additional_strings_length		=	0;
	u32  user_data_variant_count		=	0;
	for ( u32 i=0; i<requests_count; ++i )
	{
		if ( user_data && user_data[i] )
			++user_data_variant_count;

		class_id const request_class_id		=	requests ? requests[i].id : requests_create[i].id;
		if ( request_class_id != raw_data_class && !cook_base::find_cook(request_class_id) )
		{
			R_ASSERT						(g_resources_manager->cooks_are_registering(),
											 "Omg! Cook is not registered for resource_id: %d", request_class_id);
		}

		pcstr request_path				=	requests_create ? requests_create[i].name : requests[i].path;
		if ( !request_path )
			request_path				=	"";

		bool allocate_max_string_length	=	false;

		if ( g_resources_manager->cooks_are_registering() )
			allocate_max_string_length	=	true;
		else if ( requests && requests[i].id != raw_data_class )
			allocate_max_string_length	=	cook_base::does_create_resource_if_no_file(requests[i].id);

		if ( allocate_max_string_length )
			strings_lengths_with_zero[i] =	math::max(fs::max_path_length, strings::length(request_path) + 1);
		else
			strings_lengths_with_zero[i] =	strings::length(request_path) + 1;

		if ( strings_lengths_with_zero[i] > array_size(((query_result *)NULL)->m_request_path_default_storage) )
			additional_strings_length	+=	strings_lengths_with_zero[i];
	}

	u32 const queries_size				=	sizeof(queries_result) + (sizeof(query_result) * requests_count);
	u32 const user_data_size			=	(sizeof(user_data_variant) * user_data_variant_count);
	u32 const full_allocation_size		=	queries_size + user_data_size + additional_strings_length;
	
	queries_result * const queries		=	(queries_result *)
											XRAY_ALLOC_IMPL(* allocator, 
															char, 
															full_allocation_size);

	u32 const user_thread_id			=	threading::current_thread_id();
	new ( queries )							queries_result((u32)requests_count, 
															callback, 
															allocator, 
															user_thread_id,
															parent);

	long const out_unique_id			=	queries->unique_id();

	user_data_variant *	cur_user_data_pos	=	(user_data_variant *)((pstr)queries + queries_size);
	pstr	cur_string_pos				=	(pstr)queries + queries_size + user_data_size;
	u32     space_left					=	additional_strings_length;
	for ( u32 i=0; i<requests_count; ++i )
	{
		query_result & query			=	queries->at(i);
		if ( user_data && user_data[i] )
		{
			query.m_user_data			=	cur_user_data_pos;
			new (query.m_user_data)			user_data_variant;
			* query.m_user_data			=	* user_data[i];
			++cur_user_data_pos;
		}
		
		query.m_class_id				=	requests ? requests[i].id : requests_create[i].id;

		pcstr request_path				=	requests_create ? requests_create[i].name : requests[i].path;
		if ( !request_path )
			request_path				=	"";

		bool const request_path_fits_default_storage =	strings::length(request_path) < array_size(query.m_request_path_default_storage);

		query.m_request_path			=	request_path_fits_default_storage ? query.m_request_path_default_storage : cur_string_pos;
		query.m_request_path_max_size	=	request_path_fits_default_storage ? array_size(query.m_request_path_default_storage) : strings_lengths_with_zero[i];
		query.m_priority				=	priority;

		if ( requests_create && requests_create[i].data )
			query.m_creation_data_from_user	=	requests_create[i].data;

		memory::copy						(query.m_request_path, 
											 request_path_fits_default_storage ? query.m_request_path_max_size : space_left, 
											 request_path, 
											 strings_lengths_with_zero[i]);

		if ( !request_path_fits_default_storage )
		{
			space_left					-=	strings_lengths_with_zero[i];
			cur_string_pos				+=	strings_lengths_with_zero[i];
		}
	}

	inc_num_current_fs_ops					();
	bool all_resources_cached			=	true;

	XRAY_MEMORY_BARRIER_FULL				();

	queries->translate_request_paths		();
	queries->translate_query_if_needed		();
	
	bool locked_mutex_mount_task_add	=	false;
	if ( has_no_pending_mount_ops() && !g_resources_manager->cooks_are_registering() )
	{
		locked_mutex_mount_task_add		=	true;
		g_resources_manager->mutex_mount_task_add ().lock();
	}

	if ( locked_mutex_mount_task_add && has_no_pending_mount_ops() )
	{
		for ( u32 i=0; i<requests_count; ++i )
		{
			query_result & query		=	queries->at(i);

			if ( query.is_translate_query() )
			{
				if ( !query.has_flag(query_result::flag_finished) || 
					 !query.has_flag(query_result::flag_reused_resource) )
				{
					all_resources_cached =	false;
				}
				continue;
			}
			else if ( query.consider_with_name_registry(query_result::only_try_to_get_associated_resource_true) == 
					  query_result::consider_with_name_registry_result_got_associated_resource )
			{
				query.on_query_end			(query_result::push_to_ready_queries_false);
				continue;
			}

			if ( query.creation_data_from_user() )
			{
				all_resources_cached	=	false;
				continue;
			}

			query.process_request_path		();

			if ( !query.check_fat_for_resource_reusage () )
			{
				query.allocate_raw_unmanaged_resource_if_needed();

				if ( !query.try_synchronous_cook_from_inline_data() )
					all_resources_cached	=	false;
			}
		}
	}
	else
	{
		all_resources_cached			=	false;
	}

	dec_num_current_fs_ops					();

	if ( all_resources_cached )
	{
		// SYNC WAY
		R_ASSERT							(queries->is_successful() || !queries->size());
		queries->set_result					(true);
		callback							(* queries);
		queries->~queries_result			();
		queries_result const * queries_non_const_ptr	=	const_cast<queries_result *>(queries);
		XRAY_FREE_IMPL						(* allocator, queries_non_const_ptr);
	}
	else
	{
		thread_local_data *	const local_data	=	get_thread_local_data(threading::current_thread_id(), false);
		bool const in_transaction				=	local_data && local_data->in_transaction && (query_type == query_type_normal);
		
		// ASYNC WAY
		for ( u32 i=0; i<requests_count; ++i )
		{
			query_result & query		=	queries->at(i);
			if ( query.has_flag(query_result::flag_finished) )
				continue;					// skip, it was finished by try_init_from_inline_data
			if ( query.has_flag(query_result::flag_translated_query) )
				continue;					// already worked synchronously
			if ( query.has_flag(query_result::flag_is_referer) )
				continue;					// query became a referer, skip
			
			if ( in_transaction )
				local_data->to_init_by_transaction.push_back	(& query);
			else
				push_new_query				(& query, query_type);
		}

		if ( !in_transaction )
			wakeup_resources_thread			();
	}

	if ( locked_mutex_mount_task_add )
		g_resources_manager->mutex_mount_task_add ().unlock();

	return									out_unique_id;
}
class query_resources_helper
{
public:
	query_resources_helper (pcstr					const dir_path, 
							pcstr					const mask, 
							class_id				const class_id, 
							query_callback const&		  callback, 
							memory::base_allocator*	const allocator,
							query_flag_enum			const flags, 
							int						const priority,
							query_result_for_cook*	const parent) 
							: m_dir_path		(dir_path), 
							  m_mask			(mask), 
							  m_class_id		(class_id), 
							  m_user_callback	(callback), 
							  m_allocator		(allocator), 
							  m_flags			(flags),
							  m_priority		(priority),
							  m_parent_query	(parent)
	{
		query_fs_iterator	(dir_path, 
							 boost::bind(& query_resources_helper::on_fs_iterator_ready, this, _1),
						  	 m_allocator);
	}

private:
	void   add_resources_from_folder (fs_iterator						iterator, 
									  buffer_vector<fs::path_string>&	request_pathes,
									  buffer_vector<request>&			requests)
	{
		for ( fs_iterator	child_it	=	iterator.children_begin();
							child_it	!=	iterator.children_end();
						  ++child_it )
		{
			if ( !child_it.is_folder() )
			{
				pcstr const name			=	child_it.get_name();
				if ( strings::compare_with_wildcards(m_mask, name) )
				{
					fs::path_string	cur_path;
					child_it.get_full_path		(cur_path);
					request_pathes.push_back	(cur_path);
					request	request			=	{ request_pathes.back().c_str(), m_class_id };
					requests.push_back			(request);
				}
			}

			if ( !!(m_flags & query_flag_recursive) )
				add_resources_from_folder		(child_it, request_pathes, requests);
		}
	}

	void   on_fs_iterator_ready (fs_iterator iterator)
	{
		if ( iterator.is_end() )
		{
			u32 const user_thread_id		=	threading::current_thread_id();
			queries_result*	queries			=	(queries_result*)
												XRAY_ALLOC_IMPL(*m_allocator, char, sizeof(queries_result));

			new (queries) queries_result		(0, m_user_callback, m_allocator, user_thread_id, m_parent_query);

			queries->set_result					(false);
			m_user_callback						(*queries);

			queries->~queries_result			();
			XRAY_FREE_IMPL						(*m_allocator, queries);
			return;
		}

		u32 const num_children				=	!!(m_flags & query_flag_recursive) ? 
												iterator.get_num_nodes() : iterator.get_num_children();

		path_string* requests_pathes_ptr	=	XRAY_ALLOC_IMPL(*m_allocator, path_string, num_children);

		buffer_vector<fs::path_string>		request_pathes(requests_pathes_ptr, num_children);
		buffer_vector<request>				requests(ALLOCA(sizeof(request)*num_children), num_children);

		add_resources_from_folder			(iterator, request_pathes, requests);

		request*	requests_ptr		=	requests.size() ? &requests.front() : NULL;
		query_resources						(requests_ptr, 
											 (u32)requests.size(), 
											 m_user_callback, 
											 m_allocator, 
											 NULL,
											 m_priority,
											 m_parent_query);

		XRAY_FREE_IMPL						(*m_allocator, requests_pathes_ptr);

		this->~query_resources_helper		();
		query_resources_helper*	this_ptr	=	this;
		XRAY_FREE_IMPL						(*m_allocator, this_ptr);
	}

private:
	pcstr									m_mask;
	pcstr									m_dir_path;
	query_flag_enum							m_flags;
	class_id								m_class_id;
	query_callback							m_user_callback;
	memory::base_allocator*					m_allocator;
	int										m_priority;
	query_result_for_cook*					m_parent_query;

}; // query_resources_helper

void   resources_manager::query_resources_by_mask (pcstr					const request_mask, 
										  		   class_id					const class_id, 
										  		   query_callback const&		  callback, 
										  		   memory::base_allocator*	const allocator, 
				   					      		   query_flag_enum			const flags, 
										  		   int						const priority,
										  		   query_result_for_cook*	const parent)
{
	ASSERT									(strchr(request_mask, '\\') == NULL);

	pcstr const		dir_end				=	strrchr(request_mask, '/');
	fs::path_string	dir_path;
	if ( dir_end )
		dir_path.append						(request_mask, dir_end);

	pcstr const		mask				=	dir_end ? dir_end+1 : request_mask;
	u32 const		mask_len			=	strings::length(mask);

	query_resources_helper* const helper	=	XRAY_ALLOC_IMPL
												(*allocator,
												 query_resources_helper, 
												 sizeof(query_resources_helper) + 
												 mask_len + 1 + dir_path.length() + 1);

	pstr			mask_in_helper		=	(pstr)helper + sizeof(query_resources_helper);
	pstr			dir_path_in_helper	=	mask_in_helper + mask_len + 1;

	strings::copy							(mask_in_helper, mask_len+1, mask);
	strings::copy							(dir_path_in_helper, dir_path.length()+1, dir_path.c_str());

	new (helper) query_resources_helper		(dir_path_in_helper, 
											 mask_in_helper, 
											 class_id, 
											 callback, 
											 allocator, 
											 flags,
											 priority,
											 parent);
}

void   resources_manager::delete_unmanaged_resource (unmanaged_resource * dying_resource)
{
	cook_base * const cook				=	find_cook( dying_resource->get_class_id() );
	R_ASSERT								(cook);

	if ( dying_resource->creation_source() == resource_base::creation_source_deallocate_buffer_helper )
	{
		dying_resource->~unmanaged_resource	();
		cook->deallocate_resource			(dying_resource);
	}
	else
	{
		if ( translate_query_cook * const translate_query_cook = cook->cast_translate_query_cook() )
		{
			translate_query_cook->delete_resource	(dying_resource);
		}
		else
		{
			pvoid buffer_ptr			=	dynamic_cast<pvoid>(dying_resource);
			cook->call_destroy_resource		(dying_resource);
			cook->deallocate_resource		(buffer_ptr);
		}
	}

	cook->change_cook_users_count			(-1);
}

void   resources_manager::delete_delayed_unmanaged_resource (unmanaged_resource * dying_resource)
{
#ifndef MASTER_GOLD
	unregister_delay_delete_unmanaged		(dying_resource);
#endif // #ifndef MASTER_GOLD

	delete_unmanaged_resource				(dying_resource);

	ASSERT_CMP								(m_num_delay_delete_unmanaged_resources, >, 0);
	threading::interlocked_decrement		(m_num_delay_delete_unmanaged_resources);
}

void   resources_manager::delete_delayed_unmanaged_resources ()
{
	thread_local_data *	thread_data		=	get_thread_local_data(threading::current_thread_id(), false);
	if ( !thread_data )
		return;

	unmanaged_resource * dying_resource	=	thread_data->delayed_delete_unmanaged_resources.pop_all_and_clear();
	while ( dying_resource )
	{
		unmanaged_resource * const next	=	dying_resource->get_next_delay_delete();
		delete_delayed_unmanaged_resource	(dying_resource);
		dying_resource					=	next;
	}
}

void   resources_manager::dispatch_callbacks (bool const finalizing_thread)
{
	u32 const thread_id					=	threading::current_thread_id();

	delete_delayed_unmanaged_resources		();

	thread_local_data* const thread_data =	get_thread_local_data(thread_id, false);
	if ( !thread_data )
		return;

	translate_queries						(thread_data->to_translate_query, thread_data->to_translate_query.pop_all_and_clear());

	dispatch_fs_tasks_callbacks				(thread_data->ready_fs_tasks.pop_all_and_clear(),
											 finalizing_thread);

	m_allocate_resource_functionality.tick	(finalizing_thread);

	create_resources						(thread_data->to_create_resource, thread_data->to_create_resource.pop_all_and_clear(), 
											 finalizing_thread);

	dispatch_query_callbacks				(thread_data->finished_queries.pop_all_and_clear(), 
											 finalizing_thread);	
}

void   resources_manager::dispatch_query_callback (queries_result * ready_query,
												   bool const		finalizing_thread)
{
	if ( !finalizing_thread )
		ready_query->call_user_callback ();

	ready_query->~queries_result			();
	XRAY_FREE_IMPL							(*ready_query->m_allocator, ready_query);
}

void   resources_manager::dispatch_query_callbacks (queries_result * const	ready_query_list, 
												    bool const				finalizing_thread)
{
	queries_result * it_ready_query		=	ready_query_list;

	while ( it_ready_query )
	{
		queries_result * const next_ready_query	=	it_ready_query->m_next_ready;
		dispatch_query_callback				(it_ready_query, finalizing_thread);
		it_ready_query					=	next_ready_query;
	}
}

void   resources_manager::dispatch_fs_tasks_callbacks (fs_task*		ready_fs_tasks, 
													   bool const	finalizing_thread)
{
	while ( ready_fs_tasks )
	{
		fs_task* next_fs_task			=	ready_fs_tasks->next;
		
		if ( !finalizing_thread )
			ready_fs_tasks->call_user_callback ();

		XRAY_DELETE_IMPL					(*ready_fs_tasks->allocator, ready_fs_tasks);

		ready_fs_tasks					=	next_fs_task;
	}
}

void   resources_manager::on_added_queries (u32 const num_queries)
{
	for ( u32 i=0; i<num_queries; ++i )
		threading::interlocked_increment	(m_pending_queries_count);
}

void   resources_manager::on_dispatched_queries (u32 const num_queries)
{
	for ( u32 i=0; i<num_queries; ++i )
		threading::interlocked_decrement	(m_pending_queries_count);
}

void   resources_manager::mark_unmovables_before_defragmentation ()
{
	for ( device_managers::iterator it	=	m_device_managers.begin();
									it	!=	m_device_managers.end();
									++it )
	{
		device_manager * const device_manager	=	* it;
		device_manager->mark_unmovables_before_defragmentation	();
	}
}

void   resources_manager::start_cooks_registration () 
{ 
	threading::interlocked_increment		(m_num_cook_registrators);
}

void   resources_manager::finish_cooks_registration () 
{ 
	if ( threading::interlocked_decrement(m_num_cook_registrators) == 0 )
		wakeup_resources_thread				();
}

void   resources_manager::mount_mounts_path (pcstr const mounts_path)
{
	if ( !mounts_path )
	{
		threading::interlocked_exchange		(m_do_mount_mounts_path, false);
		return;
	}

	m_mounts_path						=	mounts_path;
	threading::interlocked_exchange			(m_do_mount_mounts_path, true);
	wakeup_resources_thread					();

	if ( threading::g_debug_single_thread )
		resources::tick						();
	m_mounts_initialized_event.wait			(threading::event::wait_time_infinite);
}

void   resources_manager::push_new_query (query_result * query, query_type_enum query_type)
{
	if ( query_type == query_type_normal )
	{
		if ( query->has_flag(query_result::flag_locked_fat_iterator) )
			m_new_queries_with_locked_fat_it.push_back		(query);
		else
			m_new_queries_with_unlocked_fat_it.push_back	(query);
	}
	else if ( query_type == query_type_helper_for_mount )
		m_new_queries_for_mount.push_back					(query);
	else
		UNREACHABLE_CODE();
}

void   resources_manager::push_to_translate_query (query_result * query)
{
	class_id const class_id				=	query->get_class_id();
	translate_query_cook * const cook	=	cook_base::find_translate_query_cook(class_id);
	R_ASSERT_U								(cook);
	u32 const allocate_thread_id		=	query->allocate_thread_id();
	if ( allocate_thread_id == threading::current_thread_id() )
	{
		translate_query						(query);
		return;
	}
	
	thread_local_data * local_data		=	get_thread_local_data(allocate_thread_id, true);
	local_data->to_translate_query.push_back	(query);
	if ( allocate_thread_id == m_cooker_thread_id )
		wakeup_cooker_thread				();
}

template <class queries_list>
void   resources_manager::translate_queries (queries_list const &, query_result * const to_translate_query)
{
	query_result * it_query				=	to_translate_query;
	while ( it_query )
	{
		query_result * next_query		=	queries_list::get_next_of_object(it_query);
		translate_query						(it_query);
		it_query						=	next_query;
	}
}

void   resources_manager::translate_query (query_result * const query)
{
	bool const translated_query			=	query->translate_query_if_needed();
	XRAY_UNREFERENCED_PARAMETER				(translated_query);
	R_ASSERT								(translated_query);

	if ( query->allocate_thread_id() == cooker_thread_id() )
		wakeup_cooker_thread				();
}

} // namespace resources
} // namespace xray

