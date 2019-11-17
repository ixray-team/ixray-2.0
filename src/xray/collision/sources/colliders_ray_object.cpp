////////////////////////////////////////////////////////////////////////////
//	Created		: 11.02.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "colliders_ray_object.h"
#include "loose_oct_tree.h"

using xray::collision::colliders::ray_object;
using xray::collision::query_type;
using xray::collision::ray_objects_type;
using xray::collision::objects_predicate_type;
using xray::collision::ray_triangles_type;
using xray::collision::triangles_predicate_type;
using xray::math::float3;
using xray::non_null;
using xray::collision::oct_node;

ray_object::ray_object		(
		query_type const query_type,
		xray::collision::loose_oct_tree const& tree,
		float3 const& origin,
		float3 const& direction,
		float const max_distance,
		ray_triangles_type& triangles,
		triangles_predicate_type const& predicate
	) :
	super					( origin, direction, max_distance ),
	m_tree					( tree ),
	m_triangles				( &triangles ),
	m_triangles_predicate	( &predicate ),
	m_objects				( 0 ),
	m_objects_predicate		( 0 ),
	m_query_type			( query_type )
{
	m_result				= query_triangles( );
}

ray_object::ray_object		(
		query_type const query_type,
		xray::collision::loose_oct_tree const& tree,
		float3 const& origin,
		float3 const& direction,
		float const max_distance,
		ray_objects_type& objects,
		objects_predicate_type const& predicate
	) :
	super					( origin, direction, max_distance ),
	m_tree					( tree ),
	m_triangles				( 0 ),
	m_triangles_predicate	( 0 ),
	m_objects				( &objects ),
	m_objects_predicate		( &predicate ),
	m_query_type			( query_type )
{
	m_result				= query_objects( );
}

bool ray_object::intersects_aabb	( float3 const& node_center, float3 const& extents, float& distance ) const
{
	XRAY_ALIGN(16) sse::aabb_a16	aabb;

	(float3&)aabb.min		= node_center - extents;
	aabb.min.padding		= 0.f;

	(float3&)aabb.max		= node_center + extents;
	aabb.max.padding		= 0.f;

	return					( intersects_aabb_sse( aabb, distance ) );
}

inline bool ray_object::intersects_aabb_vertical	( float3 const& node_center, float3 const& radius, float& distance ) const
{
	if (node_center.x + radius.x < m_origin.x)
		return				(false);  			   	
												   
	if (node_center.z + radius.z < m_origin.z)
		return				(false);
	
	if (node_center.x - radius.x > m_origin.x)
		return				(false);  			   	
												   
	if (node_center.z - radius.z > m_origin.z)
		return				(false);

	if ( (m_origin.y >= node_center.y) && (m_direction.y < 0.f) )
		distance			= math::abs( m_origin.y - (node_center.y + radius.y)  );
	else if ( (m_origin.y <= node_center.y) && (m_direction.y > 0.f) )
		distance			= math::abs( node_center.y - (m_origin.y + radius.y)  );
	else
		distance			= 0.f;

	return					(true);
}

namespace xray {
namespace collision {
namespace colliders {
namespace object {

template < bool vertical >
struct vertical_predicate {
	inline bool operator()( ray_object const& query, float3 const& node_center, float3 const& radius, float& distance ) const
	{
		return			query.intersects_aabb( node_center, radius, distance );
	}
}; // struct vertical_predicate

template < >
struct vertical_predicate< true > {
	inline bool operator()( ray_object const& query, float3 const& node_center, float3 const& radius, float& distance ) const
	{
		return			query.intersects_aabb_vertical( node_center, radius, distance );
	}
}; // struct vertical_predicate

struct distance_predicate {
	inline bool	operator( ) ( ray_triangle_result const& left, ray_triangle_result const& right )
	{
		return			( left.distance < right.distance );
	}
	
	inline bool	operator( ) ( ray_object_result const& left, ray_object_result const& right )
	{
		return			( left.distance < right.distance );
	}
}; // distance_predicate

} // namespace object
} // namespace colliders
} // namespace collision
} // namespace xray

template < typename T >
void ray_object::query				( non_null<oct_node const>::ptr const node, float3 const& aabb_center, float const aabb_radius, T const& predicate )
{
	XRAY_ALIGN(16) float	distance;
	if ( !predicate( *this, aabb_center, float3( aabb_radius, aabb_radius, aabb_radius ), distance ) )
		return;

	if ( distance > m_max_distance )
		return;

	float const octant_radius = aabb_radius/2.f;

	oct_node const* const* const b	= node->octants;
	oct_node const* const* i		= b;
	oct_node const* const* const e	= b + 8;
	for ( ; i != e; ++i ) {
		if ( !*i )
			continue;

		query				( *i, aabb_center + octant_vector(i - b)*octant_radius, octant_radius, predicate );
	}

	for ( collision::object* i = node->objects; i; i = i->get_next() ) {
		if ( !i->is_type_suitable(m_query_type) )
			continue;

		float object_distance
#ifdef DEBUG
			= horrible_cast<u32,float>(platform::little_endian() ? 0xfdfdfdcd : 0xcdfdfdfd).second
#endif // #ifdef DEBUG
		;

		if ( !predicate( *this, i->get_aabb_center(), i->get_aabb_extents(), object_distance ) )
			continue;

		if ( !m_objects ) {
			R_ASSERT		( m_triangles_predicate );
			i->ray_query	( m_origin, m_direction, m_max_distance, object_distance, *m_triangles, *m_triangles_predicate );
			continue;
		}

		object_distance = m_max_distance;
		if ( !i->ray_test(m_origin, m_direction, m_max_distance, object_distance) )
			continue;
		
		m_objects->push_back( ray_object_result(&*i, object_distance) );
	}
}

bool ray_object::query_triangles( )
{
	m_triangles->clear			( );

	if ( math::abs( m_direction.y ) == 1.f )
		query				(
			m_tree.root( ),
			m_tree.aabb_center( ),
			m_tree.aabb_radius( ),
			object::vertical_predicate< true >( )
		);
	else
		query				(
			m_tree.root( ),
			m_tree.aabb_center( ),
			m_tree.aabb_radius( ),
			object::vertical_predicate< false >( )
		);

	if ( m_triangles->empty( ) )
		return				( false );

	std::sort				( m_triangles->begin( ), m_triangles->end( ), object::distance_predicate( ) );

	ray_triangles_type::const_iterator	i = m_triangles->begin( );
	ray_triangles_type::const_iterator	e = m_triangles->end( );
	for ( ; i != e; ++i ) {
		if ( (*m_triangles_predicate)( *i ) )
			return			( true );
	}

	return					( false );
}

bool ray_object::query_objects	( )
{
	m_objects->clear			( );

	if ( math::abs( m_direction.y ) == 1.f )
		query				(
			m_tree.root( ),
			m_tree.aabb_center( ),
			m_tree.aabb_radius( ),
			object::vertical_predicate< true >( )
		);
	else
		query				(
			m_tree.root( ),
			m_tree.aabb_center( ),
			m_tree.aabb_radius( ),
			object::vertical_predicate< false >( )
		);

	if ( m_objects->empty( ) )
		return				( false );

	std::sort				( m_objects->begin( ), m_objects->end( ), object::distance_predicate( ) );

	if ( !*m_objects_predicate )
		return				( true );

	ray_objects_type::const_iterator	i = m_objects->begin( );
	ray_objects_type::const_iterator	e = m_objects->end( );
	for ( ; i != e; ++i ) {
		if ( (*m_objects_predicate)( *i ) )
			return			( true );
	}

	return					( false );
}