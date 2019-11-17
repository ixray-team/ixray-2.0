////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_UNCONSTRUCTED_STATIC_H_INCLUDED
#define XRAY_UNCONSTRUCTED_STATIC_H_INCLUDED

#include <xray/debug_pointer_cast.h>

namespace xray {
namespace detail {

template <class Class, class ConstructorClass=int>
class uninitialized_reference_impl
{
public:
	uninitialized_reference_impl	() : m_initialized(false), m_variable((Class &)m_static_memory) {}

	Class *			operator->	() 			{ ASSERT(m_initialized); return & m_variable; }
	Class &			operator *	() 			{ ASSERT(m_initialized); return m_variable; }
	Class const *	operator->	() const	{ ASSERT(m_initialized); return & m_variable; }
	Class const &	operator *	() const	{ ASSERT(m_initialized); return m_variable; }

	Class *			c_ptr		()			{ ASSERT(m_initialized); return & m_variable; }
	Class const *	c_ptr		() const	{ ASSERT(m_initialized); return & m_variable; }

	bool			initialized	() const { return m_initialized; }
	
protected:
	Class *	construction_memory	() 
	{ 
		ASSERT					(!m_initialized); 
		m_initialized		=	true; 
		return					::xray::pointer_cast<Class*>(m_static_memory);
	}

	void	destroy				() 
	{ 
		ASSERT					(m_initialized); 
		m_variable.~Class		(); 
		m_initialized		=	false; 
	}

private:
	void operator =				(uninitialized_reference_impl const&);
	char XRAY_DEFAULT_ALIGN		m_static_memory[ sizeof(Class) ];
	Class &						m_variable;
	bool						m_initialized;

	template <class T>
	struct						friend_helper	{ typedef T type; };

	//typedef typename friend_helper<ConstructorClass>::type	friend_type;
	//friend class friend_helper<ConstructorClass>::type;
};

} // namespace detail 

template <class Class, class ConstructorClass=int>
class uninitialized_reference : public detail::uninitialized_reference_impl<Class, ConstructorClass>
{
};

template <class Class>
class uninitialized_reference<Class, int> : public detail::uninitialized_reference_impl<Class, int>
{
	typedef	detail::uninitialized_reference_impl<Class, int>	super;
public:
	Class*			construction_memory	() { return super::construction_memory	(); }
	void			destroy				() { super::destroy		(); }
};

#define XRAY_CONSTRUCT_REFERENCE(uninitialized_ref, Class)	\
					new ( uninitialized_ref.construction_memory() ) Class

#define XRAY_DESTROY_REFERENCE(uninitialized_ref)			\
					uninitialized_ref.destroy();
					

} // namespace xray

#endif // #ifndef XRAY_UNCONSTRUCTED_STATIC_H_INCLUDED