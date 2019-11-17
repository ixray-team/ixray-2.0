////////////////////////////////////////////////////////////////////////////
//	Created		: 03.06.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_RESOURCE_PTR_H_INCLUDED
#define XRAY_RESOURCES_RESOURCE_PTR_H_INCLUDED

namespace xray {
namespace resources {

template < typename object_type, typename base_type >
class resource_ptr : public intrusive_ptr < object_type, base_type, threading::multi_threading_mutex_policy >
{
	typedef resource_ptr  < object_type, base_type >									self_type;
	typedef intrusive_ptr < object_type, base_type, threading::multi_threading_mutex_policy >	super;

public:
					resource_ptr	() {}
					resource_ptr	(object_type * object);
					resource_ptr	(self_type const & object);
	
	self_type &		operator =		(object_type * object);
	self_type &		operator =		(self_type const & object);
}; // class resource_ptr

class XRAY_CORE_API child_resource_ptr_base
{
protected:
					child_resource_ptr_base	() : m_resource(NULL) {}
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( child_resource_ptr_base )
	void			set_resource_base		(resource_base * resource) { m_resource = resource; }

private:
	resource_base *	get_resource			() const { return m_resource; }

private:
	resource_base *	m_resource;

}; // class child_resource_ptr_base

template < typename object_type, typename base_type >
class child_resource_ptr : public resource_ptr < object_type, base_type >, public child_resource_ptr_base
{
	typedef child_resource_ptr < object_type, base_type >		self_type;
	typedef resource_ptr < object_type, base_type >				super;

public:
			child_resource_ptr	() : m_parent(NULL) {}
			~child_resource_ptr	() { unlink_with_parent_if_needed(); }

			template < class parent_class >
			void	initialize_and_set_parent	(parent_class *									parent,
												 child_resource_ptr < object_type, base_type >	parent_class::*member_pointer_to_child,
												 object_type *									value)
			{
				unlink_with_parent_if_needed		();

				ASSERT								(parent);
				R_ASSERT							((parent->*member_pointer_to_child).c_ptr() == super::c_ptr());
				super::operator =					(value);
				set_resource_base					(value);

				if ( super::c_ptr() )
					super::c_ptr()->link_parent_resource	(parent);
				parent->link_child_resource			(this);
			}

private:
	void	unlink_with_parent_if_needed		()
	{
		if ( m_parent )
		{
			ASSERT									(* this);
			m_parent->unlink_child_resource			(this);
			super::c_ptr()->unlink_parent_resource	(m_parent);
			m_parent							=	NULL;
		}
	}

	void 		operator =		(object_type * ) {}
	void 		operator =		(super const & ) {}
	void 		operator =		(self_type const & ) {}

private:
	resource_base *		m_parent;

}; // class child_resource_ptr

} // namespace resources

template < typename dest_type, typename object_type, typename base_type >
dest_type		static_cast_resource_ptr	(resources::resource_ptr<object_type, base_type> const src_ptr);

} // namespace xray

#include <xray/resources_resource_ptr_inline.h>

#endif // #ifndef XRAY_RESOURCES_RESOURCE_PTR_H_INCLUDED