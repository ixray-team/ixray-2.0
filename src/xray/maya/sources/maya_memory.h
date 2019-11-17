////////////////////////////////////////////////////////////////////////////
//	Created		: 07.05.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <xray/buffer_vector.h>
#include <xray/fixed_vector.h>
#include <xray/associative_vector.h>
#include <xray/hash_multiset.h>

#include <map>
#include <vector>
#include <set>

#include <xray/memory_crt_allocator.h>

namespace xray {
namespace maya {

typedef memory::doug_lea_allocator_type				allocator_type;
extern allocator_type								g_allocator;

#define USER_ALLOCATOR								::xray::maya::g_allocator
#include <xray/std_containers.h>
#undef USER_ALLOCATOR

typedef memory::crt_allocator						crt_allocator_type;
extern crt_allocator_type							g_crt_allocator;

} // namespace maya
} // namespace xray

#define NEW( type )									XRAY_NEW_IMPL(		::xray::maya::g_allocator, type )
#define DELETE( pointer )							XRAY_DELETE_IMPL(	::xray::maya::g_allocator, pointer )
#define MALLOC( size, description )					XRAY_MALLOC_IMPL(	::xray::maya::g_allocator, size, description )
#define REALLOC( pointer, size, description )		XRAY_REALLOC_IMPL(	::xray::maya::g_allocator, pointer, size, description )
#define FREE( pointer )								XRAY_FREE_IMPL(		::xray::maya::g_allocator, pointer )
#define ALLOC( type, count )						XRAY_ALLOC_IMPL(	::xray::maya::g_allocator, type, count )

#define CRT_NEW( type )								XRAY_NEW_IMPL(		::xray::maya::g_crt_allocator, type )
#define CRT_DELETE( pointer )						XRAY_DELETE_IMPL(	::xray::maya::g_crt_allocator, pointer )
#define CRT_MALLOC( size, description )				XRAY_MALLOC_IMPL(	::xray::maya::g_crt_allocator, size, description )
#define CRT_REALLOC( pointer, size, description )	XRAY_REALLOC_IMPL(	::xray::maya::g_crt_allocator, pointer, size, description )
#define CRT_FREE( pointer )							XRAY_FREE_IMPL(		::xray::maya::g_crt_allocator, pointer )
#define CRT_ALLOC( type, count )					XRAY_ALLOC_IMPL(	::xray::maya::g_crt_allocator, type, count )

#endif // #ifndef MEMORY_H_INCLUDED