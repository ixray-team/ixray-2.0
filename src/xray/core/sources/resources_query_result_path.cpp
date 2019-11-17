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

using namespace fs;

bool   query_result::is_translate_query () const
{
	return								!!cook_base::find_translate_query_cook(m_class_id);
}

void   query_result::translate_request_path ()
{
	cook_base * const	cook		=	g_resources_manager->find_cook(m_class_id);
	if ( !cook )
		return;

	fs::path_string		new_path;
	cook->translate_request_path		(get_requested_path(), new_path);
	R_ASSERT							(new_path.length());

	if ( new_path != get_requested_path() )
	{
		m_request_path				=	(pstr)XRAY_ALLOC_IMPL(* m_user_allocator, char, new_path.length()+1);
		strings::copy					(m_request_path, new_path.length()+1, new_path.c_str());
		set_flag						(flag_should_free_request_path);
	}
}

void   query_result_for_cook::set_request_path (pcstr path)
{
	strings::copy						(m_request_path, m_request_path_max_size, path);
}

fs::path_string   query_result_for_user::get_full_path () const 
{
	fs::path_string path;
	fat_iterator	fat_it			=	wrapper_to_fat_it(m_fat_it);
	return								fat_it.get_full_path();
}

bool   query_result::next_item_in_request_path (request_path_iterator & out_it) const
{
	if ( !get_requested_path() )
		return							false;

	if ( !out_it.iterator )
		out_it.iterator				=	get_requested_path();

	if ( ! * out_it.iterator )
		return							false;

	pcstr const next_item_separator	=	strchr(out_it.iterator, request::path_separator);
	if ( next_item_separator )
	{
		out_it.item.assign				(out_it.iterator, next_item_separator);
		out_it.iterator				=	next_item_separator + 1;
	}
	else
	{
		out_it.item					=	out_it.iterator;
		out_it.iterator				=	out_it.iterator + strings::length(out_it.iterator);
	}

	return								true;
}

bool   query_result::has_same_item_in_request_path (query_result const & other_request_path) const
{
	request_path_iterator				it;
	while ( next_item_in_request_path(it) )
	{
		request_path_iterator			other_it;
		while ( other_request_path.next_item_in_request_path(other_it) )
		{
			if ( it.item == other_it.item )
				return					true;
		}
	}

	return								false;
}

signalling_bool   query_result::select_disk_path_from_request_path (buffer_string * out_disk_path) const
{
	request_path_iterator				it;
	while ( next_item_in_request_path(it) )
	{
		if ( fs::g_fat->get_disk_path_to_store_file(it.item.c_str(), out_disk_path) )
		{
			strings::copy				(m_request_path, m_request_path_max_size, it.item.c_str());
			return						true;
		}
	}

	FATAL								("omg! none of request_path items '%s' are mounted to disk - cannot select disk path!", 
										 get_requested_path());

	return								false;
}


//-----------------------------------------------
// misc code
//-----------------------------------------------

fs::path_string   get_native_file_path (query_result const * query, bool assert_on_fail) 
{
	XRAY_UNREFERENCED_PARAMETER			( assert_on_fail );

	if ( query->is_save_type() )
	{
		fs::path_string	disk_path;
		bool const result			=	query->select_disk_path_from_request_path(& disk_path);
		XRAY_UNREFERENCED_PARAMETER		( result );
		R_ASSERT						(result);
		return							convert_to_native(disk_path.c_str());
	}

	using namespace						fs;
	fat_iterator const it			=	wrapper_to_fat_it(query->get_fat_it());
	path_string							path;
	g_fat->get_disk_path				(it, path);

	R_ASSERT							(path.length() || !assert_on_fail);

	if ( query->is_replication_type() )
	{
		path_string						replicated_path;
		g_fat->replicate_path			(path.c_str(), replicated_path);
		return							convert_to_native(replicated_path.c_str());
	}

	return								path.length() ? convert_to_native(path.c_str()) : "";
}

void   query_result::process_request_path ()
{
	if ( has_flag(flag_processed_request_path) )
		return;

	unlock_fat_it						();

	set_flag							(flag_processed_request_path);
	pstr	cur_path				=	m_request_path;
	file_system::iterator	fat_it;
	fat_it.set_end						();

	while ( cur_path )
	{
		pstr const separator_pos	=	strchr(cur_path, request::path_separator);
		if ( separator_pos )
			*separator_pos			=	NULL;

		if ( *cur_path == physical_path_char )
		{
			if ( threading::current_thread_id() == m_user_thread_id )
				m_temp_disk_fat_it_allocator	=	m_user_allocator;
			else
				m_temp_disk_fat_it_allocator	=	& memory::g_mt_allocator;

			fat_it					=	g_fat->create_temp_disk_it(m_temp_disk_fat_it_allocator, cur_path+1);
			if ( fat_it != g_fat->end() )
				set_flag				(flag_uses_physical_path);
		}
		else
		{
			if ( get_create_resource_result() == cook_base::result_requery )
				g_fat->mount_disk_node_by_logical_path	(cur_path);

			fat_it					=	g_fat->find(cur_path);
		}

		if ( fat_it != g_fat->end() && !fat_it.is_folder() )
		{
			if ( cur_path != m_request_path )
			{
				pstr					path	=	NULL;
				STR_JOINA				(path, cur_path);
				// we're sure buffer is ok
				strings::copy			(m_request_path, strings::length(path)+1, path);
			}
			break;
		}

		if ( separator_pos )
		{
			*separator_pos			=	request::path_separator;
			cur_path				=	separator_pos + 1;
		}
		else
			break;
	}

	m_fat_it						=	fat_it_to_wrapper(fat_it);

	lock_fat_it							();
}

void   query_result::lock_fat_it ()
{
	file_system::iterator	fat_it	=	wrapper_to_fat_it(m_fat_it);
	if ( fat_it.is_end() )
		return;

	R_ASSERT							(!has_flag(flag_locked_fat_iterator));
	set_flag							(flag_locked_fat_iterator);
	g_resources_manager->change_count_of_pending_query_with_fat_it (+1);
	fat_it.set_is_locked				(true);
}

void   query_result::unlock_fat_it ()
{
	file_system::iterator	fat_it	=	wrapper_to_fat_it(m_fat_it);
	if ( fat_it.is_end() )
	{
		R_ASSERT						(!has_flag(flag_locked_fat_iterator));
		return;
	}

	R_ASSERT							(has_flag(flag_locked_fat_iterator));
	unset_flag							(flag_locked_fat_iterator);
	g_resources_manager->change_count_of_pending_query_with_fat_it (-1);
	fat_it.set_is_locked				(false);
}

void   query_result::late_set_fat_it (fat_it_wrapper const it)
{
	fat_iterator const previous_it	=	wrapper_to_fat_it(m_fat_it);
	fat_iterator new_it				=	wrapper_to_fat_it(it);
	if ( previous_it == new_it )
		return;

	R_ASSERT							(previous_it.is_end());
	R_ASSERT							(!has_flag(flag_locked_fat_iterator));
	m_fat_it						=	it;

	set_flag							(flag_locked_fat_iterator);
	g_resources_manager->change_count_of_pending_query_with_fat_it (+1);
	new_it.set_is_locked				(true);

	if ( m_raw_managed_resource )
		m_raw_managed_resource->late_set_fat_it		(it);

	if ( m_managed_resource )
		m_managed_resource->late_set_fat_it			(it);

	if ( m_unmanaged_resource )
		m_unmanaged_resource->late_set_fat_it		(it);
}

} // namespace resources
} // namespace xray

