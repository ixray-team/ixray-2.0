////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_manager.h"
#include <xray/resources_queries_result.h>

namespace xray {
namespace resources {

//----------------------------------------------------------
// queries_result
//----------------------------------------------------------

queries_result::queries_result (u32 const						num_queries, 
								query_callback const			callback, 
								memory::base_allocator* const	allocator,
								u32	const						user_thread_id,
								query_result_for_cook* const	parent) 
								:	m_size(num_queries), 
									m_callback(callback), 
									m_result(false),
									m_allocator(allocator),
									m_thread_id(threading::current_thread_id()),
									m_next_ready(NULL),
									m_parent_query(parent)
{
	for ( u32 i=0; i<num_queries; ++i )
	{
		new ( &m_queries[i] ) query_result(query_result::flag_load, this, allocator, user_thread_id);
	}

	m_unique_id							=	g_resources_manager->next_unique_id_for_queries();
	g_resources_manager->on_added_queries	(m_size);
}

queries_result::~queries_result () 
{
	for ( u32 i=0; i<m_size; ++i )
	{
//#pragma message(XRAY_TODO("remove this after debugging"))
//		LOGI_INFO				("urgent", "on removing query: %s", m_queries[i].get_requested_path());

		m_queries[i].~query_result();
	}

	g_resources_manager->on_dispatched_queries(m_size);
}

u32   queries_result::size () const
{ 
	return m_size;			
}

// will never return true :)
bool queries_result::empty () const
{ 
	return !m_size;
}

query_result_for_user&   queries_result::operator [] (int index)			
{
	ASSERT	(index<(int)m_size); 
	return	m_queries[index];	
}

const query_result_for_user&   queries_result::operator [] (int index) const	
{
	ASSERT	(index<(int)m_size); 
	return	m_queries[index];	
}

bool   queries_result::is_failed () const
{
	return	!m_result;			
}

bool   queries_result::is_successful () const
{
	return	m_result;
}

void   queries_result::on_child_query_end (query_result * const					child, 
										   bool const							result, 
										   push_to_ready_queries_bool const		push_to_ready_queries)
{
	XRAY_UNREFERENCED_PARAMETERS(child, result);
	bool		res			=	true;

	for ( u32 i=0; i<size(); ++i )
	{
		query_result const & cur	=	at(i);
		if ( !cur.is_finished() )
			return;

		res					&=	cur.is_success();
	}

	on_query_end				(res, push_to_ready_queries);
}

void   queries_result::on_query_end (bool const							result, 
									 push_to_ready_queries_bool const	push_to_ready_queries)
{
	m_result					=	result;
	if ( push_to_ready_queries == query_result::push_to_ready_queries_true )
		g_resources_manager->on_query_finished	(this);
}

void   queries_result::call_user_callback ()
{
	m_callback						(*this);
}

void   queries_result::translate_request_paths ()
{
	for ( u32 i=0; i<size(); ++i )
	{
		query_result & query		=	at(i);
		if ( query.creation_data_from_user() )
			continue;

		query.translate_request_path	();
	}
}

void   queries_result::translate_query_if_needed ()
{
	for ( u32 i=0; i<size(); ++i )
	{
		query_result & query			=	at(i);		
		query.translate_query_if_needed		();
	}
}

} // namespace xray
} // namespace resources