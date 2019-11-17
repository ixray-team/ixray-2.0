////////////////////////////////////////////////////////////////////////////
//	Created		: 28.12.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/resources_query_result.h>
#include "resources_manager.h"

namespace xray {
namespace resources {

void   query_result::send_to_create_resource ()
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	bool const is_inline_uncompressed	=	!fat_it.is_end() && fat_it.is_inlined() && !fat_it.is_compressed();
	R_ASSERT_U							((is_success() && (m_raw_managed_resource || m_raw_unmanaged_buffer || is_inline_uncompressed)) || 
										 cook_base::does_create_resource_if_no_file(m_class_id) || 
										 creation_data_from_user());

	if ( cook_base::does_create_resource(m_class_id) )
	{
		g_resources_manager->add_resource_to_create	(this);
		return;
	}

	// NO PROCESSING WAY

	m_managed_resource				=	m_raw_managed_resource;
	LOGI_INFO("resources:resource",		"created %s", 
										m_managed_resource->log_string	().c_str());
	set_create_resource_result			(cook_base::result_success);
	m_final_resource_size			=	m_managed_resource->get_size();
	on_create_resource_end				(push_to_ready_queries_true);
}

void   query_result::set_create_resource_result (cook_base::result_enum const result)
{
	if ( result == cook_base::result_postponed )
	{
		if ( !has_flag(flag_finished_create_resource) )
			m_create_resource_result			=	result;
		else
			R_ASSERT								(m_create_resource_result != cook_base::result_undefined);
	}
	else
	{
		R_ASSERT									(!has_flag(flag_finished_create_resource) ||
													  has_flag(flag_is_referer),
													 "finish_query was already called!");
		m_create_resource_result				=	result;
	}
}

void   query_result::do_managed_create_resource (managed_cook * cook)
{
	if ( m_managed_resource )
		m_managed_resource->set_is_unmovable	(true);

	if ( need_create_resource_if_no_file() )
	{
		managed_cook::create_resource_if_no_file_delegate_type delegate	=	cook->get_create_resource_if_no_file_delegate();
		R_ASSERT								(delegate);

		delegate								(* this, m_managed_resource);
	}
	else
	{
		R_ASSERT								(!m_offset_to_file);
		const_buffer const raw_data	=			pin_raw_file();
		cook->create_resource					(* this, raw_data, m_managed_resource);
		unpin_raw_file							(raw_data);
	}

	if ( m_managed_resource )
		m_managed_resource->set_is_unmovable	(false);
}

void   query_result::do_inplace_managed_create_resource (inplace_managed_cook * cook)
{
	if ( m_raw_managed_resource )
		m_raw_managed_resource->set_is_unmovable	(true);

	if ( need_create_resource_if_no_file() )
	{
		inplace_managed_cook::create_resource_if_no_file_delegate_type delegate	=	cook->get_create_resource_if_no_file_delegate();
		R_ASSERT									(delegate);
		delegate									(* this, m_raw_managed_resource, m_final_resource_size);
	}
	else
	{
		cook->create_resource						(* this, m_raw_managed_resource, get_raw_file_size(), m_final_resource_size);
	}

	m_managed_resource							=	m_raw_managed_resource;
	if ( m_raw_managed_resource )
		m_raw_managed_resource->set_is_unmovable	(false);
}

void   query_result::do_unmanaged_create_resource (unmanaged_cook * cook)
{
	if ( need_create_resource_if_no_file() )
	{
		unmanaged_cook::create_resource_if_no_file_delegate_type delegate	=	cook->get_create_resource_if_no_file_delegate();
		R_ASSERT									(delegate);
		delegate									(* this, m_unmanaged_buffer);
	}
	else
	{
		const_buffer const raw_data				=	pin_raw_buffer();
		cook->create_resource						(* this, raw_data, m_unmanaged_buffer);
		unpin_raw_buffer							(raw_data);
	}

	if ( m_create_resource_result != cook_base::result_postponed && 
		 m_create_resource_result != cook_base::result_requery && 
		 !has_flag(flag_zero_unmanaged_resource_was_set) )
	{
		CURE_ASSERT									(m_unmanaged_resource, return, "cook::create_resource should have called query.set_unmanaged_resource");

		if ( m_unmanaged_resource->creation_source() == resource_base::creation_source_unset )
		{
			R_ASSERT								(m_unmanaged_resource->creation_source() == resource_base::creation_source_unset);
			set_deleter_object						(m_unmanaged_resource.c_ptr());
			m_unmanaged_resource->late_set_fat_it	(get_fat_it_zero_if_physical_path_it());
			set_creation_source_for_resource		(m_unmanaged_resource);
		}
		else
			R_ASSERT								(m_unmanaged_resource->has_deleter_object());
	}
	else
		R_ASSERT								(!m_unmanaged_resource);
}

void   query_result::do_inplace_unmanaged_create_resource (inplace_unmanaged_cook * cook)
{
	bool const do_inplace_create_resource	=	need_create_resource_inplace_in_creation_or_inline_data();
	if ( do_inplace_create_resource )
	{
		inplace_unmanaged_cook::create_resource_inplace_delegate_type delegate	=	has_uncompressed_inline_data() ?
			cook->get_create_resource_inplace_in_inline_fat_delegate() : cook->get_create_resource_inplace_in_creation_data_delegate();

		R_ASSERT								(delegate);
		delegate								(* this, m_raw_unmanaged_buffer);

		R_ASSERT								(cook->reuse_type() == cook_base::reuse_true || has_flag(flag_zero_unmanaged_resource_was_set));
	}
	else if ( need_create_resource_if_no_file() )
	{
		inplace_unmanaged_cook::create_resource_if_no_file_delegate_type delegate	=	cook->get_create_resource_if_no_file_delegate();
		R_ASSERT								(delegate);
		delegate								(* this, m_raw_unmanaged_buffer);
	}
	else
	{
		cook->create_resource					(* this, m_raw_unmanaged_buffer);
	}

	if ( m_create_resource_result != cook_base::result_postponed && 
		 m_create_resource_result != cook_base::result_requery && 
		!has_flag(flag_zero_unmanaged_resource_was_set) )
	{
		CURE_ASSERT						(m_unmanaged_resource, return, "cook::create_resource should have called query.set_unmanaged_resource or query.set_zero_unmanaged_resource");
		
		if ( do_inplace_create_resource && has_uncompressed_inline_data() )
			m_unmanaged_resource->set_as_inlined_in_fat	();
		else
		{
			if ( !m_unmanaged_resource->has_deleter_object() )
				set_deleter_object		(m_unmanaged_resource.c_ptr());
		}

		if ( m_unmanaged_resource->creation_source() == resource_base::creation_source_unset )
		{
			m_unmanaged_resource->late_set_fat_it		(get_fat_it_zero_if_physical_path_it());
			set_creation_source_for_resource			(m_unmanaged_resource);
		}
	}
	else
		R_ASSERT									(!m_unmanaged_resource);
}

void   query_result::do_create_resource ()
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	if ( fat_it.is_end() )
	{
		R_ASSERT						(cook_base::does_create_resource_if_no_file(m_class_id) ||
										 creation_data_from_user() ||
										 has_uncompressed_inline_data());
	}

