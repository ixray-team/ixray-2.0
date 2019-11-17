////////////////////////////////////////////////////////////////////////////
//	Created 	: 25.09.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "memory_platform.h"

#define _WIN32_WINNT 0x0500
#include <xray/os_include.h>

static pvoid s_single_block_arena		= 0;
static u64 s_single_block_arena_size	= 0;

#if 0
static void printf_all								( pcstr format, ... )
{
	char				temp[4096];
	va_list				arguments;
	va_start			(arguments,format);
	vsprintf_s			(temp, sizeof(temp), format, arguments);
	xray::debug::output	(temp);
}

static pcstr protect_to_string						( int const protection )
{
	switch (protection & 0xff) {
		case 0x00000001 : return	"PAGE_NOACCESS";
		case 0x00000002 : return	"PAGE_READONLY";
		case 0x00000004 : return	"PAGE_READWRITE";
		case 0x00000008 : return	"PAGE_WRITECOPY";
		case 0x00000010 : return	"PAGE_EXECUTE";
		case 0x00000020 : return	"PAGE_EXECUTE_READ";
		case 0x00000040 : return	"PAGE_EXECUTE_READWRITE";
		case 0x00000080 : return	"PAGE_EXECUTE_WRITECOPY";
		default			: return	"";
	}
}

static pcstr protect_modifier_to_string				( int const protection )
{
	switch (protection >> 8) {
		case 0x00000001 : return	"PAGE_GUARD";
		case 0x00000002 : return	"PAGE_NOCACHE";
		case 0x00000004 : return	"PAGE_WRITECOMBINE";
		default			: return	"";
	}
}

static pcstr state_to_string						( int const state )
{
	switch (state) {
		case 0x00001000 : return	"MEM_COMMIT";
		case 0x00002000 : return	"MEM_RESERVE";
		case 0x00010000 : return	"MEM_FREE";
		default         : NODEFAULT(return "");
	}
}

static pcstr type_to_string							( int const type )
{
	switch (type) {
		case 0x00020000 : return	"MEM_PRIVATE";
		case 0x00040000 : return	"MEM_MAPPED";
		case 0x01000000 : return	"MEM_IMAGE";
		default         : return	"";
	}
}

void xray::memory::platform::dump_memory		( )
{
	printf_all		("\n\n");

	MEMORY_BASIC_INFORMATION	memory_info;
//	u32 const		max_address = ( ( u32(1) << 31 ) /**| ( u64(1) << 30 ) /**/);
	u32 count		= 0;
	for (size_t i=0; ; ) {
		size_t		return_value = 
			VirtualQuery(
				(pcvoid)i,
				&memory_info,
				sizeof(memory_info)
			);

		if (!return_value) {
			i		+= 4096;
//			if ( i > max_address )
//				break;

			continue;
		}

		i			+= memory_info.RegionSize;
//		if ( i > max_address )
//			break;

		printf_all	("Region:             %d\n", count);
		printf_all	("Base Address:       0x%08x\n", *(u32*)&memory_info.BaseAddress);
		printf_all	("Allocation Base:    0x%08x\n", *(u32*)&memory_info.AllocationBase);
		printf_all	("Allocation Protect: 0x%08x [%s | %s]\n", memory_info.AllocationProtect, protect_to_string(memory_info.AllocationProtect), protect_modifier_to_string(memory_info.AllocationProtect));
		printf_all	("Region Size:        0x%08x [%d]\n", memory_info.RegionSize, memory_info.RegionSize);
		printf_all	("State:              0x%08x [%s]\n", memory_info.State, state_to_string(memory_info.State));
		printf_all	("Protect:            0x%08x [%s | %s]\n", memory_info.Protect, protect_to_string(memory_info.Protect), protect_modifier_to_string(memory_info.Protect));
		printf_all	("Type:               0x%08x [%s]\n\n", memory_info.Type, type_to_string(memory_info.Type));

		++count;
	}
}
#endif // #if 0

