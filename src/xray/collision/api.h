////////////////////////////////////////////////////////////////////////////
//	Created 	: 20.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_COLLISION_API_H_INCLUDED
#define XRAY_COLLISION_API_H_INCLUDED

#ifndef XRAY_COLLISION_API
#	ifdef XRAY_STATIC_LIBRARIES
#		define XRAY_COLLISION_API
#	else // #ifdef XRAY_STATIC_LIBRARIES
#		ifdef XRAY_COLLISION_BUILDING
#			define XRAY_COLLISION_API				XRAY_DLL_EXPORT
#		else // #ifdef XRAY_COLLISION_BUILDING
#			ifndef XRAY_ENGINE_BUILDING
#				define XRAY_COLLISION_API			XRAY_DLL_IMPORT
#			else // #ifndef XRAY_ENGINE_BUILDING
#				define XRAY_COLLISION_API			XRAY_DLL_EXPORT
#			endif // #ifndef XRAY_ENGINE_BUILDING
#		endif // #ifdef XRAY_COLLISION_BUILDING
#	endif // #ifdef XRAY_STATIC_LIBRARIES
#endif // #ifndef XRAY_COLLISION_API

#include <xray/collision/common_types.h>			// for object_type

namespace xray {
namespace collision {

struct space_partitioning_tree;
struct geometry_instance;
struct geometry;
class object;

XRAY_COLLISION_API	non_null<space_partitioning_tree>::ptr create_space_partitioning_tree ( memory::base_allocator* allocator, float min_aabb_radius, u32 reserve_node_count );
XRAY_COLLISION_API	void							destroy							( space_partitioning_tree* tree );

XRAY_COLLISION_API	non_null<geometry>::ptr			create_triangle_mesh_geometry	(
														memory::base_allocator* allocator, 
														math::float3 const* vertices,
														u32 vertex_count,
														u32 const* indices,
														u32 index_count,
														u32 const* triangle_data,
														u32 triangle_count
													);
XRAY_COLLISION_API	non_null<geometry>::ptr			create_triangle_mesh_geometry	(
														memory::base_allocator* allocator, 
														math::float3 const* vertices,
														u32 vertex_count,
														u32 const* indices,
														u32 index_count
													);
XRAY_COLLISION_API	non_null<geometry>::ptr			create_triangle_mesh_geometry	( memory::base_allocator* allocator, resources::managed_resource_ptr resource );
XRAY_COLLISION_API	non_null<geometry>::ptr			create_sphere_geometry			( memory::base_allocator* allocator, float radius );
XRAY_COLLISION_API	non_null<geometry>::ptr			create_box_geometry				( memory::base_allocator* allocator, const xray::math::float3 &half_sides );
XRAY_COLLISION_API	non_null<geometry>::ptr			create_cylinder_geometry		( memory::base_allocator* allocator, float radius, float half_length );
XRAY_COLLISION_API	non_null<geometry>::ptr			create_compound_geometry		( memory::base_allocator* allocator, vectora<collision::geometry_instance*> &instances );
XRAY_COLLISION_API	void							destroy							( geometry* geometry );

XRAY_COLLISION_API	non_null<object>::ptr			create_geometry_object			( memory::base_allocator* allocator, object_type object_type, math::float4x4 const& matrix, non_null<geometry const>::ptr geometry );
XRAY_COLLISION_API	void							destroy							( object* object );

XRAY_COLLISION_API	non_null<geometry_instance>::ptr create_geometry_instance		( memory::base_allocator* allocator, math::float4x4 const& matrix, non_null<geometry const>::ptr geometry );
XRAY_COLLISION_API	void							destroy							( geometry_instance* object );


} // namespace collision
} // namespace xray

#endif // #ifndef XRAY_COLLISION_API_H_INCLUDED