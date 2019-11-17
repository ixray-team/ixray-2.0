////////////////////////////////////////////////////////////////////////////
//	Created 	: 20.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MATH_FLOAT4_H_INCLUDED
#define XRAY_MATH_FLOAT4_H_INCLUDED

namespace xray {
namespace math {

class float4;
class float3;

class float4_pod {
public:
	typedef float		type;

public:
#if defined(_MSC_VER)
#	pragma warning(push)
#	pragma warning(disable:4201)
#endif // #if defined(_MSC_VER)
	union {
		struct {
			type		x;
			type		y;
			type		z;
			type		w;
		};
		struct {
			type		r;
			type		g;
			type		b;
			type		a;
		};
		float			elements[4];
	};
#if defined(_MSC_VER)
#	pragma warning(pop)
#endif // #if defined(_MSC_VER)

public:
	inline	float4&		operator -		( );

	inline	float4&		operator +=		( float4_pod const& other );
	inline	float4&		operator +=		( type value );

	inline	float4&		operator -=		( float4_pod const& other );
	inline	float4&		operator -=		( type value );

	inline	float4&		operator *=		( type value );
	inline	float4&		operator /=		( type value );

	inline	float3&		xyz				( );
	inline	float3 const& xyz			( ) const;

	inline	type&		operator [ ]	( int index );
	inline	const type&	operator [ ]	( int index )const;
	inline	float4&		set				( type vx, type vy, type vz, type vw );

	inline	float4&		normalize		( );
	inline	type		normalize_r		( ); // returns old magnitude

	inline	float4&		normalize_safe	( float4_pod const& result_in_case_of_zero );
	inline	type		normalize_safe_r( float4_pod const& result_in_case_of_zero ); // returns old magnitude

	inline	float4&		abs				( );

	inline	float4&		min				( float4_pod const& other );
	inline	float4&		max				( float4_pod const& other );

	inline	type		magnitude		( ) const;
	inline	type		square_magnitude( ) const;

	inline	bool		similar			( float4_pod const& other, float epsilon = epsilon_5 ) const;

	inline	bool		valid			( ) const;
}; // struct float4_pod

class float4 : public float4_pod {
public:
	inline				float4			( );
	inline				float4			( type x, type y, type z, type w );
	inline				float4			( float3 const& xyz, type w );
	inline				float4			( float4_pod const& other );
}; // struct float4

inline float4			operator +		( float4_pod const& left, float4_pod const& right );
inline float4			operator +		( float4_pod const& left, float4_pod::type const& right );
inline float4			operator -		( float4_pod const& left, float4_pod const& right );
inline float4			operator -		( float4_pod const& left, float4_pod::type const& right );
inline float4			operator *		( float4_pod const& left, float4_pod::type const& right );
inline float4			operator *		( float4_pod::type const& left, float4_pod const& right );
inline float4			operator /		( float4_pod const& left, float4_pod::type const& right );
inline bool				operator <		( float4_pod const& left, float4_pod const& right );
inline bool				operator <=		( float4_pod const& left, float4_pod const& right );
inline bool				operator >		( float4_pod const& left, float4_pod const& right );
inline bool				operator >=		( float4_pod const& left, float4_pod const& right );
inline bool				operator !=		( float4_pod const& left, float4_pod const& right );
inline bool				operator ==		( float4_pod const& left, float4_pod const& right );
inline float4			normalize		( float4_pod const& object );
inline float4			normalize_safe	( float4_pod const& object, float4_pod const& result_in_case_of_zero = float4( 0.f, 0.f, 0.f, 1.f ) );

} // namespace math
} // namespace xray

#include <xray/math_float4_inline.h>

#endif // #ifndef XRAY_MATH_FLOAT4_H_INCLUDED