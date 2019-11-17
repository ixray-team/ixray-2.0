////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_manager.h"
#include "resources_allocators.h"
#include "resources_fs_impl.h"

namespace xray {
namespace resources {

fs::path_string	  s_resource_path;

void   initialize ()
{
	XRAY_CONSTRUCT_REFERENCE				(g_resources_manager, resources_manager) ();
	
	threading::yield						(10);
// 	fs::set_allocator_thread_id				(threading::current_thread_id());
// 	memory::g_resources_helper_allocator.user_current_thread_id();
// 	g_resources_manager->get_managed_resource_allocator()->test_defragment();
}

void   mount_mounts_path (pcstr const mounts_path)
{
	g_resources_manager->mount_mounts_path	(mounts_path);
}

bool   is_initialized ()
{
	return									g_resources_manager.initialized();
}

void   finalize ()
{
	XRAY_DESTROY_REFERENCE					(g_resources_manager);
}

void   finalize_thread_usage (bool const calling_from_main_thread)
{
	g_resources_manager->finalize_thread_usage	(calling_from_main_thread);
}

void   wait_and_dispatch_callbacks	(bool const calling_from_main_thread)
{
	g_resources_manager->wait_and_dispatch_callbacks(calling_from_main_thread, false);
}

void   dispatch_callbacks ()
{
	g_resources_manager->dispatch_callbacks	(false);
}

void   start_query_transaction ()
{
	g_resources_manager->start_query_transaction	();
}

void   end_query_transaction ()
{
	g_resources_manager->end_query_transaction	();
}

long   query_resource (pcstr const						request_path, 
					   class_id const					class_id, 
					   query_callback const&			callback, 
					   memory::base_allocator* const	allocator, 
					   user_data_variant const *		user_data,
					   int const						priority,
					   query_result_for_cook* const		parent)
{
	request		request		=	{ request_path, class_id };
	user_data_variant const * user_data_array[]	=	{ user_data };
	return query_resources		(&request, 1, callback, allocator, user_data_array, priority, parent);
}

long   query_resources (request const*					requests, 
					    u32 const						request_count, 
					    query_callback const&			callback, 
					    memory::base_allocator*	const	allocator,
						user_data_variant const *		user_data[],
					    int const						priority,
						query_result_for_cook* const	parent)
{
	return g_resources_manager->query_resources_impl (requests, NULL, request_count, callback, allocator, user_data, priority, parent);
}

struct query_resources_and_wait_callback_proxy_pred
{
	query_resources_and_wait_callback_proxy_pred(query_callback const callback) : callback_(callback), receieved_callback_(false) {}
	void callback (queries_result & result)
	{
		callback_				(result);
		receieved_callback_	=	true;
	}

	bool received_callback		() const { return receieved_callback_; }

private:
	bool						receieved_callback_;
	query_callback				callback_;
};

void   query_resources_and_wait	(request const *				requests, 
					    		 u32 const						request_count, 
					    		 query_callback const &			callback, 
					    		 memory::base_allocator * const	allocator,
								 user_data_variant const *		user_data[],
					    		 int const						priority,
								 query_result_for_cook * const	parent)
{
	query_resources_and_wait_callback_proxy_pred callback_proxy	(callback);

	query_resources					(requests, request_count, boost::bind(& query_resources_and_wait_callback_proxy_pred::callback, & callback_proxy, _1), allocator, user_data, priority, parent);

	while ( !callback_proxy.received_callback() )
	{
		if ( threading::g_debug_single_thread )
			resources::tick			();

		dispatch_callbacks			();
	}
}

void   query_resource_and_wait (pcstr const						request_path, 
					  			class_id const					class_id, 
					  			query_callback const&			callback, 
					  			memory::base_allocator* const	allocator, 
								user_data_variant const *		user_data,
					  			int const						priority,
					  			query_result_for_cook* const	parent)
{
	request		request		=	{ request_path, class_id };
	user_data_variant const * user_data_array[]	=	{ user_data };
	query_resources_and_wait	(&request, 1, callback, allocator, user_data_array, priority, parent);
}

long   query_create_resource  (pcstr							request_name,
							   const_buffer 					src_data, 
							   class_id const					class_id, 
							   query_callback const &			callback, 
							   memory::base_allocator * const	allocator, 
							   user_data_variant const *		user_data,
							   int const						priority,
							   query_result_for_cook * const	parent)
{
	creation_request	request	=	{ request_name, src_data, class_id };
	user_data_variant const * user_data_array[]	=	{ user_data };
	return query_create_resources		(& request, 1, callback, allocator, user_data_array, priority, parent);
}

long   query_create_resources  (creation_request const *		requests, 
								u32 const						request_count, 
								query_callback const &			callback, 
								memory::base_allocator * const	allocator,
								user_data_variant const *		user_data[],
								int const						priority,
								query_result_for_cook * const	parent)
{
	return g_resources_manager->query_resources_impl (NULL, requests, request_count, callback, allocator, user_data, priority, parent);
}

struct query_create_resources_and_wait_callback_proxy_pred
{
	query_create_resources_and_wait_callback_proxy_pred(query_callback const callback) : callback_(callback), receieved_callback_(false) {}
	void callback (queries_result & result)
	{
		callback_				(result);
		receieved_callback_	=	true;
	}

