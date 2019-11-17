////////////////////////////////////////////////////////////////////////////
//	Created		: 23.04.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef LINKAGE_RESOLVER_H_INCLUDED
#define LINKAGE_RESOLVER_H_INCLUDED

#include <xray/linkage_helper.h>

INCLUDE_TO_LINKAGE(animation_entry_point)

INCLUDE_TO_LINKAGE(core_math_sphere)
INCLUDE_TO_LINKAGE(core_geometry_utils)
INCLUDE_TO_LINKAGE(core_os_functions_win)
INCLUDE_TO_LINKAGE(core_debug_platform)

INCLUDE_TO_LINKAGE(engine_entry_point)

INCLUDE_TO_LINKAGE(maya_animation_entry_point)

INCLUDE_TO_LINKAGE(sound_entry_point)
INCLUDE_TO_LINKAGE(sound_library_linkage)

INCLUDE_TO_LINKAGE(core_memory_writer)

#endif // #ifndef LINKAGE_RESOLVER_H_INCLUDED