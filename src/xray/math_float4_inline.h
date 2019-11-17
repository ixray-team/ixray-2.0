////////////////////////////////////////////////////////////////////////////
//	Created 	: 20.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MATH_FLOAT4_INLINE_H_INCLUDED
#define XRAY_MATH_FLOAT4_INLINE_H_INCLUDED

namespace xray {
namespace math {

inline float4& float4_pod::operator -			( )
{
	x			= -x;
	y			= -y;
	z			= -z;
	w			= -w;

	return		( static_cast<float4&>(*this) );
}

inline float4& float4_pod::operator +=			( float4_pod const& other )
{
	x			+= other.x;
	y			+= other.y;
	z			+= other.z;
	w			+= other.w;
	
	return		( static_cast<float4&>(*this) );
}

inline float4& float4_pod::operator +=			( type const value )
{
	x			+= value;
	y			+= value;
	z			+= value;
	w			+= value;

	return		( static_cast<float4&>(*this) );
}

inline float4& float4_pod::operator -=			( float4_pod const& other )
{
	x			-= other.x;
	y			-= other.y;
	z			-= other.z;
	w			-= other.w;

	return		( static_cast<float4&>(*this) );
}

inline float4& float4_pod::operator -=			( type const value )
{
	x			-= value;
	y			-= value;
	z			-= value;
	w			-= value;

	return		( static_cast<float4&>(*this) );
}

inline float4& float4_pod::operator *=			( type const value )
{
	x			*= value;
	y			*= value;
	z			*= value;
	w			*= value;

	return		( static_cast<float4&>(*this) );
}

inline float4& float4_pod::operator /=			( type const value )
{
	ASSERT		( !is_zero( value ) );

	x			/= value;
	y			/= value;
	z			/= value;
	w			/= value;

	return		( static_cast<float4&>(*this) );
}

inline float3& float4_pod::xyz					( )
{
	return		( ( float3& )( *this ) );
}

inline float3 const& float4_pod::xyz			( ) const
{
	return		( ( float3& )( *this ) );
}

inline float4_pod::type& float4_pod::operator [ ]	( int index )
{
	ASSERT		( index < 4 );
	return		( elements[ index ] );
}

inline float4_pod::type const& float4_pod::operator [ ]	( int index )const
{
	ASSERT		( index < 4 );//4 ?
	return		( elements[ index ] );
}

inline float4& float4_pod::set					( float4_pod::type vx, float4_pod::type vy, float4_pod::type vz, float4_pod::type vw )
{
	x			= vx;
	y			= vy;
	z			= vz;
	w			= vw;
	return		( static_cast<float4&>(*this) );
}

inline float4& float4_pod::normalize			( )
{
	return		( *this /= magnitude( ) );
}

inline float4_pod::type float4_pod::normalize_r		( )
{
	type const	magnitude = this->magnitude( );
	*this		/= magnitude;
	return		( magnitude );
}

inline float4& float4_pod::normalize_safe		( float4_pod const& result_in_case_of_zero )
{
	type const	magnitude = this->magnitude( );
	if ( !is_zero( magnitude ) )
		return	( *this	/= magnitude );

	return		( static_cast<float4&>(*this = result_in_case_of_zero) );
}

inline float4::type float4_pod::normalize_safe_r( float4_pod const& result_in_case_of_zero )
{
	type const	magnitude = this->magnitude( );
	if ( !is_zero( magnitude ) ) {
		*this	/= magnitude;
		return	( magnitude );
	}

	*this		= result_in_case_of_zero;
	return		( magnitude );
}

inline float4& float4_pod::abs					( )
{
	x			= math::abs( x );
	y			= math::abs( y );
	z			= math::abs( z );
	w			= math::abs( w );

	return		( static_cast<float4&>(*this) );
}

inline float4& float4_pod::min					( float4_pod const& other )
{
	x			= math::min( x, other.x );
	y			= math::min( y, other.y );
	z			= math::min( z, other.z );
	w			= math::min( w, other.w );

	return		( static_cast<float4&>(*this) );
}

inline float4& float4_pod::max					( float4_pod const& other )
{
	x			= math::max( x, other.x );
	y			= math::max( y, other.y );
	z			= math::max( z, other.z );
	w			= math::max( w, other.w );

	return		( static_cast<float4&>(*this) );
}

inline float4_pod::type float4_pod::magnitude		( ) const
{
	return		( sqrt( square_magnitude( ) ) );
}

inline float4_pod::type float4_pod::square_magnitude( ) const
{
	return		( sqr( x ) + sqr( y ) + sqr( z ) + sqr( w ) );
}

inline bool float4_pod::similar					( float4_pod const& other, float epsilon ) const
{
	if ( !math::similar( x, other.x, epsilon ) )
		return	( false );

	if ( !math::similar( y, other.y, epsilon ) )
		return	( false );

	if ( !math::similar( z, other.z, epsilon ) )
		return	( false );

	if ( !math::similar( w, other.w, epsilon ) )
		return	( false );

	return		( true );
}

inline bool float4_pod::valid					( ) const
{
	return		( math::valid( x ) && math::valid( y ) && math::valid( z ) && math::valid( w ) );
}


inline float dot_product					( const float4_pod &left, const float4_pod &right )
{
	return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w ;
}


inline float4 operator +					( float4_pod const& left, float4_pod const& right )
{
	return		( float4 ( left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w ) );
}

inline float4 operator +					( float4_pod const& left, float4_pod::type const& value )
{
	return		( float4 ( left.x + value, left.y + value, left.z + value, left.w + value ) );
}

inline float4 operator -					( float4_pod const& left, float4_pod const& right )
{
	return		( float4 ( left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w ) );
}

inline float4 operator -					( float4_pod const& left, float4_pod::type const& value )
{
	return		( float4 ( left.x - value, left.y - value, left.z - value, left.w - value ) );
}

inline float4 operator *					( float4_pod const& left, float4_pod::type const& value )
{
	return		( float4 ( left.x * value, left.y * value, left.z * value, left.w * value ) );
}

inline float4 operator *					( float4_pod::type const& value, float4_pod const& right )
{
	return		( float4 ( value * right.x, value * right.y, value * right.z, value * right.w ) );
}

inline float4 operator /					( float4_pod const& left, float4_pod::type const& value )
{
	ASSERT		( !is_zero( value ) );
	return		( float4 ( left.x/value, left.y/value, left.z/value, left.w/value ) );
}

inline bool operator <						( float4_pod const& left, float4_pod const& right )
{
	return		( ( left.x < right.x ) && ( left.y < right.y ) && ( left.z < right.z )&& ( left.w < right.w ) );
}

inline bool operator <=						( float4_pod const& left, float4_pod const& right )
{
	return		( ( left.x <= right.x ) && ( left.y <= right.y ) && ( left.z <= right.z )&& ( left.w <= right.w ) );
}

inline bool operator >						( float4_pod const& left, float4_pod const& right )
{
	return		( ( left.x > right.x ) && ( left.y > right.y ) && ( left.z > right.z )&& ( left.w > right.w ) );
}

inline bool operator >=						( float4_pod const& left, float4_pod const& right )
{
	return		( ( left.x >= right.x ) && ( left.y >= right.y ) && ( left.z >= right.z )&& ( left.w >= right.w ) );
}

inline bool	operator !=						( float4_pod const& left, float4_pod const& right )
{
	return		!(left == right);
}

inline bool	operator ==						( float4_pod const& left, float4_pod const& right )
{
	return		( ( left.x == right.x ) && ( left.y == right.y ) && ( left.z == right.z ) && ( left.w == right.w ) );
}

inline float4 normalize		( float4_pod const& object )
{
	float4_pod::type const magnitude = object.magnitude( );
	ASSERT		( !is_zero( magnitude ) );
	return		( float4( object.x/magnitude, object.y/magnitude, object.z/magnitude, object.w/magnitude ) );
}

inline float4 normalize_safe( float4_pod const& object, float4_pod const& result_in_case_of_zero )
{
	float4_pod::type const magnitude = object.magnitude( );
	if ( !is_zero( magnitude ) )
		return	( float4( object.x/magnitude, object.y/magnitude, object.z/magnitude, object.w/magnitude ) );

	return		( result_in_case_of_zero );
}

inline float4::float4								( ) 
{
#ifdef DEBUG
	x			= SNaN;
	y			= SNaN;
	z			= SNaN;
	w			= SNaN;
#endif // #ifdef DEBUG
}

inline float4::float4								( type const other_x, type const other_y, type const other_z, type const other_w )
{
	x			= other_x;
	y			= other_y;
	z			= other_z;
	w			= other_w;
}

inline float4::float4								( float3 const& xyz, type const other_w )
{
	x			= xyz.x;
	y			= xyz.y;
	z			= xyz.z;
	w			= other_w;
}

inline float4::float4								( float4_pod const& other )
{
	x			= other.x;
	y			= other.y;
	z			= other.z;
	w			= other.w;
}

} // namespace math
} // namespace xray

#endif // #ifndef XRAY_MATH_FLOAT3_INLINE_H_INCLUDED