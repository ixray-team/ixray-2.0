////////////////////////////////////////////////////////////////////////////
//	Created 	: 18.03.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_INTRUSIVE_DOUBLE_LINKED_LIST_H_INCLUDED
#define XRAY_INTRUSIVE_DOUBLE_LINKED_LIST_H_INCLUDED

#include <xray/threading_policies.h>

namespace xray {

template <	class T, 
			class BaseWithMember,
			T* BaseWithMember::*MemberPrev, 
			T* BaseWithMember::*MemberNext, 
			class Policy = threading::multi_threading_mutex_policy	
		 >
class intrusive_double_linked_list : private boost::noncopyable {
public:
	typedef				Policy				policy;
	typedef				T					data_type;

public:
	inline				intrusive_double_linked_list	();

	inline	void		swap							(intrusive_double_linked_list & other);
	inline	void		clear							();

	inline	void		push_back						(T * object, bool * out_pushed_first = NULL);
	inline	void		push_front						(T * object, bool * out_pushed_first = NULL);

	inline	T *			pop_back						(bool * out_popped_last = NULL);
	inline	T *			pop_front						(bool * out_popped_last = NULL);
	inline	T *			pop_all_and_clear				();
	inline	T *			pop_back_and_clear				();
	inline	void		erase							(T * object);
	inline	T *			front							() const;
	inline	T *			back							() const;
	inline	bool		empty							() const;

	template <class Predicate>
	inline	void		for_each						(Predicate const & pred);
	template <class Predicate>
	inline	void		for_each						(Predicate & pred);

	static T *			get_next_of_object				(T * object) { return object->*MemberNext; }
	static T *			get_prev_of_object				(T * object) { return object->*MemberPrev; }

	Policy &			threading_policy				() const { return m_threading_policy; }

private:
	template <class Predicate>
	class predicate_ref : private boost::noncopyable
	{
	public:
			predicate_ref	(Predicate & pred) : m_predicate_ref(pred) {}
	void	operator ()		(T * arg) const { m_predicate_ref(arg); }
	private:
		Predicate &	m_predicate_ref;
	}; // class predicate_ref

	mutable Policy		m_threading_policy;
	T * 				m_first;
	T *					m_last;

};

} // namespace xray

template <class T, class BaseWithMember, T* BaseWithMember::*MemberPrev, T* BaseWithMember::*MemberNext, class Policy>
inline	void	swap	(xray::intrusive_double_linked_list<T, BaseWithMember, MemberPrev, MemberNext, Policy>& left, 
						 xray::intrusive_double_linked_list<T, BaseWithMember, MemberPrev, MemberNext, Policy>& right);

#include <xray/intrusive_double_linked_list_inline.h>

#endif // #ifndef XRAY_INTRUSIVE_DOUBLE_LINKED_LIST_H_INCLUDED