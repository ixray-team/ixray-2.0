////////////////////////////////////////////////////////////////////////////
//	Created		: 18.02.2009
//	Author		: Dmitriy Iassenev
//	Description	: a cuboid is a solid figure bounded by six faces, forming a convex polyhedron
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"

using xray::math::cuboid;
using xray::math::plane;
using xray::math::float4x4;
using xray::math::aabb;
using xray::math::sphere;
using xray::math::intersection;

cuboid::cuboid					( plane const (&planes)[plane_count] )
{
	aabb_plane* i				= &m_planes[0];
	aabb_plane const* const e	= &m_planes[plane_count];
	plane const* j				= &planes[0];
	for ( ; i != e; ++i, ++j ) {
		(*i).plane				= *j;
		(*i).normalize			( );
	}
}

cuboid::cuboid					( aabb const& aabb, float4x4 const& matrix )
{
	m_planes[0].plane			= math::create_plane( aabb.vertex(0), aabb.vertex(1), aabb.vertex(2) );
	m_planes[1].plane			= math::create_plane( aabb.vertex(0), aabb.vertex(2), aabb.vertex(4) );
	m_planes[2].plane			= math::create_plane( aabb.vertex(0), aabb.vertex(1), aabb.vertex(4) );
	m_planes[3].plane			= math::create_plane( aabb.vertex(7), aabb.vertex(5), aabb.vertex(6) );
	m_planes[4].plane			= math::create_plane( aabb.vertex(7), aabb.vertex(3), aabb.vertex(5) );
	m_planes[5].plane			= math::create_plane( aabb.vertex(7), aabb.vertex(3), aabb.vertex(6) );

	aabb_plane* i				= &m_planes[0];
	aabb_plane const* const e	= &m_planes[plane_count];
	for ( ; i != e; ++i ) {
		(*i).plane				= transform( (*i).plane, matrix );
		(*i).normalize			( );
	}
}

cuboid::cuboid					( cuboid const& other, float4x4 const& matrix )
{
	aabb_plane* i				= &m_planes[0];
	aabb_plane const* const e	= &m_planes[plane_count];
	aabb_plane const* j			= &other.m_planes[0];
	for ( ; i != e; ++i, ++j ) {
		(*i).plane				= transform( (*j).plane, matrix );
		(*i).normalize			( );
	}
}

intersection cuboid::test_inexact		( aabb const& aabb) const
{
	u32 inside_count				= 0;

	aabb_plane const* i				= m_planes;
	aabb_plane const* const	e		= m_planes + plane_count;
	for ( ; i != e; ++i) {
		switch ( (*i).test(aabb) ) {
			case intersection_outside	:
				return		(intersection_outside);
			case intersection_intersect :
				continue;
			case intersection_inside	: {
				++inside_count;
				continue;
			}
			default	:		NODEFAULT();
		}
	}

	if ( inside_count < plane_count )
		return						( intersection_intersect );

	ASSERT							( inside_count == plane_count );
	return							( intersection_inside );
}

intersection cuboid::test				(sphere const& sphere) const
{
	const aabb_plane*	i					= m_planes;
	aabb_plane const* const e		= m_planes + plane_count;

	bool outside = false;
	bool inside = true;



	for ( ; i != e; ++i)
	{
		const plane& p = i->plane;

		outside |= p.classify(sphere.center)>sphere.radius+epsilon_5;

		if (outside)
			break;

		inside  &= p.classify(sphere.center)<-sphere.radius-epsilon_5;
	}

	if (outside)
		return intersection_outside;

	if (inside)
		return intersection_inside;

	return intersection_intersect;
}