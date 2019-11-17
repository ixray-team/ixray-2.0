#include "pch.h"
#include "resources_helper.h"
#include "resources_manager.h"
#include "resources_callbacks.h"
#include "resources_device_manager.h"

#include <xray/resources_cook_classes.h>
#include <xray/compressor_ppmd.h>

#include <xray/os_preinclude.h>
#define _WIN32_WINNT 500
#include <xray/os_include.h>

namespace xray {
namespace resources {

using namespace fs;

cook_base *   resources_manager::find_cook (class_id const resource_class)
{
	R_ASSERT									(resource_class < last_resource_class);
	return										 m_cooks_registry[resource_class];
}

void   resources_manager::register_cook	(cook_base * const cook)
{
	class_id const resource_class			=	cook->get_class_id();
	R_ASSERT									(resource_class < last_resource_class);

	R_ASSERT									(!find_cook(resource_class), 
												 "cook for this kind of resource is already registered" );
	m_cooks_registry[resource_class]		=	cook;
}

bool   resources_manager::thread_can_exit ()
{
	u32	const current_thread_id				=	threading::current_thread_id();
	thread_local_data*	const local_data	=	get_thread_local_data(current_thread_id, false);
	
	if ( !local_data )
		return									true;

	bool const local_data_allows			=	local_data->delayed_delete_unmanaged_resources.empty()
														&&
												local_data->ready_fs_tasks.empty()
														&&
												local_data->finished_queries.empty()
														&&
												local_data->to_free_user_buffers.empty()
														&&
												local_data->to_create_resource.empty();

	return										!m_pending_queries_count
														&&
												m_created_resources.empty()
														&&
												m_delayed_delete_resources.empty()
														&&
												m_delayed_delete_unmanaged_resources.empty()
														&&
												m_fs_tasks.empty()
														&&
												!m_count_of_pending_mount_operations
														&&
												!m_num_current_fs_ops
														&&
												!m_dispatching_delay_delete_unmanaged_resources_flag 
														&&
												local_data_allows;
}

cook_base *   resources_manager::unregister_cook (class_id const resource_class)
{
	R_ASSERT									(resource_class < last_resource_class);
	cook_base * const result				=	m_cooks_registry[resource_class];
	if ( result )
		R_ASSERT_CMP							(result->cook_users_count(), ==, 0);

	m_cooks_registry[resource_class]		=	NULL;
	return										result;
}

void   resources_manager::dispatch_created_resources ()
{
	R_ASSERT									(threading::current_thread_id() == m_resources_thread_id);

	delete_delayed_unmanaged_resources			();

	query_result * const	last_query		=	m_created_resources.back();

	while ( identity(true) )
	{
		query_result *	cur_query			=	m_created_resources.front();
		if ( !cur_query )
			break;

		cur_query->on_create_resource_end		(query_result::push_to_ready_queries_true);
		m_created_resources.pop_front			();

		if ( cur_query == last_query )
			break;
	}
}

void   resources_manager::dispatch_decompressed_resources ()
{
	R_ASSERT									(threading::current_thread_id() == m_resources_thread_id);

	query_result *	query					=	m_decompressed_resources.pop_all_and_clear();

	while ( query )
	{
		query_result * const next			=	query->m_next_decompress;
		query->m_next_decompress			=	NULL;
		query->on_decompressing_end				();
		query								=	next;
	}
}

void   resources_manager::add_resource_to_create (query_result * const query)
{
	cook_base *	const cook					=	find_cook(query->m_class_id);
	R_ASSERT									(cook);
	
	u32 const creation_thread_id			=	cook->creation_thread_id();

	if ( creation_thread_id == m_cooker_thread_id )
	{
		m_resources_to_create.push_back		(query);
		wakeup_cooker_thread					();
	}
	else
	{
		thread_local_data * const local_data	=	get_thread_local_data(creation_thread_id, true);
		local_data->to_create_resource.push_back			(query);
	}
}

template <class query_list>
void   resources_manager::create_resources (query_list const &, query_result * it_query, bool finalizing_thread)
{
	while ( it_query )
	{
		query_result * next					=	query_list::get_next_of_object(it_query);
		
		if ( finalizing_thread )
			it_query->set_create_resource_result (cook_base::result_error);
		else
			it_query->do_create_resource				();

		if ( it_query->need_saving_of_generated_resource() )
			push_generated_resource_to_save		(it_query);
		else if ( it_query->get_create_resource_result() != cook_base::result_postponed )
			on_created_resource					(it_query);

		it_query							=	next;
	}
}

void   resources_manager::on_created_resource (query_result * query)
{
	if ( query->get_create_resource_result() == cook_base::result_requery )
	{
		query->unset_flag						(query_result::flag_processed_request_path);
		query->unset_flag						(query_result::flag_translated_query);
		push_new_query							(query, query_type_normal);
		wakeup_resources_thread					();
	}
	else
	{
		m_created_resources.push_back			(query);
		wakeup_resources_thread					();
	}
}

void   resources_manager::decompress_resource (query_result* const query)
{
	const_buffer	src_file				=	query->pin_compressed_file();
	mutable_buffer	dest_file				=	cast_away_const(query->pin_raw_file());

	u32 out_size;
	u32 const uncompressed_size				=	query->get_raw_file_size();
	if ( !m_compressor->decompress(src_file, dest_file, out_size) || out_size != uncompressed_size )
	{
		R_ASSERT								(identity(false), "failed to decompress file: '%s'", query->get_requested_path());
		query->m_error_type					=	query_result::error_type_cannot_decompress_file;
	}

	query->unpin_compressed_file				(src_file);
	query->unpin_raw_file						(dest_file);

	query->m_next_decompress				=	NULL;
	m_decompressed_resources.push_back			(query);
}

void   resources_manager::decompress_resources ()
{
	query_result * resource_to_decompress	=	m_resources_to_decompress.pop_all_and_clear();

	while ( resource_to_decompress )
	{
		query_result * const next			=	resource_to_decompress->m_next_decompress;

		decompress_resource						(resource_to_decompress);		

		resource_to_decompress				=	next;

		wakeup_resources_thread					();
	}
}

void   resources_manager::cooker_tick_impl ()
{
	query_result * resource_to_cook			=	m_resources_to_create.pop_all_and_clear();

	dispatch_callbacks							(false);
	delete_delayed_unmanaged_resources			();
	create_resources							(m_resources_to_create, resource_to_cook, false);

	decompress_resources						();
}

void   resources_manager::cooker_thread_proc ()
{
	ASSERT										(!threading::g_debug_single_thread);
	while ( !m_cooker_thread_exit )
	{
		m_cooker_wakeup_event.wait				(threading::event::wait_time_infinite);
		cooker_tick_impl						();
	}

	finalize_thread_usage						(false);
	m_cooker_thread_exited					=	true;
}

void   resources_manager::add_resource_to_decompress (query_result* const query)
{
	m_resources_to_decompress.push_back			(query);
	wakeup_cooker_thread						();
}

} // namespace xray
} // namespace resources
