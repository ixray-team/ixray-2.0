////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_device_manager.h"
#include "resources_macros.h"
#include "resources_managed_allocator.h"
#include "resources_manager.h"
#include "fs_helper.h"
#include <xray/resources_query_result.h>
#include <xray/os_include.h>

namespace xray {
namespace resources {

using namespace	fs;

namespace detail
{

path_string   make_query_logging_string (query_result* query)
{
	path_string file_path				=	get_native_file_path(query, false);

	fat_iterator it						=	wrapper_to_fat_it(query->get_fat_it());
	path_string	full_path				=	it.get_full_path();
	
	if ( it.is_end() )
		full_path.appendf					(" request path: %s", query->get_requested_path());

	path_string		out_result;
	out_result.assignf						( "\"%s\" from \"%s\"", full_path.c_str(), file_path.c_str());
	return									out_result;
}

} // namespace detail

//----------------------------------------------------------
// class device_manager
//----------------------------------------------------------

device_manager::device_manager (pcstr thread_desc, u32 const sector_size) 
								:	m_doing_async_write(false), m_sector_size(sector_size),
									m_async_file_operation_thread_must_exit(false), 
									m_async_file_operation_thread_exited(false),
									m_pre_allocated_size(0),
									m_min_wanted_pre_allocated_size(4*1024),
									m_sector_data_last_file_pos((file_size_type)-1)

{
	m_query								=	NULL;
	m_ready								=	false;
	m_file_handle						=	NULL;
	m_min_bytes_to_operate				=	0;

	if ( !threading::g_debug_single_thread )
	{
		u32 const thread_id				=	threading::spawn(boost::bind(&device_manager::async_file_operation_thread, this), 
															 thread_desc, 
															 thread_desc,
															 0,
															 2 % threading::core_count(),
															 threading::tasks_unaware);
		XRAY_UNREFERENCED_PARAMETER			(thread_id);
		R_ASSERT							(thread_id);
	}
}

device_manager::~device_manager ()
{
	if ( !threading::g_debug_single_thread )
	{
		threading::interlocked_exchange				(m_async_file_operation_thread_must_exit, true);
		m_async_file_operation_thread_has_work_event.set(true);

		while ( !m_async_file_operation_thread_exited )
			threading::yield						();
	}
}

void   device_manager::push_query (query_result * query)
{
	LOGI_INFO								("resources:device_manager", "%s query added for %s", 
											query->is_load_type() ? "read" : "write", detail::make_query_logging_string (query).c_str());

	push_query_impl							(query);
}

void   device_manager::on_pre_allocated_size_change (int size_delta, query_result * const query)
{
	XRAY_UNREFERENCED_PARAMETER				( query );
	R_ASSERT								(query->is_load_type());

	threading::mutex_raii	raii			(m_pre_allocated_size_mutex);
// 	LOGI_INFO								("resources:test", "pre_allocated_size = %d, increment = %d (resource: %s)", 
// 											m_pre_allocated_size, size_delta, detail::make_query_logging_string (query).c_str());
	R_ASSERT_CMP							(m_pre_allocated_size, >=, -size_delta);
	m_pre_allocated_size				+=	size_delta;	
}

bool   device_manager::pre_allocate (query_result* const query)
{
	if ( query->is_load_type() )
	{
		if ( !query->raw_unmanaged_buffer() )
		{
			if ( !query->allocate_raw_managed_resource_if_needed() )
			{
				m_finished.push_back		(query);
				return						false;
			}
		}
	}

	threading::mutex_raii					pre_allocated_mutex_raii(m_pre_allocated_mutex);
	threading::mutex_raii					pre_allocated_size_raii(m_pre_allocated_size_mutex);
	m_pre_allocated.push_back				(query);
	
	if ( query->is_load_type() )
	{
		u32 const raw_buffer_size		=	query->raw_buffer_size();
		on_pre_allocated_size_change		(raw_buffer_size, query);
	}

	return									true;
}

void   device_manager::fill_pre_allocated ()
{
	while ( query_result * const query = pop_next_query() )
	{
		pre_allocate						(query);
		m_async_file_operation_thread_has_work_event.set (true);
	}
}

void   device_manager::update ()
{
	if ( m_pre_allocated_size < (int)m_min_wanted_pre_allocated_size )
		fill_pre_allocated					();

	if ( threading::g_debug_single_thread )
		async_file_operation_tick			();
	
	query_result * finished				=	m_finished.pop_all_and_clear();
	while ( finished )
	{
		if ( finished->is_replication_type() )
		{
			fat_iterator fat_it			=	wrapper_to_fat_it( finished->get_fat_it() );
			fs::g_fat->commit_replication	(fat_it);
			LOGI_INFO						("resources:resource", "replicated %s ", detail::make_query_logging_string (finished).c_str());
		}
		else if ( finished->is_save_type() )
		{
			fs::file_system::iterator		it;
			bool const mounted_node		=	fs::g_fat->mount_disk_node_by_logical_path(finished->get_requested_path(), & it);
			R_ASSERT						(mounted_node);
			
			if ( mounted_node )
			{
				LOGI_INFO					("resources:resource", "written generated resource to file: %s ", finished->get_requested_path());
				finished->late_set_fat_it	(fat_it_to_wrapper(it));
			}
		}

 		query_result * const next_finished	=	finished->m_next_in_device_manager;	

 		finished->on_file_operation_end		();
 		finished						=	next_finished;
	}
}

} // namespace resources
} // namespace xray