void xray::memory::platform::set_low_fragmentation_heap		( )
{
	if ( xray::debug::is_debugger_present( ) )
		return;

	HMODULE const kernel32		= LoadLibrary( "kernel32.dll" );
	R_ASSERT					( kernel32 );

	typedef BOOL (__stdcall *HeapSetInformation_type) ( HANDLE, HEAP_INFORMATION_CLASS, PVOID, SIZE_T );
	HeapSetInformation_type const heap_set_information = 
		(HeapSetInformation_type)GetProcAddress(kernel32, "HeapSetInformation");

	if ( !heap_set_information )
		return;

	ULONG HeapFragValue			= 2;
	{
		BOOL const result		= 
			heap_set_information(
				GetProcessHeap(),
				HeapCompatibilityInformation,
				&HeapFragValue,
				sizeof(HeapFragValue)
			);

		ASSERT_U				(result, "can't set process heap low fragmentation");
	}
	{
		BOOL const result		= 
			heap_set_information(
				(pvoid)_get_heap_handle(),
				HeapCompatibilityInformation,
				&HeapFragValue,
				sizeof(HeapFragValue)
			);

		ASSERT_U				(result, "can't set process heap low fragmentation");
	}
}

static u32 allocation_granularity				( )
{
	SYSTEM_INFO					system_info;
	GetSystemInfo				( &system_info );
	return						system_info.dwAllocationGranularity;
}

void xray::memory::platform::calculated_desirable_arena_sizes	(
		u64 const arena_sizes,
		u64& desirable_managed_arena_size,
		u64& desirable_unmanaged_arena_size
	)
{
	MEMORYSTATUSEX				memory_status;
	memory_status.dwLength		= sizeof(memory_status);
	GlobalMemoryStatusEx		( &memory_status );

#if XRAY_PLATFORM_WINDOWS_32
	u64 const total_memory		= math::min( u64(4)*1024*Mb, memory_status.ullTotalPhys );
	R_ASSERT_CMP				( memory_status.ullTotalPhys, >=, total_memory );
	u64 const memory_slack		= memory_status.ullTotalPhys - total_memory;
#else // #if XRAY_PLATFORM_WINDOWS_32
	u64 const total_memory		= memory_status.ullTotalPhys;
	u64 const memory_slack		= 0;
#endif // #if XRAY_PLATFORM_WINDOWS_32

	OSVERSIONINFOEX						os_version_info;
	memory::zero						( &os_version_info, sizeof(os_version_info) );
	os_version_info.dwOSVersionInfoSize = sizeof(os_version_info);

	u64 const Mb				= 1024*1024;
	u64	os_memory				= 0;
	bool duplicates_video_memory = true;
	GetVersionEx				( (OSVERSIONINFO*)&os_version_info );
	switch ( os_version_info.dwMajorVersion ) {
		// Windows Server 2003 R2, Windows Server 2003, Windows XP, or Windows 2000
		case 5 : {
			os_memory			= 768*Mb;
			duplicates_video_memory	= false;
			break;
		}
		case 6 : {
			switch ( os_version_info.dwMinorVersion ) {
				// Windows Vista or Windows Server 2008
				case 0 : {
					os_memory	= 384*Mb;
					break;
				}
				// Windows 7 or Windows Server 2008 R2
				case 1 : {
					os_memory	= 512*Mb;//+1256*Mb;
					break;
				}
				//
				default : {
					os_memory	= 512*Mb;
					break;
				}
			}
			duplicates_video_memory	= true;
			break;
		}
		// we are not going to support any earlier versions (even Windows XP is under big question),
		// therefore this value would be for any future versions
		default : {
			os_memory			= 512*Mb;
			duplicates_video_memory	= true;
			break;
		}
	}

	R_ASSERT_CMP				( total_memory, >, os_memory );
	R_ASSERT_CMP				( total_memory, >, os_memory + arena_sizes );

	if ( memory_slack >= os_memory )
		os_memory				= 0;

	u32 const granularity		= allocation_granularity( );

	float const proportion				= 1.f/4.f;
	float const unmanaged_proportion	= 1.f/4.f;
	u64 const free_size			= total_memory - (os_memory + arena_sizes);
 	if ( duplicates_video_memory ) {
		desirable_unmanaged_arena_size	= math::align_up( (u64)(free_size*(1.f - proportion)*unmanaged_proportion), (u64)granularity );
		desirable_managed_arena_size	= math::align_up( (u64)(free_size*(1.f - proportion)*(1.f-unmanaged_proportion)), (u64)granularity );
		return;
	}

	desirable_unmanaged_arena_size	= math::align_up( (u64)(free_size*(1.f - proportion)*unmanaged_proportion + free_size*proportion), (u64)granularity );
	desirable_managed_arena_size	= math::align_up( (u64)(free_size*(1.f - proportion)*(1.f-unmanaged_proportion)), (u64)granularity );
}