	unset_flag									(flag_finished_create_resource);
	set_flag									(flag_in_create_resource);			

	if ( managed_cook * cook = cook_base::find_managed_cook(m_class_id) )
	{
		do_managed_create_resource				(cook);		
	}
	else if ( inplace_managed_cook * cook = cook_base::find_inplace_managed_cook(m_class_id) )
	{
		do_inplace_managed_create_resource		(cook);		
	}
	else if ( unmanaged_cook * cook = cook_base::find_unmanaged_cook(m_class_id) )
	{
		do_unmanaged_create_resource			(cook);		
	}
	else if ( inplace_unmanaged_cook * cook = cook_base::find_inplace_unmanaged_cook(m_class_id) )
	{
		do_inplace_unmanaged_create_resource	(cook);
	}
	else
	{
		set_create_resource_result				(cook_base::result_success);
	}

	R_ASSERT									(m_create_resource_result != cook_base::result_undefined,
												 "cooker should have called finish_query!");

	unset_flag									(flag_in_create_resource);

	if ( need_saving_of_generated_resource() )
	{
		R_ASSERT								(m_create_resource_result == cook_base::result_success,
												 "when save_generated_resource should only be called along with finish_query(result_success)");
		unset_flag								(flag_finished_create_resource); // unsetting, because we're waiting async save operation that will finish us
		return;
	}

