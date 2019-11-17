////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCES_DEVICE_MANAGER_H_INCLUDED
#define RESOURCES_DEVICE_MANAGER_H_INCLUDED

#include "resources_helper.h"
#include "xray/threading_mutex.h"
#include <xray/intrusive_list.h>

namespace xray {
namespace resources {

class device_manager
{
public:
							device_manager				(pcstr thread_desc, u32 sector_size);
	virtual					~device_manager				();

	virtual bool			can_handle_query			(pcstr	path)	=	0;
	void					update						();

private:
	void					mark_unmovables_before_defragmentation	();
	void					push_delayed_query  		(query_result *);
	void					push_query					(query_result *);
	virtual void			push_query_impl				(query_result *) = 0;
	virtual query_result*	pop_next_query				() = 0;
	void					fill_pre_allocated			();
	bool					pre_allocate				(query_result * query);
	void					async_file_operation_thread ();
	void					async_file_operation_tick	();
	void					push_current_query_to_finished_queue();
	query_result *			pop_front_preallocated		();
	void					on_pre_allocated_size_change(int size_delta, query_result * query);
	void					close_file					();
	
	threading::atomic32_type	m_async_file_operation_thread_must_exit;
	threading::atomic32_type	m_async_file_operation_thread_exited;
	threading::event		m_async_file_operation_thread_has_work_event;
	threading::event		m_async_file_operation_finished_event;
	threading::mutex		m_pre_allocated_size_mutex;
	int						m_pre_allocated_size;

	threading::mutex		m_pre_allocated_mutex;

	// note:
	// when amount of data for device thread is less then m_min_wanted_pre_allocated_size
	// resource manager thread preallocates more data
	u32						m_min_wanted_pre_allocated_size;

	typedef	associative_vector< u32, u32, vector >	thread_id_to_num_queries;
	thread_id_to_num_queries						m_thread_id_to_num_queries;

	typedef	intrusive_list<query_result, query_result, &query_result::m_next_in_device_manager, threading::single_threading_policy>	queries_list;
	queries_list			m_pre_allocated;

 	query_result *			m_query;
	bool					m_ready;
	void *					m_file_handle;
	int						m_overlapped[(3*sizeof(pvoid) + max<2*sizeof(u32),sizeof(pvoid)>::value)/sizeof(int) ];		// 4*5 == 20 == sizeof(OVERLAPPED)
	u32						m_min_bytes_to_operate;

	queries_list			m_finished;

	bool    				open_file						();
	bool					do_async_operation				(mutable_buffer out_data, u64 const file_pos, bool sector_aligned);
	bool					process_read_query				();
	bool					process_write_query				();

	bool					m_need_fill_pre_allocated;
	bool					m_doing_async_write;	
	u32						m_sector_size;
	u32						get_sector_size					() { return m_sector_size; }

	enum					{ max_sector_size	=	4096 };
	char					m_sector_data_first[max_sector_size];
	char					m_sector_data_last[max_sector_size];
	file_size_type			m_sector_data_last_file_pos;
	fs::path_string			m_last_file_name;

	friend	class			query_result;
	friend	class			resources_manager;
};

namespace detail
{
fs::path_string				make_query_logging_string		(query_result* query);

} // namespace detail


} // namespace resources
} // namespace xray

#endif // RESOURCES_DEVICE_MANAGER_H_INCLUDED