static xray::command_line::key	s_fill_arenas_with_garbage("fill_arenas_with_garbage", "", "memory", "fills all the memory in all the arenas with garbage; could slowdown startup significantly!");

pvoid allocate_region							( u64 const size, pvoid const address, u32 const additional_flags, bool assert_on_failure = true )
{
	pvoid const result		= 
		VirtualAlloc	(
			address,
#if XRAY_PLATFORM_WINDOWS_64
			size,
#else // #if XRAY_PLATFORM_WINDOWS_64
			(SIZE_T)size,
#endif // #if XRAY_PLATFORM_WINDOWS_64
			MEM_RESERVE | MEM_COMMIT | additional_flags,
			PAGE_READWRITE // PAGE_EXECUTE_READWRITE
		);
	if ( assert_on_failure )
		R_ASSERT_CMP_U		( result, ==, address );

#ifndef MASTER_GOLD
	if ( result && s_fill_arenas_with_garbage.is_set() )
		xray::memory::fill32( result, (size_t)size, xray::memory::uninitialized_value<u32>(), (size_t)size/sizeof(u32) );
#endif // #ifndef MASTER_GOLD

	return					result;
}

void xray::memory::platform::free_region		( pvoid buffer, u64 const buffer_size )
{
	if ( s_single_block_arena_size ) {
		R_ASSERT_CMP			( s_single_block_arena_size, >=, buffer_size );
		s_single_block_arena_size	-= buffer_size;
		if ( s_single_block_arena_size )
			return;

		buffer					= s_single_block_arena;
	}

	BOOL result					= 
		VirtualFree				(
			buffer,
			0,
			MEM_RELEASE
		);
	XRAY_UNREFERENCED_PARAMETER	( result );
	R_ASSERT					(result);
}

template < typename P >
void iterate_regions		( u32 const start_address, u32 const allocation_granularity, u64 const min_buffer_size, P& predicate )
{
	MEMORY_BASIC_INFORMATION	memory_info;
	u64 i						= start_address;
#if !XRAY_PLATFORM_WINDOWS_64
	u64 const max_address		= u64(1) << 32;
#else // #if XRAY_PLATFORM_WINDOWS_32
	u64 const max_address		= u64(1) << 40;
#endif // #if XRAY_PLATFORM_WINDOWS_32

	for ( ; i < max_address; ) {
		size_t const			return_value =
			VirtualQuery(
				( pcvoid )i,
				&memory_info,
				sizeof( memory_info )
			);

		if ( !return_value ) {
			i					+= allocation_granularity;
			continue;
		}

		if ( memory_info.RegionSize < min_buffer_size ) {
			i					+= xray::math::align_up( memory_info.RegionSize, (SIZE_T)allocation_granularity );
			continue;
		}

		i						+= xray::math::align_up( memory_info.RegionSize, (SIZE_T)allocation_granularity );

		if (memory_info.State != MEM_FREE)
			continue;

		predicate				( memory_info.RegionSize, memory_info.BaseAddress );
	}
}


static u64 select_best_region	(
		u64 largest_but_two,
		u64 largest_but_one,
		u64 largest,
		xray::memory::platform::regions_type& resource_arenas,
		u64 const allocation_granularity
	)
{
	xray::memory::platform::region& minimum	= resource_arenas.front();
	xray::memory::platform::region& maximum	= resource_arenas.back();
	R_ASSERT					( minimum.size <= maximum.size );

	if ( largest < largest_but_one )
		std::swap				( largest, largest_but_one );

	if ( largest_but_one < largest_but_two )
		std::swap				( largest_but_one, largest_but_two );

	if ( largest >= (maximum.size + minimum.size) )
		return					maximum.size + minimum.size;

	if ( (largest >= maximum.size) && (largest_but_one >= minimum.size) )
		return					maximum.size + minimum.size;

	if ( largest*((float)minimum.size/float(minimum.size + maximum.size)) >= largest_but_one )
		return					largest;

	float const share			= (float)minimum.size / maximum.size;
	u64 const test_size			= xray::math::align_up( (u64)(largest_but_one*share), allocation_granularity );
	if ( test_size > largest )
		return					largest + xray::math::align_up( (u64)(largest*share), allocation_granularity );

	return						largest_but_one + test_size;
}