	bool received_callback		() const { return receieved_callback_; }

private:
	bool						receieved_callback_;
	query_callback				callback_;

};

void   query_create_resources_and_wait  (creation_request const *		requests, 
										 u32 const						request_count, 
										 query_callback const &			callback, 
										 memory::base_allocator * const	allocator,
										 user_data_variant const *		user_data[],
										 int const						priority,
										 query_result_for_cook * const	parent)
{
	query_create_resources_and_wait_callback_proxy_pred		callback_proxy		(callback);
	query_create_resources			(requests, request_count, boost::bind(& query_create_resources_and_wait_callback_proxy_pred::callback, & callback_proxy, _1), allocator, user_data, priority, parent);

	while ( !callback_proxy.received_callback() )
	{
		if ( threading::g_debug_single_thread )
			resources::tick			();

		dispatch_callbacks			();
	}
}

void   query_resources_by_mask (pcstr const						request_mask, 
								class_id const					class_id, 
								query_callback const &			callback, 
								memory::base_allocator * const	allocator,
					    		query_flag_enum const			flags,
								int const						priority,
								query_result_for_cook * const	parent)
{
	g_resources_manager->query_resources_by_mask (request_mask, class_id, callback, allocator, flags, priority, parent);
}

void   tick ()
{
	if ( threading::g_debug_single_thread )
	{
		g_resources_manager->resources_tick_impl	();
		g_resources_manager->cooker_tick_impl		();
	}
}

void   register_cook (cook_base * const cook)
{
	g_resources_manager->register_cook(cook);
}

cook_base *   unregister_cook (class_id const resource_class)
{
	return	g_resources_manager->unregister_cook(resource_class);
}

fat_iterator   wrapper_to_fat_it (fat_it_wrapper wrapper)
{
	fat_iterator*	temp	=	(fat_iterator*)&wrapper;
	return						*temp;
}

fat_it_wrapper   fat_it_to_wrapper (fat_iterator it)
{
	fat_it_wrapper*	temp	=	(fat_it_wrapper*)&it;
	return						*temp;
}

void   start_cooks_registration ()
{
	g_resources_manager->start_cooks_registration	();
}

void   finish_cooks_registration ()
{
	g_resources_manager->finish_cooks_registration	();
}

memory::base_allocator *   unmanaged_allocator ()
{
	return						& memory::g_resources_unmanaged_allocator;
}

pvoid   allocate_unmanaged_memory (u32 size, pcstr type_name)
{
	return						g_resources_manager->allocate_unmanaged_memory (size, type_name);
}

managed_resource *   allocate_managed_resource (u32 size)
{
	return						g_resources_manager->allocate_managed_resource (size);
}

void   free_managed_resource (managed_resource * resource)
{
	g_resources_manager->free_managed_resource (resource);
}

void	set_query_finished_callback (query_finished_callback callback)
{
	g_resources_manager->set_query_finished_callback	(callback);
}

void	set_out_of_memory_callback (out_of_memory_callback callback)
{
	g_resources_manager->set_out_of_memory_callback	(callback);
}

} // namespace xray
} // namespace resources


