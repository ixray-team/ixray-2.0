////////////////////////////////////////////////////////////////////////////
//	Created 	: 23.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MATH_QUATERNION_INLINE_H_INCLUDED
#define XRAY_MATH_QUATERNION_INLINE_H_INCLUDED

namespace xray {
namespace math {

inline quaternion::quaternion				( )
#ifdef DEBUG
:	vector			( float4() )
#endif // #ifdef DEBUG
{
}

inline quaternion::quaternion				( float4 const& vector ) :
	vector			( vector )
{
}

inline quaternion::quaternion				( float3 angles )
{
	angles			*= .5f;
	float const		sin_x = sin(angles.x); //-
	float const		cos_x = cos(angles.x);
	float const		sin_y = sin(angles.y); //-
	float const		cos_y = cos(angles.y);
	float const		sin_z = sin(angles.z); //-
	float const		cos_z = cos(angles.z);

	// wikipedia & w = - w & angles = -angles
	x =  -sin_x * cos_y * cos_z - cos_x * sin_y * sin_z ;
	y =  -cos_x * sin_y * cos_z + sin_x * cos_y * sin_z ;
	z =  -cos_x * cos_y * sin_z - sin_x * sin_y * cos_z ;
	w =  -cos_x * cos_y * cos_z + sin_x * sin_y * sin_z ;
}

inline quaternion::quaternion				( float3 const& direction, float angle )
{
	w				= cos( angle * .5f );
	vector.xyz()	= direction * sin( angle * .5f );
}

inline quaternion& quaternion::operator +=	( quaternion const& other )
{
	vector			+= other.vector;
	return			( *this );
}

inline quaternion& quaternion::operator -=	( quaternion const& other )
{
	vector			-= other.vector;
	return			( *this );
}

inline quaternion& quaternion::normalize	( )
{
	vector.normalize( );
	return			( *this );
}




inline quaternion& quaternion::conjugate	( )
{
	vector.xyz()	= -vector.xyz();
	return			( *this );
}

inline quaternion& quaternion::identity		( )
{
	vector			= float4( 0.f, 0.f, 0.f, 1.f );
	return			( *this );
}

inline float quaternion::magnitude			( ) const
{
	return			( vector.magnitude( ) );
}

inline bool quaternion::axe_angle			( float3& axe, float& angle ) const
{
	float const s	= vector.xyz().magnitude();
	if ( s > epsilon_7 ) {
		axe			= vector.xyz()*( 1.f / s );
		angle		= 2.f * atan2( s, w );
		return		( true );
	}

	axe				= float3( 0.f, 0.f, 0.f );
	angle			= 0.0f;
	return			( false );
}

inline bool quaternion::similar				( quaternion const& other, float const epsilon ) const
{
	quaternion		test ( other );
	hemi_flip		( test, *this );
	return			( vector.similar( test.vector, epsilon ) );
}

inline bool quaternion::is_unit				( ) const
{
	return			( math::similar( vector.magnitude( ), 1.f ) );
}

inline bool quaternion::valid				( ) const
{
	return			( vector.valid( ) );
}

inline quaternion operator +				( quaternion const& left, quaternion const& right )
{
	quaternion		result;
	result.vector	= left.vector + right.vector;
	return			( result );
}

inline quaternion operator -				( quaternion const& left, quaternion const& right )
{
	quaternion		result;
	result.vector	= left.vector - right.vector;
	return			( result );
}

inline quaternion operator *				( quaternion const& left, quaternion const& right )
{
	quaternion		result;

	result.w		= ( left.w*right.w - left.x*right.x - left.y*right.y - left.z*right.z );
	result.x		= ( left.w*right.x + left.x*right.w + left.y*right.z - left.z*right.y );
	result.y		= ( left.w*right.y - left.x*right.z + left.y*right.w + left.z*right.x );
	result.z		= ( left.w*right.z + left.x*right.y - left.y*right.x + left.z*right.w );

	return			( result );
}

inline	void hemi_flip				( quaternion& q, quaternion const& to )
{
	if( dot_product ( q.vector, to.vector ) < 0.f )
		q.vector = -q.vector; 
	
}


// Wild Magic
inline quaternion logn						( quaternion const& value )
{
    // If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
    // log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
    // sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

    quaternion result;
    result.w = (float)0.0;

   	if ( abs( value.w ) < (float)1.0 )
    {
		float const		n = value.vector.xyz().square_magnitude( );
		float const		r = sqrt( n );
		
		float angle	= atan2( r, value.w );
		float sine	= sin(angle);

        if ( abs( sine ) >= epsilon_6 ) //  Math<Real>::ZERO_TOLERANCE = 1.f e-06
        {
            float coeff = angle/sine;

  			result.vector.xyz() = coeff * value.vector.xyz();

            return result;
        }
    }

 	result.vector.xyz() = value.vector.xyz();
    return result;
}

// Wild Magic
inline quaternion exp					( quaternion const& value )
{
    // If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
    // exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
    // use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

    quaternion result;

	float angle = sqrt( value.vector.xyz().magnitude() );

	sine_cosine	sin_cos( angle );

	result.w = sin_cos.cosine;

	if ( abs(sin_cos.sine) >= epsilon_6 )//Math<Real>::ZERO_TOLERANCE
    {
        float coeff = sin_cos.sine/angle;
		result.vector.xyz() = coeff * value.vector.xyz();
    }
    else
       result.vector.xyz() =  value.vector.xyz();

    return result;
}

} // namespace math
} // namespace xray

#endif // #ifndef XRAY_MATH_QUATERNION_INLINE_H_INCLUDED