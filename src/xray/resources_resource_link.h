////////////////////////////////////////////////////////////////////////////
//	Created		: 04.06.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCES_RESOURCE_LINK_H_INCLUDED
#define RESOURCES_RESOURCE_LINK_H_INCLUDED

#include <xray/intrusive_list.h>

namespace xray {
namespace resources {

class resource_base;
class child_resource_ptr_base;

class XRAY_CORE_API resource_link 
{
public:
	union
	{
		resource_base *				parent;
		child_resource_ptr_base  *	child_member;
	};
	resource_link *					next_link;
	
	resource_link	() : parent(NULL), next_link(NULL) { }

}; // class resource_link

typedef		intrusive_list< resource_link, resource_link, & resource_link::next_link, 
							threading::multi_threading_interlocked_policy >
			resource_link_list;

template class XRAY_CORE_API	intrusive_list< resource_link, resource_link, & resource_link::next_link, 
								threading::multi_threading_interlocked_policy >;

} // namespace resources
} // namespace xray

#endif // #ifndef RESOURCES_RESOURCE_LINK_H_INCLUDED