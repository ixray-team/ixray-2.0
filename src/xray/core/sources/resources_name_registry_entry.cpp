////////////////////////////////////////////////////////////////////////////
//	Created		: 28.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/resources_name_registry_entry.h>
#include "fs_helper.h"

namespace xray {
namespace resources {

namespace detail {

u32   name_registry_hash::operator () (name_registry_entry & entry) const
{
	return	xray::fs::crc32(entry.name, strings::length(entry.name));
}

bool   name_registry_equal::operator () (name_registry_entry const & left, name_registry_entry const & right) const
{
	return	strings::equal(left.name, right.name);
}

} // namespace detail

} // namespace resources
} // namespace xray
