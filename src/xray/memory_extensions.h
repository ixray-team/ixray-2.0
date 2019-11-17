////////////////////////////////////////////////////////////////////////////
//	Created 	: 02.09.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MEMORY_EXTENSIONS_H_INCLUDED
#define XRAY_MEMORY_EXTENSIONS_H_INCLUDED

#include <xray/macro_extensions.h>
#include <xray/type_extensions.h>
#include <xray/debug_extensions.h>

#if XRAY_PLATFORM_WINDOWS
#	define XRAY_MAX_CACHE_LINE_SIZE		64
#elif XRAY_PLATFORM_XBOX_360 // #if XRAY_PLATFORM_WINDOWS
#	define XRAY_MAX_CACHE_LINE_SIZE		128
#elif XRAY_PLATFORM_PS3 // #elif XRAY_PLATFORM_XBOX_360
#	define XRAY_MAX_CACHE_LINE_SIZE		128
#else // #elif XRAY_PLATFORM_PS3
#	error please define your platform!
#endif // #if XRAY_PLATFORM_WINDOWS

#define XRAY_MAX_CACHE_LINE_PAD			char XRAY_STRING_CONCAT(m_cache_line_pad_$, __LINE__) [XRAY_MAX_CACHE_LINE_SIZE]

#define XRAY_USE_CRT_MEMORY_ALLOCATOR	0

#if XRAY_USE_CRT_MEMORY_ALLOCATOR || defined(XRAY_MAYA_BUILDING) || defined(__SNC__)
#	include <xray/memory_crt_allocator.h>
#endif // #if XRAY_USE_CRT_MEMORY_ALLOCATOR

#define XRAY_DEFAULT_ALIGN_SIZE			8
#define XRAY_DEFAULT_ALIGN				XRAY_ALIGN(XRAY_DEFAULT_ALIGN_SIZE)

#include <xray/memory_pthreads3_allocator.h>

namespace xray {
namespace memory {

typedef pthreads3_allocator						pthreads_allocator_type;
extern XRAY_CORE_API pthreads_allocator_type	g_mt_allocator;

} // namespace memory
} // namespace xray

#include <xray/memory_override_operators.h>
#include <xray/memory_doug_lea_allocator.h>
#include <xray/memory_allocator_helper.h>
#include <xray/memory_macros.h>
#include <xray/memory_buffer.h>
#include <xray/uninitialized_reference.h>
#include <fastdelegate/fastdelegate.h>

namespace xray {

enum {
	Kb	= u32(1024),
	Mb	= 1024*Kb,
};

namespace memory {

namespace debug {
	enum {
		underrun_guard			= 0xffbdFDFD,
		overrun_guard			= 0xffbdFDFD,
		uninitialized_memory	= 0xffbdCDCD,
		freed_memory			= 0xffbdDDDD,
	};
} // namespace debug

XRAY_CORE_API	void allocate_region				( );

typedef doug_lea_allocator							doug_lea_allocator_type;
typedef pthreads3_allocator							pthreads_allocator_type;

extern XRAY_CORE_API pthreads_allocator_type		g_mt_allocator;

typedef fastdelegate::FastDelegate<void ( base_allocator const&, u32, pvoid, u64, pcstr )>	allocator_predicate_type;
XRAY_CORE_API	void iterate_allocators				( allocator_predicate_type const& predicate );
XRAY_CORE_API	void register_allocator				( base_allocator& allocator, u64 arena_size, pcstr description );

XRAY_CORE_API	void dump_statistics				( );
XRAY_CORE_API	void copy							( mutable_buffer const & destination, const_buffer const & source );
XRAY_CORE_API	void copy							( pvoid destination, size_t destination_size, pcvoid source, size_t source_size );

XRAY_CORE_API	void zero							( mutable_buffer const & destination );
XRAY_CORE_API	void zero							( pvoid destination, size_t destination_size );

				template <class T, size_t count>
				void zero							( T (& destination)[count] ) { zero(destination, count * sizeof(T)); }

XRAY_CORE_API	void fill32							( mutable_buffer const & destination, u32 value, u32 count );
XRAY_CORE_API	void fill32							( pvoid destination, size_t destination_size_in_bytes, u32 value, size_t count );
XRAY_CORE_API	void fill8							( mutable_buffer const & destination, u8  value, u32 count );
XRAY_CORE_API	void fill8							( pvoid destination, size_t destination_size_in_bytes, u8  value, size_t count );

XRAY_CORE_API	int  compare						( const_buffer const & buffer1, const_buffer const & buffer2 );
XRAY_CORE_API	bool equal							( const_buffer const & buffer1, const_buffer const & buffer2 );

XRAY_CORE_API	void lock_process_heap				( );
XRAY_CORE_API	bool try_lock_process_heap			( );
XRAY_CORE_API	void unlock_process_heap			( );

} // namespace memory
} // namespace xray

// multithreaded general purpose memory allocation macros
// use ONLY in case when you need multi threaded functionality
// otherwise use specific single threaded memory allocator
#define MT_NEW( type )								XRAY_NEW_IMPL(		::xray::memory::g_mt_allocator, type )
#define MT_DELETE( pointer )						XRAY_DELETE_IMPL(	::xray::memory::g_mt_allocator, pointer )
#define MT_MALLOC( size, description )				XRAY_MALLOC_IMPL(	::xray::memory::g_mt_allocator, size, description )
#define MT_REALLOC( pointer, size, description )	XRAY_REALLOC_IMPL(	::xray::memory::g_mt_allocator, pointer, size, description )
#define MT_FREE( pointer )							XRAY_FREE_IMPL(		::xray::memory::g_mt_allocator, pointer )
#define MT_ALLOC( type, count )						XRAY_ALLOC_IMPL(	::xray::memory::g_mt_allocator, type, count )

#endif // #ifndef XRAY_MEMORY_EXTENSIONS_H_INCLUDED