////////////////////////////////////////////////////////////////////////////
//	Created		: 14.06.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_manager.h"
#include "fs_file_system.h"

namespace xray {
namespace resources {

using namespace fs;

void   resources_manager::add_to_generate_if_no_file_queue (query_result & query)
{
	m_generate_if_no_file_queue.push_back	(& query);
	R_ASSERT								(!query.has_flag(query_result::flag_is_referer));
	R_ASSERT								(!query.has_flag(query_result::flag_in_generating_because_no_file_queue));
	query.set_flag							(query_result::flag_in_generating_because_no_file_queue);
}

void   resources_manager::remove_from_generate_if_no_file_queue	(query_result & query)
{
	R_ASSERT								(!query.has_flag(query_result::flag_is_referer));
	R_ASSERT								(query.has_flag(query_result::flag_in_generating_because_no_file_queue));
	query.unset_flag						(query_result::flag_in_generating_because_no_file_queue);
	m_generate_if_no_file_queue.erase		(& query);
}

static command_line::key    s_skip_file_not_found	("skip_file_not_found", "", "fs", "doesn't debug break if file is not found");

void   resources_manager::init_query_with_no_fat_it (query_result & query)
{
#ifdef DEBUG
	file_system::iterator const	fat_it	=	wrapper_to_fat_it(query.m_fat_it);
	ASSERT									(fat_it.is_end());
#endif // #ifdef DEBUG
	 
	if ( cook_base::does_create_resource_if_no_file(query.get_class_id()) && 
		 query.get_create_resource_result() != cook_base::result_requery )
	{
		generate_if_no_file_queue::policy::mutex_raii	raii	(m_generate_if_no_file_queue.threading_policy());
		if ( cook_base::reuse_type(query.get_class_id()) != cook_base::reuse_false )
		{
			query_result * active_query		=	m_generate_if_no_file_queue.front();
			while ( active_query )
			{
				if ( query.has_same_item_in_request_path(* active_query) )
				{
					LOGI_INFO					("resources:manager", "no fat_it query '%s' added as refering", query.get_requested_path());
					active_query->add_referrer	(& query, false);
					return;
				}
				active_query				=	m_generate_if_no_file_queue.get_next_of_object(active_query);
			}
		}

		LOGI_INFO							("resources:manager", "no fat_it query '%s' added for generate_if_no_file cook", query.get_requested_path());
		query.set_error_type				(query_result::error_type_file_not_found);
		add_to_generate_if_no_file_queue	(query);
		prepare_raw_resource				(& query);
		return;
	}

	if ( debug::is_debugger_present() && !s_skip_file_not_found )
		LOGI_WARNING						("resources", "cannot find file: \"%s\"", query.get_requested_path());

	query.set_error_type					(query_result::error_type_file_not_found);
	query.on_query_end						();
}

void   resources_manager::init_query_result (query_result & query)
{
	if ( query.is_translate_query() )
	{
		push_to_translate_query				(& query);
		return;
	}

	bool const is_query_create			=	query.creation_data_from_user();

	query_result::consider_with_name_registry_result_enum const result	=	
			query.consider_with_name_registry(is_query_create ? 
											  query_result::only_try_to_get_associated_resource_false :
											  query_result::only_try_to_get_associated_resource_true);

	if ( result == query_result::consider_with_name_registry_result_error || 
		 result == query_result::consider_with_name_registry_result_got_associated_resource )
	{
		query.on_query_end					();
		return;
	}
	else if ( result == query_result::consider_with_name_registry_result_added_as_referer )
		return;

	if ( query.creation_data_from_user() )
	{
		prepare_raw_resource				(& query);
		return;
	}

	query.process_request_path				();
	file_system::iterator fat_it		=	wrapper_to_fat_it(query.m_fat_it);
	if ( fat_it.is_end() )
	{
		init_query_with_no_fat_it			(query);
		return;
	}

	path_string	file_path; 
	g_fat->get_disk_path					(fat_it, file_path);

	cook_base::reuse_enum const reuse_type	=	cook_base::reuse_type(query.get_class_id());

	const_buffer							inline_data;
	if ( unmanaged_resource_ptr const cached_unmanaged = fat_it.get_associated_unmanaged_resource_ptr() )
	{
		R_ASSERT							(reuse_type == cook_base::reuse_true);

		query.set_unmanaged_resource		(cached_unmanaged, cached_unmanaged->memory_usage());
		query.set_flag						(query_result::flag_reused_resource);
		query.on_query_end					();
	}
	else if ( managed_resource_ptr const cached_resource = fat_it.get_associated_managed_resource_ptr() )
	{
		if ( reuse_type == cook_base::reuse_raw )
			R_ASSERT						(cached_resource->needs_cook());
		else if ( reuse_type == cook_base::reuse_true )
			R_ASSERT						(!cached_resource->needs_cook() && cook_base::cooks_managed_resource(query.get_class_id()));
		else
			NOT_IMPLEMENTED();

		if ( cached_resource->needs_cook() )
		{
			query.m_raw_managed_resource	=	cached_resource;
			R_ASSERT						(cached_resource->get_size());
			query.on_file_operation_end		();
		}
		else
		{
			query.set_managed_resource		(cached_resource);
			query.set_flag					(query_result::flag_reused_resource);
			query.on_query_end				();
		}
	}
	else if ( query_result * const active_query = fat_it.get_associated_query_result() )
	{
		active_query->add_referrer			(& query);
	}
	else
	{
		R_ASSERT 							(!fat_it.is_associated() || !query.is_load_type());
		
		if ( query.is_load_type() && !fat_it.is_end() )
		{
			if ( cook_base::reuse_type(query.get_class_id()) != cook_base::reuse_false )
				fat_it.set_associated		(& query);
		}

		prepare_raw_resource				(& query);
	}
}

void   resources_manager::init_new_queries (query_result* const queries_with_unlocked_fat_it)
{
	query_result *	new_query			=	queries_with_unlocked_fat_it;

	while ( new_query )
	{
		query_result *	const next_query	=	new_query->m_next_to_init;
		
		class_id const class_id				=	new_query->get_class_id();

		if ( class_id != raw_data_class && !find_cook(class_id) )
		{
			if ( !cooks_are_registering() )
			{
				new_query->set_error_type	(query_result::error_type_cook_not_registered);
				new_query->on_query_end		();
			}
			else
			{
				m_new_queries_waiting_for_cook_register.push_back	(new_query);
			}						
		}
		else
		{
			init_query_result				(*new_query);
		}

		new_query						=	next_query;
	}
}

} // namespace resources
} // namespace xray
