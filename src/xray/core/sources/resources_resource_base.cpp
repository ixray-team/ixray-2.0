////////////////////////////////////////////////////////////////////////////
//	Created		: 28.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/resources_resource_base.h>
#include <xray/resources_managed_resource.h>
#include <xray/resources_unmanaged_resource.h>
#include "resources_allocators.h"

namespace xray {
namespace resources {

using namespace fs;

//----------------------------------------------------------
// resource_base
//----------------------------------------------------------

resource_base::resource_base(flag_t flag) : m_flag(flag), 
											m_creation_source(creation_source_unset), 
											m_name_registry_entry(NULL),
											m_recalculate_memory_usage_last_frame(0),
											m_next_for_query_finished_callback(NULL)
{
}

resource_base::~resource_base ()
{
}

managed_resource *   resource_base::cast_managed_resource ()
{
	if ( m_flag & is_resource_flag )
		return								static_cast_checked<managed_resource*>(this);

	return									NULL;
}

unmanaged_resource *   resource_base::cast_unmanaged_resource ()
{
	if ( m_flag & is_unmanaged_resource_flag )
		return								static_cast_checked<unmanaged_resource*>(this);

	return									NULL;
}

query_result *   resource_base::cast_query_result ()
{
	if ( m_flag & is_query_result_flag )
		return								static_cast_checked<query_result*>(this);

	return									NULL;
}

base_of_intrusive_base *   resource_base::cast_base_of_intrusive_base ()
{
	if ( managed_resource * const resource = cast_managed_resource() )
		return								resource;
	else if ( unmanaged_resource * const unmanaged_resource = cast_unmanaged_resource() )
		return								unmanaged_resource;

	return									NULL;
}

void   resource_base::set_need_cook ()
{ 
	m_flag								|=	needs_cook_flag;

	managed_resource * const this_as_resource	=	cast_managed_resource();
	if ( !this_as_resource )
		return;

	if ( this_as_resource->m_node )
		this_as_resource->m_node->m_manager_info.need_cook	=	true;
}

fs::path_string   resource_base::reusable_request_name ()
{
	fat_iterator	fat_it;
	bool associated_with_fat			=	false;
	if ( managed_resource * const managed_resource = cast_managed_resource() )
	{
		fat_it							=	wrapper_to_fat_it(managed_resource->get_fat_it());
		if ( !fat_it.is_end() )
			associated_with_fat			=	managed_resource->is_associated_with_fat(managed_resource);
	}
	else if ( unmanaged_resource * const unmanaged_resource = cast_unmanaged_resource() )
	{
		fat_it							=	wrapper_to_fat_it(unmanaged_resource->get_fat_it());
		if ( !fat_it.is_end() )
			associated_with_fat			=	unmanaged_resource->is_associated_with_fat(unmanaged_resource);
	}
	else
		NOT_IMPLEMENTED						(return	"");

	if ( !fat_it.is_end() )
	{
		if ( associated_with_fat )
			return							fat_it.get_full_path().c_str();
	}
	else if ( m_name_registry_entry )
		return								m_name_registry_entry->name;

	return									"";
}

void   resource_base::link_parent_resource (resource_base * parent)
{
	resource_link * const new_link	=	memory::g_resources_links_allocator.allocate();
	CURE_ASSERT							(new_link, return);
	new (new_link)						resource_link;
	new_link->parent				=	parent;
	m_parents_resources.push_back_unique	(new_link);
}

void   resource_base::link_child_resource (child_resource_ptr_base * child)
{
	resource_link * const new_link	=	memory::g_resources_links_allocator.allocate();
	CURE_ASSERT							(new_link, return);
	new (new_link)						resource_link;
	new_link->child_member			=	child;
	m_children_resources.push_back_unique	(new_link);
}

struct erase_resource_link_predicate
{
	erase_resource_link_predicate (resource_base * parent, child_resource_ptr_base * child) 
		:	parent_(parent), child_(child), erased_link_(NULL) {}
	
	bool operator () (resource_link * link)
	{
		if ( link->parent == parent_ || link->child_member == child_ )
		{
			ASSERT					(!erased_link_);
			erased_link_		=	link;
			return					true;
		}

		return					false;
	}

	resource_base *				parent_;
	child_resource_ptr_base *	child_;

	resource_link *				erased_link_;
};

void   resource_base::unlink_parent_resource (resource_base * parent)
{
	erase_resource_link_predicate		erase_predicate(parent, NULL);
	CURE_ASSERT							(m_parents_resources.erase_if(erase_predicate), return);
	ASSERT								(erase_predicate.erased_link_);
	memory::g_resources_links_allocator.deallocate	(erase_predicate.erased_link_);
}

void   resource_base::unlink_child_resource (child_resource_ptr_base * child)
{
	erase_resource_link_predicate		erase_predicate(NULL, child);
	CURE_ASSERT							(m_children_resources.erase_if(erase_predicate), return);
	ASSERT								(erase_predicate.erased_link_);
	memory::g_resources_links_allocator.deallocate	(erase_predicate.erased_link_);
}

u32   resource_base::get_reference_count () const
{
	return	((resource_base *)(this))->cast_base_of_intrusive_base()->reference_count();
}


} // namespace resources
} // namespace xray