static xray::memory::platform::regions_type::iterator& select_best_region(
		u64 const largest_but_two_size,
		xray::memory::platform::regions_type::iterator& largest_but_one,
		xray::memory::platform::regions_type::iterator& largest,
		u64 const size,
		xray::memory::platform::regions_type& resource_arenas,
		u32 const allocation_granularity
	)
{
	u64 const result0			= select_best_region( largest_but_two_size, (*largest_but_one).size - size, (*largest).size, resource_arenas, allocation_granularity );
	u64 const result1			= select_best_region( largest_but_two_size, (*largest_but_one).size, (*largest).size - size, resource_arenas, allocation_granularity );
	return						result0 >= result1 ? largest_but_one : largest;
}

struct regions_count {
	inline	regions_count	( ) :
		m_region_count	( 0 )
	{
	}

	inline	void	operator( )	( u64 const size, pvoid const address )
	{
		XRAY_UNREFERENCED_PARAMETERS	( size, address );
		++m_region_count;
	}

	u32 m_region_count;
};

struct regions_filler : private boost::noncopyable {
	inline	regions_filler	( xray::memory::platform::regions_type& regions ) :
		m_regions	( regions )
	{
	}

	inline	void	operator( )	( u64 const size, pvoid const address )
	{
		xray::memory::platform::region const value = { size, address, 0 };
		m_regions.push_back	( value );
	}

	xray::memory::platform::regions_type& m_regions;
};

static void allocate_arenas					(
		xray::memory::platform::regions_type& arenas,
		xray::memory::platform::regions_type& resource_arenas,
		u32 const start_address
	)
{
	R_ASSERT					( !arenas.empty() );

	std::sort					( arenas.begin(), arenas.end() );
	std::reverse				( arenas.begin(), arenas.end() );
	while ( !arenas.empty() && !arenas.back().size )
		arenas.pop_back			( );
	std::reverse				( arenas.begin(), arenas.end() );

	R_ASSERT					( !arenas.empty() );
	
	regions_count				counter;
	u64 const min_buffer_size	= arenas.front().size;
	u32 const allocation_granularity = ::allocation_granularity( );
	iterate_regions				( start_address, allocation_granularity, min_buffer_size, counter );

	using xray::memory::platform::regions_type;
	using xray::memory::platform::region;

	regions_type				regions( ALLOCA(counter.m_region_count*sizeof(region)), counter.m_region_count );
	regions_filler				filler(regions);
	iterate_regions				( start_address, allocation_granularity, min_buffer_size, filler );

	std::sort					( regions.begin(), regions.end() );

	// sort resource arenas
	if ( resource_arenas.front().size > resource_arenas.back().size )
		std::swap				( resource_arenas.front(), resource_arenas.back() );

	regions_type::reverse_iterator i		= arenas.rbegin();
	regions_type::reverse_iterator const e	= arenas.rend();
	for ( ; i != e; ++i ) {
		(*i).size				= ( ((*i).size - 1)/allocation_granularity + 1 )*allocation_granularity;

		regions_type::iterator const b	= regions.begin();
		regions_type::iterator const e	= regions.end();
		regions_type::iterator j		= std::lower_bound( b, e, *i );
		R_ASSERT				( j != regions.end() );
		R_ASSERT_CMP			( (*j).size, >=, (*i).size );

		regions_type::iterator k		= j + 1;
		if ( (k != e) && ( (k + 1) == e) )
			j					= select_best_region( j == b ? 0 : (*(j-1)).size, j, k, (*i).size, resource_arenas, allocation_granularity );

		(*i).address			= ::allocate_region( (*i).size, (*j).address, 0 );
		(pbyte&)((*j).address )	+= (*i).size;
		(*j).size				-= (*i).size;

		if ( j != regions.begin() ) {
			if ( (*j).size >= (*(j-1)).size )
				continue;
		}
		else {
			if ( (*j).size >= min_buffer_size )
				continue;

			regions.erase		( j );
			continue;
		}

		region temp				= *j;

		regions.erase			( j );
		if ( temp.size >= min_buffer_size )
			regions.insert(
				std::lower_bound( regions.begin(), regions.end(), temp ),
				temp
			);
	}

	R_ASSERT					( !regions.empty() );
	i							= regions.rbegin();

	R_ASSERT_CMP				( resource_arenas.size(), ==, 2 );
	region& minimum				= resource_arenas.front();
	region& maximum				= resource_arenas.back();
	R_ASSERT					( minimum.size <= maximum.size );
	u64 const resource_arenas_size	= minimum.size + maximum.size;

	if ( (*i).size >= resource_arenas_size ) {
		minimum.address			= allocate_region( minimum.size, (*i).address, 0 );
		maximum.address			= allocate_region( maximum.size, (pbyte)(*i).address + minimum.size, 0 );
		return;
	}

	regions_type::reverse_iterator j = i + 1;
	float const share			= (float)minimum.size / (float)(maximum.size + minimum.size);
	if ( (regions.size() == 1) || ((*i).size*share >= (*j).size) ) {
		minimum.size			= xray::math::align_down( (u64)((*i).size*share), (u64)allocation_granularity );
		maximum.size			= (*i).size - minimum.size;
		minimum.address			= allocate_region( minimum.size, (*i).address, 0 );
		maximum.address			= allocate_region( maximum.size, (pbyte)(*i).address + minimum.size, 0 );
		return;
	}

	if ( ((*i).size >= maximum.size) && ((*j).size >= minimum.size) ) {
		minimum.address			= allocate_region( minimum.size, (*j).address, 0 );
		maximum.address			= allocate_region( maximum.size, (*i).address, 0 );
		return;
	}

	float const proportion		= (float)minimum.size / (float)maximum.size;
	u64 const test_size			= xray::math::align_up( (u64)((*j).size*proportion), (u64)allocation_granularity );
	if ( test_size > (*i).size ) {
		minimum.size			= xray::math::align_up( (u64)((*i).size*proportion), (u64)allocation_granularity );
		minimum.address			= allocate_region( minimum.size, (*j).address, 0 );

		maximum.size			= (*i).size;
		maximum.address			= allocate_region( maximum.size, (*i).address, 0 );
		return;
	}

	minimum.size				= (*j).size;
	minimum.address				= allocate_region( minimum.size, (*j).address, 0 );

	maximum.size				= test_size;
	maximum.address				= allocate_region( maximum.size, (*i).address, 0 );
}

