////////////////////////////////////////////////////////////////////////////
//	Created 	: 10.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_INTRUSIVE_LIST_INLINE_H_INCLUDED
#define XRAY_INTRUSIVE_LIST_INLINE_H_INCLUDED

#define TEMPLATE_SIGNATURE	template <typename T, typename BaseWithMember, T* BaseWithMember::*MemberNext, typename Policy>
#define INTRUSIVE_LIST		xray::intrusive_list<T, BaseWithMember, MemberNext, Policy>

TEMPLATE_SIGNATURE
inline INTRUSIVE_LIST::intrusive_list						( ) :
	m_first				( NULL ),
	m_last				( NULL )
{
}

TEMPLATE_SIGNATURE
inline void INTRUSIVE_LIST::swap							( intrusive_list& other )
{
	m_threading_policy.lock			( );
	other.m_threading_policy.lock	( );
	
	std::swap			( m_first, other.m_first );
	std::swap			( m_last,  other.m_last  );

	other.m_threading_policy.unlock	( );
	m_threading_policy.unlock		( );
}

TEMPLATE_SIGNATURE
inline void INTRUSIVE_LIST::clear							( )
{
	typename Policy::mutex_raii raii(m_threading_policy);
	m_first				= NULL;
	m_last				= NULL;
}

TEMPLATE_SIGNATURE
inline void INTRUSIVE_LIST::push_back			(  T* const object, bool * out_pushed_first )
{
	object->*MemberNext	= NULL;

	typename Policy::mutex_raii raii(m_threading_policy);

	if ( out_pushed_first )
		* out_pushed_first	= !m_first;

	if ( !m_first ) {
		m_first			= object;
		m_last			= object;
		return;
	}

	m_last->*MemberNext	= object;
	m_last				= object;
}

TEMPLATE_SIGNATURE
inline void INTRUSIVE_LIST::push_front			(  T* const object, bool * out_pushed_first )
{
	typename Policy::mutex_raii raii(m_threading_policy);

	if ( out_pushed_first )
		* out_pushed_first	= !m_first;

	if ( !m_first ) {
		object->*MemberNext	= NULL;
		m_first				= object;
		m_last				= object;
		return;
	}

	object->*MemberNext	= m_first;
	m_first				= object;
}

TEMPLATE_SIGNATURE
inline T* INTRUSIVE_LIST::pop_front				( bool * out_popped_last )
{
	if ( !m_first )
	{
		if ( out_popped_last )
			* out_popped_last		=	false;
		return			NULL;
	}

	typename Policy::mutex_raii raii(m_threading_policy);

	if ( m_first )
	{
		T* result		= m_first;
		m_first			= m_first->*MemberNext;
		if ( !m_first )
		{
			m_last		= NULL;
			if ( out_popped_last )
				* out_popped_last	=	true;
		}
		else if ( out_popped_last )
				* out_popped_last	=	false;

		result->*MemberNext	=	NULL;
		return			result;
	}
	else
	{
		if ( out_popped_last )
			* out_popped_last		=	false;
	}

	return				NULL;
}

TEMPLATE_SIGNATURE
inline T* INTRUSIVE_LIST::pop_front				( )
{
	if ( !m_first )
		return			NULL;

	typename Policy::mutex_raii raii(m_threading_policy);

	if ( m_first )
	{
		T* result		= m_first;
		m_first			= m_first->*MemberNext;
		if ( !m_first )
			m_last		= NULL;
		result->*MemberNext	=	NULL;
		return			result;
	}

	return				NULL;
}

TEMPLATE_SIGNATURE
inline T* INTRUSIVE_LIST::pop_all_and_clear	( )
{
	typename Policy::mutex_raii raii(m_threading_policy);

	T* result			= m_first;
	m_first				= NULL;
	m_last				= NULL;
	return				result;
}

TEMPLATE_SIGNATURE
inline	T* INTRUSIVE_LIST::front ( ) const
{
	return				m_first;
}

TEMPLATE_SIGNATURE
inline	T* INTRUSIVE_LIST::back ( ) const
{
	return				m_last;
}

TEMPLATE_SIGNATURE
inline bool INTRUSIVE_LIST::empty							( ) const
{
	return				( !m_first );
}

TEMPLATE_SIGNATURE
inline bool INTRUSIVE_LIST::erase	( T* object )
{
	if ( !m_first )
		return			false;

	typename Policy::mutex_raii raii(m_threading_policy);

	T* previous_i	= 0;
	T* i			= m_first;
	for ( ; i && (i != object); previous_i = i, i = get_next_of_object(i) );

	if ( i != object )
		return			false;

	if ( previous_i )
		set_next_of_object(previous_i, get_next_of_object(i));
	else
		m_first					= get_next_of_object(i);

	if ( ! get_next_of_object(i) )
		m_last					= previous_i ? previous_i : m_first;
	return				true;
}

TEMPLATE_SIGNATURE
template <class Predicate>
bool   INTRUSIVE_LIST::erase_if (Predicate & pred)
{
	if ( !m_first )
		return			false;

	typename Policy::mutex_raii raii(m_threading_policy);

	T *	current		=	m_first;
	T * previous	=	NULL;
	while ( current )
	{
		if ( pred(current) )
		{
			if ( previous )
			{
				set_next_of_object	(previous, get_next_of_object(current));
				current	=	get_next_of_object(current);
			}
			else
			{
				m_first	=	get_next_of_object(current);
				current	=	m_first;
			}
		}
		else
		{
			previous	=	current;
			current		=	get_next_of_object(current);
		}
	}
	
	return				true;
}

TEMPLATE_SIGNATURE
template <class Predicate>
void   INTRUSIVE_LIST::for_each (Predicate const & pred)
{
	typename Policy::mutex_raii raii(m_threading_policy);
	for ( T *	current	=	m_first;
				current;
				)
	{
		T * const next	=	get_next_of_object(current);
		pred				(current);
		current			=	next;
	}
}

TEMPLATE_SIGNATURE
template <class Predicate>
void   INTRUSIVE_LIST::for_each (Predicate & pred)
{
	for_each(predicate_ref<Predicate>(pred));
}

TEMPLATE_SIGNATURE
bool   INTRUSIVE_LIST::contains_object_thread_unsafe (T* object)
{
	for ( T *	current	=	m_first;
				current;
				current	=	get_next_of_object(current) )
	{
		if ( current == object )
			return			true;
	}

	return					false;
}

TEMPLATE_SIGNATURE
bool   INTRUSIVE_LIST::push_back_unique	(T * object, bool * out_pushed_first)
{
	typename Policy::mutex_raii raii(m_threading_policy);
	if ( !contains_object_thread_unsafe(object) )
	{
		push_back			(object, out_pushed_first);
		return				true;
	}

	return					false;
}

TEMPLATE_SIGNATURE
bool   INTRUSIVE_LIST::push_front_unique (T * object, bool * out_pushed_first)
{
	typename Policy::mutex_raii raii(m_threading_policy);
	if ( !contains_object_thread_unsafe(object) )
	{
		push_front			(object, out_pushed_first);
		return				true;
	}

	return					false;
}

TEMPLATE_SIGNATURE
inline void swap											( INTRUSIVE_LIST& left, INTRUSIVE_LIST& right )
{
	left.swap			( right );
}

#undef INTRUSIVE_LIST
#undef TEMPLATE_SIGNATURE

#endif // #ifndef XRAY_INTRUSIVE_LIST_INLINE_H_INCLUDED