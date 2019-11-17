////////////////////////////////////////////////////////////////////////////
//	Created		: 17.02.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_COLLISION_GEOMETRY_INSTANCE_H_INCLUDED
#define XRAY_COLLISION_GEOMETRY_INSTANCE_H_INCLUDED

#include <xray/collision/common_types.h>

namespace xray {

namespace render {
namespace debug {
	struct renderer;
} // namespace debug
} // namespace render

namespace collision {

class object;
struct geometry;
struct on_contact;

struct geometry_instance {

	virtual	non_null< geometry const >::ptr	get_geometry		( ) const = 0;

	virtual	void							set_matrix			( math::float4x4 const& matrix ) = 0;
	virtual	math::float4x4 const&			get_matrix			( ) const = 0;

	virtual	bool							aabb_query			( object const* object, math::aabb const& aabb, triangles_type& triangles ) const = 0;
	virtual	bool							cuboid_query		( object const* object, math::cuboid const& cuboid, triangles_type& triangles ) const = 0;
	virtual	bool							ray_query			( object const* object, math::float3 const& origin, math::float3 const& direction, float max_distance, float& distance, ray_triangles_type& triangles, triangles_predicate_type const& predicate ) const = 0;

	virtual	bool							aabb_test			( math::aabb const& aabb ) const = 0;
	virtual	bool							cuboid_test			( math::cuboid const& cuboid ) const = 0;
	virtual	bool							ray_test			( math::float3 const& origin, math::float3 const& direction, float max_distance, float& distance ) const = 0;

	virtual	math::aabb const&				get_aabb			( ) const = 0;
	virtual	bool							is_valid			( ) const = 0;
	virtual	void							render				( render::debug::renderer& renderer ) const = 0;

	virtual	void							generate_contacts	( on_contact& c, const xray::collision::geometry_instance& oi )	const = 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( geometry_instance )
}; // struct geometry_instance

} // namespace collision
} // namespace xray

#endif // #ifndef XRAY_COLLISION_GEOMETRY_INSTANCE_H_INCLUDED