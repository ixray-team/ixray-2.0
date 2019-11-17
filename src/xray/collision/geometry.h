////////////////////////////////////////////////////////////////////////////
//	Created 	: 11.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_COLLISION_GEOMETRY_H_INCLUDED
#define XRAY_COLLISION_GEOMETRY_H_INCLUDED

#include <xray/collision/common_types.h>

namespace xray {
namespace render {
namespace debug {
	struct renderer;
} // namespace debug
} // namespace render

namespace collision {

struct contact_info;
struct on_contact;
class sphere_geometry;
class box_geometry;
class cylinder_geometry;
class triangle_mesh_base;
class compound_geometry;

struct XRAY_NOVTABLE geometry {
	virtual	void				render				( render::debug::renderer& renderer, math::float4x4 const& matrix ) const = 0;

	virtual	math::float3 const* vertices			( ) const = 0;
	virtual	u32 const*			indices				( u32 triangle_id ) const = 0;
	virtual u32					get_custom_data		( u32 triangle_id ) const { XRAY_UNREFERENCED_PARAMETER(triangle_id); return 0; };

	virtual	bool				aabb_query			( object const* object, math::aabb const& aabb, triangles_type& triangles ) const = 0;
	virtual	bool				cuboid_query		( object const* object, math::cuboid const& cuboid, triangles_type& triangles ) const = 0;
	virtual	bool				ray_query			( object const* object, math::float3 const& origin, math::float3 const& direction, float max_distance, float& distance, ray_triangles_type& triangles, triangles_predicate_type const& predicate ) const = 0;

	virtual	bool				aabb_test			( math::aabb const& aabb ) const = 0;
	virtual	bool				cuboid_test			( math::cuboid const& cuboid ) const = 0;
	virtual	bool				ray_test			( math::float3 const& origin, math::float3 const& direction, float max_distance, float& distance ) const = 0;

	virtual	void				add_triangles		( triangles_type& triangles ) const = 0;

	virtual	math::aabb&			get_aabb			( math::aabb& result ) const = 0;
	virtual	memory::base_allocator&	get_allocator	( ) const = 0;

	virtual	void				generate_contacts	( on_contact& c, const math::float4x4 &self_transform, const math::float4x4 &transform, const collision::geometry& og )		const = 0;
	virtual	void				generate_contacts	( on_contact& c, const math::float4x4 &self_transform, const math::float4x4 &transform, const compound_geometry& og )		const = 0;
	virtual	void				generate_contacts	( on_contact& c, const math::float4x4 &self_transform, const math::float4x4 &transform, const box_geometry& og )			const = 0;
	virtual	void				generate_contacts	( on_contact& c, const math::float4x4 &self_transform, const math::float4x4 &transform, const sphere_geometry& og )			const = 0;
	virtual	void				generate_contacts	( on_contact& c, const math::float4x4 &self_transform, const math::float4x4 &transform, const cylinder_geometry& og )		const = 0;
	virtual	void				generate_contacts	( on_contact& c, const math::float4x4 &self_transform, const math::float4x4 &transform, const triangle_mesh_base& og )		const = 0;

	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( geometry )
}; // class geometry

} // namespace collision
} // namespace xray

#endif // #ifndef XRAY_COLLISION_GEOMETRY_H_INCLUDED