	if ( m_create_resource_result == cook_base::result_postponed )
		return;

	do_create_resource_end_part					();
}
	
void   query_result::do_create_resource_end_part ()
{
	R_ASSERT							(!m_raw_managed_resource || !m_raw_managed_resource->is_unmovable());
	R_ASSERT							(!m_managed_resource || !m_managed_resource->is_unmovable());

	if ( m_create_resource_result == cook_base::result_requery )
	{
		R_ASSERT						(!m_raw_unmanaged_buffer && !m_unmanaged_buffer);
		m_raw_managed_resource		=	NULL;
	}

	R_ASSERT							(get_create_resource_result() != cook_base::result_error);

	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);

	if ( get_create_resource_result() == cook_base::result_error )
	{
		if ( cook_base::does_create_resource_if_no_file(m_class_id) )
		{
			LOGI_ERROR					("resources:resource", "generating failed for unfound resource %s", 
										 get_requested_path());
		}
		else
		{
			LOGI_ERROR					("resources:resource", "create_resource failed for %s", 
										 m_raw_managed_resource->log_string ().c_str());
		}
	}
	else if ( get_create_resource_result() == cook_base::result_requery )
	{
		set_error_type					(error_type_unset, false);
		LOGI_INFO						("resources:resource", "requerying resource '%s'", 
										 m_raw_managed_resource ? m_raw_managed_resource->log_string ().c_str() : get_requested_path());
	}
	else
	{
		if ( has_flag(flag_zero_unmanaged_resource_was_set) )
		{
			LOGI_INFO					("resources:resource",	"no resource was cooked by cook, request path was: '%s'", 
										 get_requested_path());
		}
		else
		{
			LOGI_INFO					("resources:resource",	"%s %s", 
										 need_create_resource_if_no_file() ? "generated" : "cooked",
										 m_managed_resource ? m_managed_resource->log_string().c_str() 
															: m_unmanaged_resource->log_string().c_str() );
		}
	}
}

void   query_result::on_create_resource_end (push_to_ready_queries_bool const push_to_ready_queries)
{
	if ( m_error_type == error_type_unset && cook_base::does_create_resource(m_class_id) )
	{
		if ( m_managed_resource )
		{
			if ( cook_base::cooks_inplace(m_class_id) )
			{
				R_ASSERT						(m_final_resource_size || !m_managed_resource->get_size());
				m_managed_resource->resize_down	(m_final_resource_size);
			}
		}
		else if ( m_unmanaged_resource || has_flag(flag_zero_unmanaged_resource_was_set) )
		{

		}
		else
			R_ASSERT							(need_saving_of_generated_resource());
	}

	associate_created_resource_with_fat_or_name_registry	();

	if ( has_flag(flag_in_generating_because_no_file_queue) )
		g_resources_manager->remove_from_generate_if_no_file_queue	(* this);

	R_ASSERT											(!m_compressed_resource || !m_compressed_resource->is_unmovable());
	R_ASSERT											(!m_raw_managed_resource || !m_raw_managed_resource->is_unmovable());

	if ( !has_flag(flag_is_referer) )
	{
		query_result * cur							=	m_next_referer;
		while ( cur != this )
		{
			query_result * next						=	cur->m_next_referer;
			cur->on_refered_query_ended					(this);
			cur										=	next;
		}
	}

	if ( m_error_type != error_type_unset )
		m_raw_managed_resource						=	NULL;

	on_query_end										(push_to_ready_queries);
}

