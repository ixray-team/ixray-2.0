////////////////////////////////////////////////////////////////////////////
//	Created 	: 20.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/collision/api.h>
#include "triangle_mesh_buffer.h"
#include "triangle_mesh_resource.h"
#include "sphere_geometry.h"
#include "box_geometry.h"
#include "cylinder_geometry.h"
#include "loose_oct_tree.h"
#include "geometry_object.h"
#include "geometry_instance.h"
#include "sphere_geometry.h"
#include "compound_geometry.h"

using xray::collision::detail::world;
using xray::collision::triangle_mesh_buffer;
using xray::collision::triangle_mesh_resource;
using xray::collision::loose_oct_tree;
using xray::collision::object;
using xray::non_null;
using xray::math::float3;
using xray::math::float4x4;

namespace xray {
namespace collision {

non_null<xray::collision::geometry>::ptr create_triangle_mesh_geometry	(
		memory::base_allocator* allocator,
		float3 const* vertices,
		u32 vertex_count,
		u32 const* indices,
		u32 index_count,
		u32 const* triangle_data,
		u32 triangle_count
	)
{
	return		XRAY_NEW_IMPL( allocator, triangle_mesh_buffer ) ( allocator, vertices, vertex_count, indices, index_count, triangle_data, triangle_count );
}

non_null<xray::collision::geometry>::ptr create_triangle_mesh_geometry	(
	memory::base_allocator* allocator,
	float3 const* vertices,
	u32 vertex_count,
	u32 const* indices,
	u32 index_count
	)
{
	return		create_triangle_mesh_geometry( allocator, vertices, vertex_count, indices, index_count, 0, 0 );
}

non_null<xray::collision::geometry>::ptr create_triangle_mesh_geometry	( memory::base_allocator* allocator, xray::resources::managed_resource_ptr const resource )
{
	return		XRAY_NEW_IMPL( allocator, triangle_mesh_resource ) ( allocator, resource );
}

non_null<xray::collision::geometry>::ptr create_sphere_geometry	( memory::base_allocator* allocator, float radius )
{
	return		XRAY_NEW_IMPL( allocator, sphere_geometry ) ( allocator, radius );
}

non_null<xray::collision::geometry>::ptr create_box_geometry( memory::base_allocator* allocator, const xray::math::float3 &half_sides )
{
	return		XRAY_NEW_IMPL( allocator, box_geometry ) ( allocator, half_sides );
}

non_null<xray::collision::geometry>::ptr create_cylinder_geometry( memory::base_allocator* allocator, float radius, float half_length )
{
	return		XRAY_NEW_IMPL( allocator, cylinder_geometry ) ( allocator, radius, half_length );
}

non_null<geometry>::ptr	create_compound_geometry		( memory::base_allocator* allocator, vectora<collision::geometry_instance*> &instances )
{
	return		XRAY_NEW_IMPL( allocator, compound_geometry ) ( allocator, instances );
}
void destroy		( xray::collision::geometry* geometry )
{
	if (!geometry)
		return;

	XRAY_DELETE_IMPL( &geometry->get_allocator(), geometry );
}

non_null<xray::collision::space_partitioning_tree>::ptr create_space_partitioning_tree( memory::base_allocator* allocator, float min_aabb_radius, u32 reserve_node_count )
{
	return		XRAY_NEW_IMPL( allocator, loose_oct_tree )( allocator, min_aabb_radius, reserve_node_count );
}

void destroy						( xray::collision::space_partitioning_tree* tree )
{
	if (!tree)
		return;

	loose_oct_tree* temp = static_cast_checked<loose_oct_tree*>(tree);
	XRAY_DELETE_IMPL( &temp->get_allocator(), temp );
}

non_null<xray::collision::object>::ptr create_geometry_object	( memory::base_allocator* allocator, object_type const object_type, float4x4 const& matrix, non_null<xray::collision::geometry const>::ptr geometry )
{
	return		XRAY_NEW_IMPL( allocator, detail::geometry_object ) ( allocator, object_type, matrix,  geometry );
}

void destroy				( object* object )
{
	if (!object)
		return;

	detail::geometry_object* temp		= static_cast_checked<detail::geometry_object*>(object);
	memory::base_allocator* allocator	= &temp->get_allocator();
	XRAY_DELETE_IMPL					( allocator, temp );
}

non_null<xray::collision::geometry_instance>::ptr create_geometry_instance	( memory::base_allocator* allocator, float4x4 const& matrix, non_null<xray::collision::geometry const>::ptr geometry )
{
	return		XRAY_NEW_IMPL( allocator, detail::geometry_instance ) ( allocator, matrix,  geometry );
}

void destroy				( xray::collision::geometry_instance* object )
{
	if (!object)
		return;

	detail::geometry_instance* temp = static_cast_checked<detail::geometry_instance*>(object);
	XRAY_DELETE_IMPL( &temp->get_allocator(), temp );
}

} // namespace collision
} // namespace xray