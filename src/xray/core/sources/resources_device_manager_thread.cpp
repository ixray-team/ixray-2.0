////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_device_manager.h"
#include "resources_allocators.h"
#include "resources_manager.h"

namespace xray {
namespace resources {

using namespace	fs;

void   device_manager::push_current_query_to_finished_queue ()
{
	threading::mutex_raii	raii			(m_pre_allocated_mutex);

	ASSERT									(m_query == m_pre_allocated.front());
	m_pre_allocated.pop_front				();
	if ( m_query->is_load_type() )
		on_pre_allocated_size_change		(-(int)m_query->raw_buffer_size(), m_query);

	m_query->set_raw_is_unmovable			(false);

	m_finished.push_back					(m_query);
	m_query								=	NULL;
	m_file_handle						=	NULL;
	g_resources_manager->wakeup_resources_thread	();
}

void   device_manager::mark_unmovables_before_defragmentation ()
{
	R_ASSERT								(memory::g_resources_managed_allocator.is_defragmenting());
	threading::mutex_raii	raii			(m_pre_allocated_mutex);

	query_result * current_query		=	m_pre_allocated.front();
	u32 queries_left					=	managed_resource_allocator::max_parallel_with_defragmentation_file_operations_allowed 
												/ 
											g_resources_manager->num_device_managers();

	while ( current_query && queries_left )
	{
		current_query->set_raw_is_unmovable	(true);
		current_query					=	m_pre_allocated.get_next_of_object(current_query);
		--queries_left;
	}
}

bool   device_manager::process_write_query ()
{
	const_buffer			src_data;
	file_size_type			file_pos	=	0;
	bool const	is_save_query			=	m_query->has_flag(query_result::flag_save);
	if ( is_save_query )
	{
		src_data						=	m_query->generated_data_to_save();
		R_ASSERT							(src_data);
	}
	else
	{
		fat_iterator const fat_it		=	wrapper_to_fat_it(m_query->get_fat_it());
		file_pos						=	fat_it.get_file_offs();
		src_data						=	m_query->pin_compressed_or_raw_file();
	}

	bool const out_result				=	do_async_operation(cast_away_const(src_data), file_pos, false);

	if ( !is_save_query )
		m_query->unpin_compressed_or_raw_file	(src_data);

	return									out_result;
}

bool   device_manager::process_read_query ()
{
	if ( m_sector_data_last_file_pos !=	(file_size_type)-1 )
		m_query->append_data_if_needed		(memory::buffer(m_sector_data_last), m_sector_data_last_file_pos);

	fat_iterator fat_it					=	wrapper_to_fat_it(m_query->get_fat_it());
	file_size_type const file_pos		=	fat_it.get_file_offs() + m_query->loaded_bytes();
	file_size_type const file_end		=	fat_it.get_file_offs() + m_query->compressed_or_raw_file_size();

	file_size_type current_pos			=	file_pos;

	bool result							=	true;
	if ( current_pos % m_sector_size )
	{
		file_size_type const first_sector_pos	=	math::align_down(current_pos, (file_size_type)m_sector_size);
		mutable_buffer const sector_data	(m_sector_data_first, m_sector_size);
		result							=	do_async_operation(sector_data, first_sector_pos, true);
		if ( result )
			m_query->append_data_if_needed	(sector_data, first_sector_pos);

		current_pos						=	first_sector_pos + m_sector_size;
	}

	file_size_type const last_sector_pos	=	math::align_down(file_end, (file_size_type)m_sector_size);
	if ( result && current_pos + m_sector_size < file_end )
	{
		mutable_buffer const file_data	=	cast_away_const(m_query->pin_compressed_or_raw_file());
		u32 const offs_in_file			=	u32(current_pos - file_pos);
		u32 const read_size				=	u32(last_sector_pos - current_pos);
		mutable_buffer const dest_data	=	file_data.slice(offs_in_file, read_size);
		result							=	do_async_operation(dest_data, current_pos, true);
		if ( result )
			m_query->add_loaded_bytes		(read_size);

		current_pos						=	last_sector_pos;
		m_query->unpin_compressed_or_raw_file	(file_data);
	}

	if ( result && current_pos < file_end )
	{
		mutable_buffer const sector_data	(m_sector_data_last, m_sector_size);
		do_async_operation					(sector_data, current_pos, true);

		m_query->append_data_if_needed		(sector_data, current_pos);
	}

	R_ASSERT								(m_query->file_loaded());
	return									result;
}

void   device_manager::async_file_operation_tick ()
{
	while ( !m_pre_allocated.empty() )
	{
		m_query							=	m_pre_allocated.front();
		if ( !open_file() )
		{
			push_current_query_to_finished_queue	();
			continue;
		}

		m_query->set_raw_is_unmovable		(true);

		bool const operation_result		=	m_query->is_load_type() ? 
											process_read_query() : 
											process_write_query();

	 	LOGI_INFO							("resources:device_manager", "async %s %s %s",
											 m_query->is_load_type() ? "read" : "write",
											 detail::make_query_logging_string (m_query).c_str(),
											 operation_result ? "successfull" : "failed");

		close_file							();
		push_current_query_to_finished_queue	();
	}
}

void   device_manager::async_file_operation_thread ()
{
	while ( !m_async_file_operation_thread_must_exit )
	{
		m_async_file_operation_thread_has_work_event.wait	(threading::event::wait_time_infinite);
		if ( m_async_file_operation_thread_must_exit )
			break;
		
		async_file_operation_tick				();
	}

	threading::interlocked_exchange				(m_async_file_operation_thread_exited, true);
}

} // namespace resources
} // namespace xray