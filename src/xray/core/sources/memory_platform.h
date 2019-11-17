////////////////////////////////////////////////////////////////////////////
//	Created 	: 27.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef MEMORY_PLATFORM_H_INCLUDED
#define MEMORY_PLATFORM_H_INCLUDED

namespace xray {

namespace memory {
namespace platform {

struct region {
	u64		size;
	pvoid	address;
	pvoid	data;
}; // struct region

inline bool operator< ( region const& left, region const& right )
{
	return	left.size < right.size;
}

typedef buffer_vector<region>					regions_type;

void	set_low_fragmentation_heap				( );
void	allocate_arenas							( regions_type& arenas, regions_type& resource_arenas );
bool	try_allocate_arenas_as_a_single_block	(
			regions_type& arenas,
			regions_type& resource_arenas,
			u64 const start_address,
			u32 const additional_flags,
			bool assert_on_failure = true
		);
void	free_region								( pvoid buffer, u64 buffer_size );
void	calculated_desirable_arena_sizes(
			u64 const arena_sizes,
			u64& desirable_managed_arena_size,
			u64& desirable_unmanaged_arena_size
		);
} // namespace platform
} // namespace memory
} // namespace xray

#endif // #ifndef MEMORY_PLATFORM_H_INCLUDED