void   query_result::finish_normal_query (cook_base::result_enum const create_resource_result)
{
	set_create_resource_result							(create_resource_result);
	// setting flag_finished_create_resource must be after set_create_resource_result

	if ( create_resource_result != cook_base::result_postponed )
		set_flag										(flag_finished_create_resource);

	if ( has_flag(flag_in_create_resource) )
		return;

	do_create_resource_end_part							();
	g_resources_manager->on_created_resource			(this);
}

void   query_result::finish_translated_query (cook_base::result_enum result)
{
	bool const is_requery_result					=	(result == cook_base::result_requery);
	R_ASSERT											(result == cook_base::result_success || 
														 result == cook_base::result_error ||
														 result == cook_base::result_out_of_memory ||
														 is_requery_result);

	if ( result == cook_base::result_out_of_memory )
	{
		m_out_of_memory_type						=	out_of_memory_on_translate_query;
		m_error_type								=	error_type_out_of_memory;

		if ( g_resources_manager->get_out_of_memory_callback() )
			g_resources_manager->push_out_of_memory_query	(this);
		else
			result									=	cook_base::result_error;
	}

	set_create_resource_result							(result);

	if ( result == cook_base::result_error )
		set_error_type									(query_result::error_type_query_translation_failed);

	bool is_new_resource							=	false;

	if ( result == cook_base::result_success )
	{
		R_ASSERT										(m_unmanaged_resource || m_managed_resource);
		if ( m_unmanaged_resource )
		{
			if ( m_unmanaged_resource->creation_source() == resource_base::creation_source_unset )
			{
				is_new_resource						=	true;
				R_ASSERT								(m_unmanaged_resource->creation_source() == resource_base::creation_source_unset);
				set_deleter_object						(m_unmanaged_resource.c_ptr());
				m_unmanaged_resource->late_set_fat_it	(get_fat_it_zero_if_physical_path_it());
				set_creation_source_for_resource		(m_unmanaged_resource);
			}
			else
				R_ASSERT								(m_unmanaged_resource->has_deleter_object());
		}
		else if ( m_managed_resource )
		{
			if ( m_managed_resource->creation_source() == resource_base::creation_source_unset )
			{
				is_new_resource						=	true;
				m_managed_resource->late_set_fat_it		(get_fat_it_zero_if_physical_path_it());
				set_creation_source_for_resource		(m_managed_resource);
			}
		}
	}

	if ( need_saving_of_generated_resource() )
	{
		R_ASSERT								(m_create_resource_result == cook_base::result_success,
												"when save_generated_resource should only be called along with finish_query(result_success)");
		unset_flag								(flag_finished_create_resource); // unsetting, because we're waiting async save operation that will finish us

		g_resources_manager->push_generated_resource_to_save	(this);
		return;
	}

	if ( has_flag(flag_zero_unmanaged_resource_was_set) )
	{
		LOGI_INFO									("resources:resource",	"no resource was cooked by cook, request path was: '%s'", 
													 get_requested_path());
	}
	else if ( !is_requery_result )
	{
		R_ASSERT									(m_managed_resource || m_unmanaged_resource);
		if ( is_new_resource )
		{
			LOGI_INFO								("resources:resource",	"cooked by translate_query_cook: %s", 
													 m_managed_resource ? m_managed_resource->log_string ().c_str() 
																		: m_unmanaged_resource->log_string ().c_str());
		}
	}

	g_resources_manager->on_created_resource		(this);
}

} // namespace resources
} // namespace xray