bool xray::memory::platform::try_allocate_arenas_as_a_single_block	(
		regions_type& arenas,
		regions_type& resource_arenas,
		u64 start_address,
		u32 const additional_flags,
		bool assert_on_failure
	)
{
	u64 total_size				= 0;
	{
		regions_type::const_iterator i		= arenas.begin( );
		regions_type::const_iterator e		= arenas.end( );
		for ( ; i != e; ++i )
			total_size			+= (*i).size;

		for (i = resource_arenas.begin(), e = resource_arenas.end(); i != e; ++i )
			total_size			+= (*i).size;
	}

	u64 const pure_total_size	= total_size;
	total_size					= math::align_up( total_size, (u64)allocation_granularity( ) );
	pbyte arena					= (pbyte)::allocate_region( total_size, *(pvoid*)&start_address, additional_flags, false );
	R_ASSERT_U					( !assert_on_failure || arena );
	if ( !arena )
		return					false;

#ifndef MASTER_GOLD
	if ( s_fill_arenas_with_garbage.is_set() )
		memory::fill32			( arena, (size_t)pure_total_size, memory::uninitialized_value<u32>(), (size_t)pure_total_size/sizeof(u32) );
#endif // #ifndef MASTER_GOLD

	s_single_block_arena			= arena;
	s_single_block_arena_size		= pure_total_size;
	{
		regions_type::iterator i = arenas.begin( );
		regions_type::iterator e = arenas.end( );
		for ( ; i != e; ++i ) {
			if ( !(*i).size )
				continue;

			(*i).address		= arena;
			arena				+= (*i).size;
		}

		for (i = resource_arenas.begin(), e = resource_arenas.end(); i != e; ++i ) {
			(*i).address		= arena;
			arena				+= (*i).size;
		}
	}

	return						true;
}

void xray::memory::platform::allocate_arenas	( regions_type& arenas, regions_type& resource_arenas )
{
	u32 const granularity		= allocation_granularity( );
	if ( try_allocate_arenas_as_a_single_block(arenas, resource_arenas, 0, 0, false ) )
		return;

	::allocate_arenas			( arenas, resource_arenas, granularity );
}