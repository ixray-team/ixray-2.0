////////////////////////////////////////////////////////////////////////////
//	Created		: 05.05.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "colliders_ray_geometry_base.h"
#include "triangle_mesh_base.h"
#include "opcode_include.h"

using xray::collision::colliders::ray_geometry_base;
using xray::collision::triangle_mesh_base;
using xray::collision::ray_triangles_type;
using xray::collision::triangles_predicate_type;
using xray::collision::object;
using xray::math::float3;
using IceMaths::Point;

ray_geometry_base::ray_geometry_base	(
		triangle_mesh_base const& geometry,
		float3 const& origin,
		float3 const& direction,
		float const max_distance,
		triangles_predicate_type const& predicate
	) :
	super					( origin, direction, max_distance ),
	m_geometry				( geometry ),
	m_predicate				( predicate),
	m_result				( false )
{
}

bool ray_geometry_base::intersects_aabb	( Opcode::AABBNoLeafNode const* node, float& distance ) const
{
	Point const&			node_center = node->mAABB.mCenter;
	float3					center;
	center					= (float3&)node_center;

	Point const&			node_extents = node->mAABB.mExtents;
	float3					extents;
	extents					= (float3&)node_extents;

	XRAY_ALIGN(16) sse::aabb_a16	aabb;

	(float3&)aabb.min		= center - extents;
	aabb.min.padding		= 0.f;

	(float3&)aabb.max		= center + extents;
	aabb.max.padding		= 0.f;

	return					( intersects_aabb_sse( aabb, distance ) );
}

bool ray_geometry_base::intersects_aabb_vertical	( Opcode::AABBNoLeafNode const* node, float& distance ) const
{
	Point const&			node_center = node->mAABB.mCenter;
	Point const&			node_extents = node->mAABB.mExtents;

	if (node_center.x + node_extents.x < m_origin.x)
		return				(false);  			   	
												   
	if (node_center.z + node_extents.z < m_origin.z)
		return				(false);
	
	if (node_center.x - node_extents.x > m_origin.x)
		return				(false);  			   	
												   
	if (node_center.z - node_extents.z > m_origin.z)
		return				(false);

	if ( (m_origin.y >= node_center.y) && (m_direction.y < 0.f) )
		distance			= math::abs( m_origin.y - (node_center.y + node_extents.y)  );
	else if ( (m_origin.y <= node_center.y) && (m_direction.y > 0.f) )
		distance			= math::abs( node_center.y - (m_origin.y + node_extents.y)  );
	else
		distance			= 0.f;

	return					(true);
}

bool ray_geometry_base::test_triangle		(
		float3 const& v0,
		float3 const& v1,
		float3 const& v2,
		float3 const& position,
		float3 const& direction,
		float const max_distance,
		float& range
	)
{
	float3 const edge1		= v1 - v0;
	float3 const edge2		= v2 - v0;
	float3 normal			= direction ^ edge2;
	float const determinant	= edge1 | normal;

	// not culling branch
	if ( math::is_zero( determinant, math::epsilon_5 ) )
		return				( false );

	float3 const			temp = position - v0;
	float const inverted_determinant = 1.f/determinant;
	float const u			= temp.dot_product(normal)*inverted_determinant;
	if ( u < 0.f )
		return				( false );

	if ( u > 1.f )
		return				( false );

	normal					= temp ^ edge1;
	float const v			= direction.dot_product(normal)*inverted_determinant;
	if ( v < 0.f )
		return				( false );

	if ( u + v > 1.f )
		return				( false );

	range					= (edge2 | normal) * inverted_determinant;
	if ( range <= 0.f )
		return				( false );

	if ( range > max_distance )
		return				( false );

	return					( true );
}

bool ray_geometry_base::test_triangle		(u32 const& triangle, float& range)
{
	u32 const* const indices		= m_geometry.indices(triangle);
	float3 const* const vertices	= m_geometry.vertices();
	float3 const& v0				= vertices[indices[0]];
	float3 const& v1				= vertices[indices[1]];
	float3 const& v2				= vertices[indices[2]];
	return							(
		test_triangle(
			v0,
			v1,
			v2,
			m_origin,
			m_direction,
			m_max_distance,
			range
		)
	);
}

