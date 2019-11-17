////////////////////////////////////////////////////////////////////////////
//	Created		: 09.06.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_MEMORY_H_INCLUDED
#define XRAY_RESOURCES_MEMORY_H_INCLUDED

namespace xray {
namespace resources {

class memory_type
{
public:
					memory_type		(pcstr name) : m_name(name) {}
private:
	pcstr			m_name;
};

extern memory_type XRAY_CORE_API 	memory_type_non_cacheable_resource;
extern memory_type XRAY_CORE_API	memory_type_managed_resource;
extern memory_type XRAY_CORE_API	memory_type_unmanaged_resource;

class XRAY_CORE_API memory_usage
{
public:
					memory_usage	() : type(0), pool(0), size(0) { ; }
					memory_usage	(memory_type const * type, u32 size, u32 pool = 0) : type(type), size(size), pool(pool) { ; }
public:
	memory_type const *	type;
	u32					pool;
	u32					size;
};

} // namespace resources
} // namespace xray

#endif // #ifndef XRAY_RESOURCES_MEMORY_H_INCLUDED