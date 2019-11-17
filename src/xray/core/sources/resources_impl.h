////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCES_IMPL_H_INCLUDED
#define RESOURCES_IMPL_H_INCLUDED

#include "resources_helper.h"

#include <xray/resources_resource.h>
#include <xray/resources.h>

namespace xray {
namespace resources {

void				initialize			();
void				mount_mounts_path	(pcstr mounts_path);
bool				is_initialized		();
void				finalize			();

} // namespace xray
} // namespace resources

#endif // #ifndef RESOURCES_IMPL_H_INCLUDED