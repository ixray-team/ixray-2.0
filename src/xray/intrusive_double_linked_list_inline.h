////////////////////////////////////////////////////////////////////////////
//	Created 	: 10.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_INTRUSIVE_DOUBLE_LINKED_LIST_INLINE_H_INCLUDED
#define XRAY_INTRUSIVE_DOUBLE_LINKED_LIST_INLINE_H_INCLUDED

 

#define TEMPLATE_SIGNATURE	template <typename T, class BaseWithMember, T* BaseWithMember::*MemberPrev, T* BaseWithMember::*MemberNext, typename Policy>
#define DOUBLE_LINKED_LIST	xray::intrusive_double_linked_list<T, BaseWithMember, MemberPrev, MemberNext, Policy>

TEMPLATE_SIGNATURE
inline DOUBLE_LINKED_LIST::intrusive_double_linked_list () :
	m_first								(NULL),
	m_last								(NULL)
{
}

TEMPLATE_SIGNATURE
inline void   DOUBLE_LINKED_LIST::swap (intrusive_double_linked_list& other)
{
	m_threading_policy.lock				();
	other.m_threading_policy.lock		();
	
	std::swap							(m_first, other.m_first);
	std::swap							(m_last,  other.m_last);

	other.m_threading_policy.unlock		();
	m_threading_policy.unlock			();
}

TEMPLATE_SIGNATURE
inline void   DOUBLE_LINKED_LIST::clear ()
{
	typename Policy::mutex_raii raii	(m_threading_policy);
	m_first							=	NULL;
	m_last							=	NULL;
}

TEMPLATE_SIGNATURE
inline void   DOUBLE_LINKED_LIST::push_back (T* const object, bool * out_pushed_first)
{
	typename Policy::mutex_raii raii	(m_threading_policy);

	object->*MemberNext				=	NULL;

	if ( out_pushed_first )
		* out_pushed_first			=	!m_first;

	if ( !m_first ) 
	{
		object->*MemberPrev			=	NULL;
		m_first						=	object;
		m_last						=	object;
		return;
	}

	object->*MemberPrev				= 	m_last;
	m_last->*MemberNext				= 	object;
	m_last							= 	object;
}

TEMPLATE_SIGNATURE
inline void   DOUBLE_LINKED_LIST::push_front (T* const object, bool * out_pushed_first)
{
	typename Policy::mutex_raii raii	(m_threading_policy);

	object->*MemberPrev				= 	NULL;

	if ( out_pushed_first )
		* out_pushed_first			=	!m_first;

	if ( !m_first ) 
	{
		object->*MemberNext			= 	NULL;
		m_first						= 	object;
		m_last						= 	object;
		return;
	}

	object->*MemberNext				=	m_first;
	m_first->*MemberPrev			=	object;
	m_first							=	object;
}

TEMPLATE_SIGNATURE
inline T*   DOUBLE_LINKED_LIST::pop_front (bool * out_popped_last)
{
	if ( !m_first )
	{
		if ( out_popped_last )
			* out_popped_last		=	false;
		return							NULL;
	}

	typename Policy::mutex_raii raii	(m_threading_policy);

	if ( m_first )
	{
		T* result					= 	m_first;
		m_first						= 	m_first->*MemberNext;
		if ( m_first )
		{
			m_first->*MemberPrev	=	NULL;
			if ( out_popped_last )
				* out_popped_last	=	false;
		}
		else
		{
			m_last					=	NULL;
			if ( out_popped_last )
				* out_popped_last	=	true;
		}

		result->*MemberNext			=	NULL;
		result->*MemberPrev			=	NULL;
		return							result;
	}
	else
	{
		if ( out_popped_last )
			* out_popped_last		=	false;
	}

	return								NULL;
}

TEMPLATE_SIGNATURE
inline T*   DOUBLE_LINKED_LIST::pop_back (bool * out_popped_last)
{
	if ( !m_first )
	{
		if ( out_popped_last )
			* out_popped_last		=	false;
		return							NULL;
	}

	typename Policy::mutex_raii raii	(m_threading_policy);

	if ( m_first )
	{
		T* const result				= 	m_last;
		m_last						= 	m_last->*MemberPrev;
		if ( m_last )
		{
			m_last->*MemberNext		=	NULL;
			if ( out_popped_last )
				* out_popped_last	=	false;
		}
		else
		{
			m_first					=	NULL;
			if ( out_popped_last )
				* out_popped_last	=	true;
		}

		result->*MemberNext			=	NULL;
		result->*MemberPrev			=	NULL;
		return							result;
	}
	else
	{
		if ( out_popped_last )
			* out_popped_last		=	false;
	}

	return								NULL;
}

TEMPLATE_SIGNATURE
inline T*   DOUBLE_LINKED_LIST::pop_all_and_clear	()
{
	typename Policy::mutex_raii raii	(m_threading_policy);
	T* result						= 	m_first;
	m_first							= 	NULL;
	m_last							=	NULL;
	return								result;
}

TEMPLATE_SIGNATURE
inline T*   DOUBLE_LINKED_LIST::pop_back_and_clear	()
{
	typename Policy::mutex_raii raii	(m_threading_policy);
	T* result						= 	m_last;
	m_first							=	NULL;
	m_last							= 	NULL;
	return								result;
}

TEMPLATE_SIGNATURE
inline	T*   DOUBLE_LINKED_LIST::front () const
{
	return								m_first;
}

TEMPLATE_SIGNATURE
inline	T*   DOUBLE_LINKED_LIST::back () const
{
	return								m_last;
}

TEMPLATE_SIGNATURE
inline void	  DOUBLE_LINKED_LIST::erase	(T* object)
{
	typename Policy::mutex_raii raii	(m_threading_policy);

	T* const prev					=	object->*MemberPrev;
	T* const next					=	object->*MemberNext;

	object->*MemberPrev				=	NULL;
	object->*MemberNext				=	NULL;

	if ( prev )
	{
		prev->*MemberNext			=	next;
	}
	else 
	{
		ASSERT							(m_first == object);
		m_first						=	next;
	}

	if ( next )
	{
		next->*MemberPrev			=	prev;
	}
	else
	{
		ASSERT							(m_last == object);
		m_last						=	prev;
	}

	object->*MemberNext				=	NULL;
	object->*MemberPrev				=	NULL;
}	

TEMPLATE_SIGNATURE
inline bool   DOUBLE_LINKED_LIST::empty	() const
{
	return								!m_first;
}

TEMPLATE_SIGNATURE
template <class Predicate>
void   DOUBLE_LINKED_LIST::for_each (Predicate const & pred)
{
	typename Policy::mutex_raii raii(m_threading_policy);
	for ( T * current = m_first; current; )
	{
		T * const next	=	get_next_of_object(current);
		pred				(current);
		current			=	next;
	}
}

TEMPLATE_SIGNATURE
template <class Predicate>
void   DOUBLE_LINKED_LIST::for_each (Predicate & pred)
{
	for_each(predicate_ref<Predicate>(pred));
}

TEMPLATE_SIGNATURE
inline void   swap (DOUBLE_LINKED_LIST& left, DOUBLE_LINKED_LIST& right)
{
	left.swap							(right);
}

#undef DOUBLE_LINKED_LIST
#undef TEMPLATE_SIGNATURE

#endif // #ifndef XRAY_INTRUSIVE_DOUBLE_LINKED_LIST_INLINE_H_INCLUDED