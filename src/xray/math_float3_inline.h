////////////////////////////////////////////////////////////////////////////
//	Created 	: 20.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MATH_FLOAT3_INLINE_H_INCLUDED
#define XRAY_MATH_FLOAT3_INLINE_H_INCLUDED

namespace xray {
namespace math {

inline float3 float3_pod::operator -			( ) const
{
	return		( float3( -x, -y, -z ) );
}

inline float3& float3_pod::operator +=			( float3_pod const& other )
{
	x			+= other.x;
	y			+= other.y;
	z			+= other.z;
	
	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::operator +=			( type const value )
{
	x			+= value;
	y			+= value;
	z			+= value;

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::operator -=			( float3_pod const& other )
{
	x			-= other.x;
	y			-= other.y;
	z			-= other.z;

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::operator -=			( type const value )
{
	x			-= value;
	y			-= value;
	z			-= value;

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::operator *=			( type const value )
{
	x			*= value;
	y			*= value;
	z			*= value;

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::operator *=			( float3_pod const& value )
{
	x			*= value.x;
	y			*= value.y;
	z			*= value.z;

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::operator /=			( type const value )
{
	ASSERT		( !is_zero( value ) );

	float const	invert_value = 1.f / value;
	*this		*= invert_value;

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::operator /=			( float3_pod const& value )
{
	ASSERT		( !is_zero( value.x ) );
	ASSERT		( !is_zero( value.y ) );
	ASSERT		( !is_zero( value.z ) );

	x			/= value.x;
	y			/= value.y;
	z			/= value.z;

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::operator ^=			( float3_pod const& other )
{
	return		( cross_product( other ) );
}

inline float3_pod::type& float3_pod::operator [ ]	( int index )
{
	ASSERT		( index < 3 );
	return		( elements[ index ] );
}
inline float3_pod::type const& float3_pod::operator [ ]	( int index )const
{
	ASSERT		( index < 3 );
	return		( elements[ index ] );
}
inline float3& float3_pod::set					( float3_pod::type vx, float3_pod::type vy, float3_pod::type vz )
{
	x			= vx;
	y			= vy;
	z			= vz;
	return		( static_cast<float3&>(*this) );
}

inline float3::type float3_pod::dot_product		( float3_pod const& other ) const
{
	return		( x*other.x + y*other.y + z*other.z );
}

inline float3& float3_pod::cross_product		( float3_pod const& other )
{
	float3_pod	self = *this;

	x			= self.y*other.z - self.z*other.y;
	y			= self.z*other.x - self.x*other.z;
	z			= self.x*other.y - self.y*other.x;

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::normalize			( )
{
	return		( *this /= magnitude( ) );
}

inline float3_pod::type float3_pod::normalize_r		( )
{
	type const	magnitude = this->magnitude( );
	*this		/= magnitude;
	return		( magnitude );
}

inline float3& float3_pod::normalize_safe		( float3_pod const& result_in_case_of_zero )
{
	type const	magnitude = this->magnitude( );
	if ( !is_zero( magnitude ) )
		return	( *this	/= magnitude );

	return		( 	static_cast<float3&>( *this = result_in_case_of_zero) );
}

inline float3_pod::type float3_pod::normalize_safe_r( float3_pod const& result_in_case_of_zero )
{
	type const	magnitude = this->magnitude( );
	if ( !is_zero( magnitude ) ) {
		*this	/= magnitude;
		return	( magnitude );
	}

	*this		= result_in_case_of_zero;
	return		( magnitude );
}

inline float3& float3_pod::abs					( )
{
	x			= math::abs( x );
	y			= math::abs( y );
	z			= math::abs( z );

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::min					( float3_pod const& other )
{
	x			= math::min( x, other.x );
	y			= math::min( y, other.y );
	z			= math::min( z, other.z );

	return		( static_cast<float3&>(*this) );
}

inline float3& float3_pod::max					( float3_pod const& other )
{
	x			= math::max( x, other.x );
	y			= math::max( y, other.y );
	z			= math::max( z, other.z );

	return		( static_cast<float3&>(*this) );
}

inline float3_pod::type float3_pod::magnitude		( ) const
{
	return		( sqrt( square_magnitude( ) ) );
}

inline float3_pod::type float3_pod::square_magnitude( ) const
{
	return		( sqr( x ) + sqr( y )+ sqr( z ) );
}

inline bool float3_pod::similar					( float3_pod const& other, float epsilon ) const
{
	if ( !math::similar( x, other.x, epsilon ) )
		return	( false );

	if ( !math::similar( y, other.y, epsilon ) )
		return	( false );

	if ( !math::similar( z, other.z, epsilon ) )
		return	( false );

	return		( true );
}

inline bool float3_pod::valid					( ) const
{
	return		( math::valid( x ) && math::valid( y ) && math::valid( z ) );
}

inline float3 operator +					( float3_pod const& left, float3_pod const& right )
{
	return		( float3( left.x + right.x, left.y + right.y, left.z + right.z ) );
}

inline float3 operator +					( float3_pod const& left, float3_pod::type const& value )
{
	return		( float3( left.x + value, left.y + value, left.z + value ) );
}

inline float3 operator -					( float3_pod const& left, float3_pod const& right )
{
	return		( float3( left.x - right.x, left.y - right.y, left.z - right.z ) );
}

inline float3 operator -					( float3_pod const& left, float3_pod::type const& value )
{
	return		( float3( left.x - value, left.y - value, left.z - value ) );
}

inline float3 operator *					( float3_pod const& left, float3_pod::type const& value )
{
	return		( float3( left.x * value, left.y * value, left.z * value ) );
}

inline float3 operator *					( float3_pod::type const& value, float3_pod const& right )
{
	return		( float3( value * right.x, value * right.y, value * right.z ) );
}

inline float3 operator *					( float3_pod const& left, float3_pod const& right )
{
	return		( float3( left.x * right.x, left.y * right.y, left.z * right.z ) );
}

inline float3 operator /					( float3_pod const& left, float3_pod::type const& value )
{
	ASSERT		( !is_zero( value ) );
	return		( float3( left.x/value, left.y/value, left.z/value ) );
}

inline float3 operator /					( float3_pod::type const& left, float3_pod const& right )
{
	ASSERT		( !is_zero( right.x ) );
	ASSERT		( !is_zero( right.y ) );
	ASSERT		( !is_zero( right.z ) );

	return		( float3( left / right.x, left / right.y, left / right.z ) );
}

inline float3 operator /					( float3_pod const& left, float3_pod const& right )
{
	ASSERT		( !is_zero( right.x ) );
	ASSERT		( !is_zero( right.y ) );
	ASSERT		( !is_zero( right.z ) );

	return		( float3( left.x / right.x, left.y / right.y, left.z / right.z ) );
}

inline float3::type dot_product				( float3_pod const& left, float3_pod const& right )
{
	return		( left.dot_product( right ) );
}

inline float3::type operator |				( float3_pod const& left, float3_pod const& right )
{
	return		( left.dot_product( right ) );
}

inline bool operator <						( float3_pod const& left, float3_pod const& right )
{
	return		( ( left.x < right.x ) && ( left.y < right.y ) && ( left.z < right.z ) );
}

inline bool operator <=						( float3_pod const& left, float3_pod const& right )
{
	return		( ( left.x <= right.x ) && ( left.y <= right.y ) && ( left.z <= right.z ) );
}

inline bool operator >						( float3_pod const& left, float3_pod const& right )
{
	return		( ( left.x > right.x ) && ( left.y > right.y ) && ( left.z > right.z ) );
}

inline bool operator >=						( float3_pod const& left, float3_pod const& right )
{
	return		( ( left.x >= right.x ) && ( left.y >= right.y ) && ( left.z >= right.z ) );
}

inline bool	operator !=						( float3_pod const& left, float3_pod const& right )
{
	return		!(left == right);
}

inline bool	operator ==						( float3_pod const& left, float3_pod const& right )
{
	return		( ( left.x == right.x ) && ( left.y == right.y ) && ( left.z == right.z ) );
}

inline float3 cross_product					( float3_pod const& left, float3_pod const& right )
{
	float3		result;
	result.x	= left.y*right.z - left.z*right.y;
	result.y	= left.z*right.x - left.x*right.z;
	result.z	= left.x*right.y - left.y*right.x;
	return		( result );
}

inline float3 operator ^					( float3_pod const& left, float3_pod const& right )
{
	return		( cross_product( left, right ) );
}

inline float3 normalize		( float3_pod const& object )
{
	float3_pod::type const magnitude = object.magnitude( );
	ASSERT		( !is_zero( magnitude ) );
	return		( float3( object.x/magnitude, object.y/magnitude, object.z/magnitude ) );
}

inline float3 normalize_safe( float3_pod const& object, float3_pod const& result_in_case_of_zero )
{
	float3_pod::type const magnitude = object.magnitude( );
	if ( !is_zero( magnitude ) )
		return	( float3( object.x/magnitude, object.y/magnitude, object.z/magnitude ) );

	return		( result_in_case_of_zero );
}

inline float3 min		( float3_pod const& left, float3_pod const& right )
{
	float3		result;
	result.x	= min( left.x, right.x );
	result.y	= min( left.y, right.y );
	result.z	= min( left.z, right.z );
	return		( result );
}

inline float3 max		( float3_pod const& left, float3_pod const& right )
{
	float3		result;
	result.x	= max( left.x, right.x );
	result.y	= max( left.y, right.y );
	result.z	= max( left.z, right.z );
	return		( result );
}

inline float3 abs		( float3_pod const& object )
{
	return			( float3( abs(object.x), abs(object.y), abs(object.z) ) );
}

inline float magnitude	( float3_pod const& object )
{
	return			( object.magnitude( ) );
}

inline float square_magnitude	( float3_pod const& object )
{
	return			( object.square_magnitude( ) );
}

inline bool similar		( float3_pod const& left, float3_pod const& right, float epsilon )
{
	return			( left.similar( right, epsilon ) );
}

inline bool valid		( float3_pod const& object )
{
	return			( object.valid( ) );
}


inline float3::float3						( ) 
{
#ifdef DEBUG
	x			= SNaN;
	y			= SNaN;
	z			= SNaN;
#endif // #ifdef DEBUG
}

inline float3::float3						( type const other_x, type const other_y, type const other_z )
{
	x			= other_x;
	y			= other_y;
	z			= other_z;
}

inline float3::float3						( float3_pod const& other ) 
{
	x			= other.x;
	y			= other.y;
	z			= other.z;
}

inline float3 min		( float3 const& left, float3 const& right )
{
	float3		result;
	result.x	= min( left.x, right.x );
	result.y	= min( left.y, right.y );
	result.z	= min( left.z, right.z );
	return		( result );
}

inline float3 max		( float3 const& left, float3 const& right )
{
	float3		result;
	result.x	= max( left.x, right.x );
	result.y	= max( left.y, right.y );
	result.z	= max( left.z, right.z );
	return		( result );
}

inline float3 abs		( float3 const& object )
{
	return			( float3( abs(object.x), abs(object.y), abs(object.z) ) );
}

inline float magnitude	( float3 const& object )
{
	return			( object.magnitude( ) );
}

inline float square_magnitude	( float3 const& object )
{
	return			( object.square_magnitude( ) );
}

inline bool similar		( float3 const& left, float3 const& right, float epsilon )
{
	return			( left.similar( right, epsilon ) );
}

inline bool valid		( float3 const& object )
{
	return			( object.valid( ) );
}

} // namespace math
} // namespace xray

#endif // #ifndef XRAY_MATH_FLOAT3_INLINE_H_INCLUDED