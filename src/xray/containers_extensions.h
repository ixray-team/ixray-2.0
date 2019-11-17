////////////////////////////////////////////////////////////////////////////
//	Created 	: 01.09.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_CONTAINERS_EXTENSIONS_H_INCLUDED
#define XRAY_CONTAINERS_EXTENSIONS_H_INCLUDED

#include <xray/c_array_functions.h>

#include <xray/buffer_vector.h>
#include <xray/fixed_vector.h>
#include <xray/associative_vector.h>

#include <map>
#include <vector>
#include <list>
#include <set>
#include <deque>

namespace xray {

#include <xray/vectora.h>

namespace mt {

#define USER_ALLOCATOR		::xray::memory::g_mt_allocator
#include <xray/std_containers.h>
#undef USER_ALLOCATOR

} // namespace mt
} // namespace xray

#endif // #ifndef XRAY_CONTAINERS_EXTENSIONS_H_INCLUDED