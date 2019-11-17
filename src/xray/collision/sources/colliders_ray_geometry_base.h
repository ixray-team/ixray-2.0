////////////////////////////////////////////////////////////////////////////
//	Created		: 05.05.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef COLLIDERS_RAY_GEOMETRY_BASE_H_INCLUDED
#define COLLIDERS_RAY_GEOMETRY_BASE_H_INCLUDED

#include "colliders_ray_base.h"

namespace Opcode {
	class AABBNoLeafNode;
} // namespace Opcode

namespace xray {
namespace collision {

class triangle_mesh_base;

namespace colliders {

namespace geometry {
	template < bool >
	struct vertical_predicate;
} // namespace geometry

class ray_geometry_base : public ray_base {
public:
					ray_geometry_base		(
						triangle_mesh_base const& geometry,
						float3 const& origin,
						float3 const& direction,
						float const max_distance,
						triangles_predicate_type const& predicate
					);
	inline	bool	result					( ) const { return m_result; }

private:
	typedef ray_base super;

protected:
	static	bool	test_triangle			(
						float3 const& v0,
						float3 const& v1,
						float3 const& v2,
						float3 const& position,
						float3 const& direction,
						float const max_distance,
						float& range
					);
			bool	intersects_aabb			( Opcode::AABBNoLeafNode const* node, float& distance ) const;
			bool	intersects_aabb_vertical( Opcode::AABBNoLeafNode const* node, float& distance ) const;
			bool	test_triangle			( u32 const& triangle, float& range );

	friend geometry::vertical_predicate< false >;
	friend geometry::vertical_predicate< true >;

protected:
	triangle_mesh_base const&		m_geometry;
	triangles_predicate_type const&	m_predicate;
	bool							m_result;
}; // class ray_geometry_base

namespace geometry {
	template < bool vertical >
	struct vertical_predicate {
		inline bool operator()( ray_geometry_base const& query, Opcode::AABBNoLeafNode const* node, float& distance ) const
		{
			return query.intersects_aabb( node, distance );
		}
	}; // struct vertical_predicate

	template < >
	struct vertical_predicate< true > {
		inline bool operator()( ray_geometry_base const& query, Opcode::AABBNoLeafNode const* node, float& distance ) const
		{
			return query.intersects_aabb_vertical( node, distance );
		}
	}; // struct vertical_predicate
} // namespace geometry

} // namespace colliders
} // namespace collision
} // namespace xray

#endif // #ifndef COLLIDERS_RAY_GEOMETRY_BASE_H_INCLUDED