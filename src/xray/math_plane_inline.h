////////////////////////////////////////////////////////////////////////////
//	Created 	: 22.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MATH_PLANE_INLINE_H_INCLUDED
#define XRAY_MATH_PLANE_INLINE_H_INCLUDED

namespace xray {
namespace math {

inline plane::plane									( )
#ifdef DEBUG
:	vector			( float4() )
#endif // #ifdef DEBUG
{
}

inline plane::plane									( float3_pod const& normal, type const distance ) :
	normal			( normal ),
	d				( distance )
{
}

inline bool plane::similar							( plane const& other ) const
{
	return			( vector.similar( other.vector ) );
}

inline float3 plane::project						( float3 const& vertex ) const
{
	return			( vertex - normal*classify( vertex ) );
}

inline plane::type plane::classify					( float3 const& vertex ) const
{
	return			( ( normal | vertex ) + d );
}

inline plane::type plane::distance					( float3 const& vertex ) const
{
	return			( abs( classify( vertex) ) );
}

inline plane& plane::normalize						( )
{
	vector			/= normal.magnitude( );
	return			( * this );
}

inline bool plane::intersect_ray					( float3 const& position, float3 const& direction, float& distance ) const
{
	float const		numerator = classify( position );
	float const		denominator = normal | direction;

	if ( abs( denominator ) < epsilon_7 )  // normal is orthogonal to vector3, cant intersect
		return	( false );

	distance		= -(numerator / denominator);
	return			( ( distance > 0.f ) || is_zero( distance ) );
}

inline bool plane::intersect_ray					( float3 const& position, float3 const& direction, float3& intersection ) const
{
	float			distance;
	if ( !intersect_ray( position, direction, distance ) )
		return	( false );

	intersection	= position + direction*distance;
	return			( true );
}

inline bool plane::intersect_segment				( float3 const& left, float3 const& right, float3& intersection ) const
{
	float3 const	right2left = left - right;
	float const		denominator = normal | right2left;
	if ( abs(denominator) < epsilon_5)
		return		( false );

	float const		distance = -( ( normal | left ) + d) / denominator;
	if ( distance < -epsilon_5 )
		return		( false );

	if ( distance > 1.f + epsilon_5 )
		return		( false );

	intersection	= left + right2left*distance;
	return			( false );
}

inline plane& plane::transform						( float4x4 const& right )
{
	normal			= right.transform_direction( normal );
	d				-= normal | right.c.xyz( );
	return			( *this );
}

inline bool plane::valid							( ) const
{
	return			( vector.valid( ) );
}

inline plane transform								( plane const& left, float4x4 const& right )
{
	return			( plane( left ).transform( right ) );
}

inline plane create_plane_normalized				( float3 const& normalized_normal, float3 const& point_on_plane )
{
	plane			result;
	result.normal	= normalized_normal;
	result.d		= -(normalized_normal | point_on_plane);
	return			result;
}

inline float3 get_plane_normal						( float3 const& first, float3 const& second, float3 const& third )
{
	return			(second - first) ^ (third - first);
}

inline plane create_plane							( float3 const& first, float3 const& second, float3 const& third )
{
	return			create_plane_normalized( normalize( get_plane_normal( first, second, third ) ), first );
}

inline plane create_plane_precise					( float3 const& first, float3 const& second, float3 const& third )
{
	return			create_plane_normalized( normalize_precise( get_plane_normal( first, second, third ) ), first );
}

inline plane create_plane							( float3 const ( &vertices )[ 3 ] )
{
	return			create_plane( vertices[0], vertices[1], vertices[2] );
}

inline plane create_plane_precise					( float3 const ( &vertices )[ 3 ] )
{
	return			create_plane_precise( vertices[0], vertices[1], vertices[2] );
}

} // namespace math
} // namespace xray

#endif // #ifndef XRAY_MATH_PLANE_INLINE_H_INCLUDED