////////////////////////////////////////////////////////////////////////////
//	Created 	: 27.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "memory_platform.h"
#include <sys/memory.h>

using xray::memory::platform::regions_type;

static sys_memory_t	s_memory_id		= 0;
static sys_addr_t s_address			= 0;
static u32 s_arena_size				= 0;
static u32 s_allocated_arena_size	= 0;
static pbyte s_arena_address		= 0;

void xray::memory::platform::set_low_fragmentation_heap		( )
{
}

void xray::memory::platform::free_region					( pvoid const buffer, u64 const buffer_size )
{
	R_ASSERT_CMP					( s_allocated_arena_size, >=, buffer_size );
	s_allocated_arena_size			-= buffer_size;
	if ( s_allocated_arena_size )
		return;

	int error;
	error							= sys_mmapper_unmap_memory( s_address, &s_memory_id );
	R_ASSERT_CMP					( error, ==, CELL_OK );

	error							= sys_mmapper_allocate_memory( s_arena_size, SYS_MEMORY_GRANULARITY_1M, &s_memory_id );
	R_ASSERT_CMP					( error, ==, CELL_OK );
	
	error							= sys_mmapper_free_address( s_address );
	R_ASSERT_CMP					( error, ==, CELL_OK );
}

void xray::memory::platform::calculated_desirable_arena_sizes	(
		u64 const arena_sizes,
		u64& desirable_managed_arena_size,
		u64& desirable_unmanaged_arena_size
	)
{
	u64 const total_memory			= 256*Mb;
	u64	const os_memory				= 70*Mb;
	R_ASSERT_CMP					( total_memory, >, os_memory );
	R_ASSERT_CMP					( total_memory, >, os_memory + arena_sizes );

	u32 const granularity			= 1*Mb;
	float const unmanaged_proportion = 1.f/3.f;
	u64 const free_size				= total_memory - (os_memory + arena_sizes);
	desirable_unmanaged_arena_size	= math::align_up( (u64)(free_size*unmanaged_proportion), (u64)granularity );
	desirable_managed_arena_size	= math::align_up( (u64)(free_size*(1.f-unmanaged_proportion)), (u64)granularity );
}

void xray::memory::platform::allocate_arenas					( regions_type& arenas, regions_type& resource_arenas )
{
	sys_memory_info_t memory_info;
	sys_memory_get_user_memory_size	( &memory_info );
	s_arena_size					= math::align_down( memory_info.available_user_memory, (u32)1*Mb );
	s_arena_size					-= 2*Mb;

	u32 const available_memory_256mb = math::align_up( memory_info.available_user_memory, (u32)256*Mb );

	int error;
	error							=
		sys_mmapper_allocate_address(
			available_memory_256mb,
			SYS_MEMORY_PAGE_SIZE_1M | SYS_MEMORY_ACCESS_RIGHT_PPU_THR | SYS_MEMORY_ACCESS_RIGHT_RAW_SPU,
			256*Mb,
			&s_address
		);
	R_ASSERT_CMP					( error, ==, CELL_OK );

	error							= sys_mmapper_allocate_memory( s_arena_size, SYS_MEMORY_GRANULARITY_1M, &s_memory_id );
	R_ASSERT_CMP					( error, ==, CELL_OK );

	error							= sys_mmapper_map_memory( s_address, s_memory_id, SYS_MEMORY_PROT_READ_WRITE );
	R_ASSERT_CMP					( error, ==, CELL_OK );

	s_arena_address					= *(pbyte*)&s_address;

#ifndef MASTER_GOLD
	memory::fill32					( s_arena_address, s_arena_size, memory::uninitialized_value<u32>(), s_arena_size/sizeof(u32) );
#endif // #ifndef MASTER_GOLD

	u32 current_arena_size			= s_arena_size;
	s_allocated_arena_size			= 0;

	regions_type::iterator i 		= arenas.begin( );
	regions_type::iterator const e 	= arenas.end( );
	for ( ; i != e; ++i ) {
		if ( !(*i).size )
			continue;

		(*i).address				= s_arena_address;
		s_arena_address				+= (*i).size;
		current_arena_size			-= (*i).size;
		s_allocated_arena_size		+= (*i).size;
		R_ASSERT_CMP				( s_arena_size, >=, s_allocated_arena_size );
	}

	// sort resource arenas
	if ( resource_arenas.front().size > resource_arenas.back().size )
		std::swap					( resource_arenas.front(), resource_arenas.back() );

	region& minimum					= resource_arenas.front();
	region& maximum					= resource_arenas.back();
	R_ASSERT						( minimum.size <= maximum.size );

	float const share				= (float)minimum.size / (float)(maximum.size + minimum.size);
	minimum.size					= xray::math::align_down( (u64)(current_arena_size*share), (u64)4*Kb );
	maximum.size					= current_arena_size - minimum.size;
	minimum.address					= s_arena_address;
	maximum.address					= s_arena_address + minimum.size;

	s_allocated_arena_size			+= minimum.size + maximum.size;
	R_ASSERT_CMP					( s_allocated_arena_size, ==, s_arena_size );
}