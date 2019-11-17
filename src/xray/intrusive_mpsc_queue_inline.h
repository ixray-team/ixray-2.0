////////////////////////////////////////////////////////////////////////////
//	Created		: 14.12.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef INTRUSIVE_MPSC_QUEUE_INLINE_H_INCLUDED
#define INTRUSIVE_MPSC_QUEUE_INLINE_H_INCLUDED

namespace xray {

template < typename T, typename BaseWithMember, T* BaseWithMember::*MemberNext >
inline void intrusive_mpsc_queue<T,BaseWithMember,MemberNext>::push_back		( T* const value )
{
	R_ASSERT_CMP			( super::m_head, !=, 0, "do not call push_back after pop_null_node called" );
	R_ASSERT_CMP			( super::m_tail, !=, 0, "unexpected situation" );

	value->*MemberNext		= 0;
	while ( threading::interlocked_compare_exchange_pointer( (pvoid&)(super::m_head->*MemberNext), value, 0) ) ;
	threading::interlocked_exchange_pointer	( reinterpret_cast<threading::atomic_ptr_type&>(super::m_head), value );
}

template < typename T, typename BaseWithMember, T* BaseWithMember::*MemberNext >
inline void intrusive_mpsc_queue<T,BaseWithMember,MemberNext>::push_null_node			( T* const null_node )
{
	R_ASSERT_CMP			( super::m_head, ==, 0, "do not call push_null_node when pop_null_node hasn't been called" );
	R_ASSERT_CMP			( super::m_tail, ==, 0, "unexpected situation" );

	null_node->*MemberNext	= 0;
	super::m_head = super::m_tail	= null_node;
}

template < typename T, typename BaseWithMember, T* BaseWithMember::*MemberNext >
inline T* intrusive_mpsc_queue<T,BaseWithMember,MemberNext>::pop_null_node			( )
{
	R_ASSERT_CMP			( super::m_head, !=, 0, "do not call pop_null_node after pop_null_node or before push_null_node called" );
	R_ASSERT_CMP			( super::m_tail, !=, 0, "unexpected situation" );
	R_ASSERT_CMP			( super::m_head, ==, super::m_tail );

	T* const result			= super::m_head;
	super::m_head = super::m_tail	= 0;
	return					result;
}

} // namespace xray

#endif // #ifndef INTRUSIVE_MPSC_QUEUE_INLINE_H_INCLUDED