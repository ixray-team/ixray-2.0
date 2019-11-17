////////////////////////////////////////////////////////////////////////////
//	Created		: 12.02.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "colliders_cuboid_object.h"
#include "triangle_mesh_base.h"
#include "loose_oct_tree.h"

using xray::collision::colliders::cuboid_object;
using xray::collision::triangle_mesh_base;
using xray::collision::query_type;
using xray::math::float3;
using xray::collision::triangles_type;
using xray::collision::objects_type;
using xray::math::intersection;
using xray::non_null;
using xray::collision::object;
using xray::collision::oct_node;

cuboid_object::cuboid_object				(
		query_type const query_type,
		xray::collision::loose_oct_tree const& tree,
		xray::math::cuboid const& cuboid,
		triangles_type& triangles
	) :
	m_tree					( tree),
	m_query_type			( query_type ),
	m_cuboid				( cuboid ),
	m_objects				( 0 ),
	m_triangles				( &triangles )
{
	process					( );
}

cuboid_object::cuboid_object				(
		query_type const query_type,
		xray::collision::loose_oct_tree const& tree,
		xray::math::cuboid const& cuboid,
		objects_type& objects
	) :
	m_tree					( tree),
	m_query_type			( query_type ),
	m_cuboid				( cuboid ),
	m_objects				( &objects ),
	m_triangles				( 0 )
{
	process					( );
}

void cuboid_object::process					( )
{
	u32 const size_before	= m_objects ? m_objects->size( ) : m_triangles->size( );
	query					( m_tree.root( ), m_tree.aabb_center( ), m_tree.aabb_radius( ) );
	m_result				= ( m_objects ? m_objects->size() : m_triangles->size() ) > size_before;
}

intersection cuboid_object::intersects_aabb	( float3 const& aabb_center, float3 const& aabb_radius ) const
{
	return					( m_cuboid.test_inexact( math::create_center_radius( aabb_center,  aabb_radius ) ) );
}

void cuboid_object::add_objects				( non_null<oct_node const>::ptr const node ) const
{
	oct_node const* const* const b	= node->octants;
	oct_node const* const* i		= b;
	oct_node const* const* const e	= b + 8;
	for ( ; i != e; ++i ) {
		if ( !*i )
			continue;

		add_objects			( *i );
	}

	for ( collision::object const* i = node->objects; i; i = i->get_next() ) {
		if ( !i->is_type_suitable(m_query_type) )
			continue;

		m_objects->push_back( i );
	}
}

void cuboid_object::add_triangles			( non_null<oct_node const>::ptr const node ) const
{
	oct_node const* const* const b	= node->octants;
	oct_node const* const* i		= b;
	oct_node const* const* const e	= b + 8;
	for ( ; i != e; ++i ) {
		if ( !*i )
			continue;

		add_triangles		( *i );
	}

	for ( collision::object const* i = node->objects; i; i = i->get_next() )
		(*i).add_triangles	( *m_triangles );
}

void cuboid_object::query					( non_null<oct_node const>::ptr const node, float3 const& aabb_center, float const aabb_radius ) const
{
	switch ( intersects_aabb( aabb_center, float3(aabb_radius, aabb_radius, aabb_radius) ) ) {
		case math::intersection_outside :
			return;
		case math::intersection_intersect :
			break;
		case math::intersection_inside : {
			if ( m_objects )
				add_objects		( node );
			else
				add_triangles	( node );
			return;
		}
		default : NODEFAULT();
	}

	float const octant_radius = aabb_radius/2.f;

	oct_node const* const* const b	= node->octants;
	oct_node const* const* i		= b;
	oct_node const* const* const e	= b + 8;
	for ( ; i != e; ++i ) {
		if ( !*i )
			continue;

		query					( *i, aabb_center + octant_vector(i - b)*octant_radius, octant_radius );
	}

	for ( collision::object* i = node->objects; i; i = i->get_next() ) {
		if ( !i->is_type_suitable(m_query_type) )
			continue;

		switch ( intersects_aabb( i->get_aabb_center(), i->get_aabb_extents() ) ) {
			case math::intersection_none : continue;
			case math::intersection_outside : continue;
		}

		if ( m_triangles ) {
			i->cuboid_query	( m_cuboid, *m_triangles );
			continue;
		}

		if ( !i->cuboid_test(m_cuboid) )
			continue;
		
		m_objects->push_back( &*i );
	}
}