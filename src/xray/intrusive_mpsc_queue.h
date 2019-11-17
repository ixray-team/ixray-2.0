////////////////////////////////////////////////////////////////////////////
//	Created		: 14.12.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef INTRUSIVE_MPSC_QUEUE_H_INCLUDED
#define INTRUSIVE_MPSC_QUEUE_H_INCLUDED

#include <xray/intrusive_spsc_queue.h>

namespace xray {

template < typename T, typename BaseWithMember, T* BaseWithMember::*MemberNext >
class intrusive_mpsc_queue : private intrusive_spsc_queue<T,BaseWithMember,MemberNext, typename threading::pointer_type<T*>::result > {
public:
	inline			intrusive_mpsc_queue	( ) { }
	inline			~intrusive_mpsc_queue	( ) { }
	inline	void	push_back				( T* const value );
	inline	void	push_null_node			( T* const null_node );
	inline	T*		pop_null_node			( );
	
private:
	typedef intrusive_spsc_queue<T,BaseWithMember,MemberNext, typename threading::pointer_type<T*>::result>	super;

public:
	using super::pop_front;
	using super::empty;
	using super::set_push_thread_id;
	using super::push_null_node;
	using super::set_pop_thread_id;
	using super::pop_null_node;
}; // class intrusive_spsc_queue

} // namespace xray

#include <xray/intrusive_mpsc_queue_inline.h>

#endif // #ifndef INTRUSIVE_MPSC_QUEUE_H_INCLUDED