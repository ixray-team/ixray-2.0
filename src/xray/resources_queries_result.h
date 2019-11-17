////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_QUERIES_RESULT_H_INCLUDED
#define XRAY_RESOURCES_QUERIES_RESULT_H_INCLUDED

#include <xray/resources_query_result.h>

namespace xray {
namespace resources {

class XRAY_CORE_API queries_result
{
public:
	u32						size						() const;
	bool					empty						() const;

	memory::base_allocator *	get_user_allocator		() const { return m_allocator; }

	query_result_for_user &			operator []			(int index);
	query_result_for_user const &	operator []			(int index) const;

	bool					is_failed					() const;
	bool					is_successful				() const;
	long					unique_id					() const { return m_unique_id; }
	void					set_unique_id				(long new_id) { m_unique_id = new_id; }
	
	query_result_for_cook *	get_parent_query			() const { ASSERT(m_parent_query); return m_parent_query; }

private:
	typedef		query_result::push_to_ready_queries_bool	push_to_ready_queries_bool;

							queries_result				(u32						num_queries, 
														 query_callback				callback,
														 memory::base_allocator*	allocator,
														 u32						user_thread_id,
														 query_result_for_cook*		parent);
							~queries_result				();

	void    				on_child_query_end			(query_result* res, bool result, push_to_ready_queries_bool);
	void    				on_query_end				(bool result, push_to_ready_queries_bool);
	void					call_user_callback			();
	void					set_result					(bool result) { m_result = result; }
	query_result &			at							(u32 const index)		{ R_ASSERT(index < m_size); return m_queries[index]; }
	query_result const &	at							(u32 const index) const { R_ASSERT(index < m_size); return m_queries[index]; }
	void					translate_request_paths		();
	void					translate_query_if_needed	();

private:
	query_callback			m_callback;
	query_result_for_cook *	m_parent_query;
	queries_result *		m_next_ready;
	memory::base_allocator*	m_allocator;
	long					m_unique_id;
	u32						m_size;
	u32						m_thread_id;
	bool					m_result;

#if defined(_MSC_VER)
#	pragma warning (push)
#	pragma warning (disable:4200)
#endif // #if defined(_MSC_VER)
	query_result			m_queries[];
#if defined(_MSC_VER)
#	pragma warning (pop)
#endif // #if defined(_MSC_VER)

	friend					class						query_result;
	friend					class						resources_manager;
	friend					class						thread_local_data;
	friend					class						query_resources_helper;
};

} // namespace resources
} // namespace xray

#endif // #ifndef XRAY_RESOURCES_QUERIES_RESULT_H_INCLUDED