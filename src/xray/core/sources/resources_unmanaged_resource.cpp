////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/resources_unmanaged_resource.h>
#include "resources_manager.h"
#include "fs_file_system.h"

namespace xray {
namespace resources {

using namespace fs;
//----------------------------------------------------------
// unmanaged_resource
//----------------------------------------------------------

unmanaged_resource::unmanaged_resource () : resource_base(is_unmanaged_resource_flag),
											m_next_delay_delete(NULL), 
											m_deleter(NULL),
											m_fat_it(NULL),
											m_inlined_in_fat(false),
											m_next_in_global_list(NULL),
											m_prev_in_global_list(NULL),
											m_next_in_global_delay_delete_list(NULL),
											m_prev_in_global_delay_delete_list(NULL),
											m_size(0)
{
}

unmanaged_resource::~unmanaged_resource ()
{
	if ( m_deleter )
	{
		R_ASSERT							(creation_source() != creation_source_unset);
		LOGI_INFO							("resources:resource", "deleted %s", log_string().c_str() );
		g_resources_manager->unregister_unmanaged_resource(this);
	}
}

void   unmanaged_resource::late_set_fat_it (fat_it_wrapper in_it)
{
	fat_iterator in_fat_it				=	wrapper_to_fat_it(in_it);
	fat_iterator my_fat_it				=	wrapper_to_fat_it(m_fat_it);
	R_ASSERT								(in_it == m_fat_it || my_fat_it.is_end());
	m_fat_it							=	in_it;	
}

void   unmanaged_resource::set_deleter_object (cook_base * const cook, u32 deallocation_thread_id)
{
	bool old_cook_was_present			=	(m_deleter != NULL);
	if ( m_deleter )
		m_deleter->change_cook_users_count	(-1);

	m_deleter							=	cook;
	m_class_id							=	cook ? cook->get_class_id() : raw_data_class;
	m_deallocation_thread_id			=	deallocation_thread_id;

	if ( cook )
	{
		if ( !old_cook_was_present )
			g_resources_manager->register_unmanaged_resource	(this);

		m_deleter->change_cook_users_count	(+1);
	}
	else
		g_resources_manager->unregister_unmanaged_resource		(this);
}

fixed_string512   unmanaged_resource::log_string () const
{ 
#ifndef MASTER_GOLD
	pcstr request_path				=		m_request_path.c_str();
#else
	pcstr request_path				=		"<was not saved>";
#endif

	fixed_string512							out_result;
	if ( creation_source() == creation_source_physical_path )
	{
		out_result.assignf					("unmanaged resource with physical path: '%s', size = %d", request_path, m_size);
		return								out_result;
	}
	if ( creation_source() == creation_source_user_data )
	{
		out_result.assignf					("unmanaged resource created from user-data with name '%s', size = %d", request_path, m_size);
		return								out_result;
	}
	
	if ( creation_source() == creation_source_created_by_user )
	{
		out_result.assignf					("unmanaged resource created by user: '%s', size = %d", request_path, m_size);
		return								out_result;
	}

	if ( creation_source() == creation_source_deallocate_buffer_helper )
	{
		out_result.assignf					("unmanaged resource buffer created by user: '%s', size = %d", request_path, m_size);
		return								out_result;
	}

	file_system::iterator fat_it		=	wrapper_to_fat_it(m_fat_it);
	if ( fat_it.is_end() )
	{
		if ( creation_source() == creation_source_translate_query )
			out_result.assignf				("unmanaged resource via translate_query: '%s', size = %d", request_path, m_size);
		else
			out_result.assignf				("unmanaged resource generated because file was not found: '%s', size = %d", request_path, m_size);

		return								out_result;
	}

	R_ASSERT								(creation_source() == creation_source_fat ||
											 creation_source() == creation_source_translate_query);
	out_result.assignf						("unmanaged resource '%s', size = %d", fat_it.get_full_path().c_str(), m_size);

	return									out_result;
}

void   unmanaged_resource::set_creation_source (creation_source_enum creation_source, pcstr request_path, u32 resource_size)
{ 
	R_ASSERT								(m_creation_source == creation_source_unset);
	m_creation_source					=	creation_source;
	m_size								=	resource_size;
	XRAY_UNREFERENCED_PARAMETER				(request_path);
#ifndef MASTER_GOLD
	m_request_path						=	request_path;
#endif
}

void   unmanaged_resource::set_as_inlined_in_fat ()
{
	m_inlined_in_fat					=	true;
	threading::interlocked_increment		(m_reference_count);
}

void   unmanaged_resource::unset_as_inlined_in_fat ()
{
	threading::interlocked_decrement		(m_reference_count);
	m_inlined_in_fat					=	false;
}

} // namespace resources
} // namespace xra
