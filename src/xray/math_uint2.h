////////////////////////////////////////////////////////////////////////////
//	Created		: 05.03.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MATH_UINT2_H_INCLUDED
#define XRAY_MATH_UINT2_H_INCLUDED

namespace xray {
namespace math {

class uint2 {
public:
	typedef unsigned int	type;

public:
#if !defined(__GCC__)
#	pragma warning(push)
#	pragma warning(disable:4201)
#endif // #if !defined(__GCC__)
	union {
		struct {
			type		x;
			type		y;
		};
		struct {
			type		width;
			type		height;
		};
		type			elements[2];
	};
#if !defined(__GCC__)
#	pragma warning(pop)
#endif // #if !defined(__GCC__)

public:
	inline				uint2			( );
	inline				uint2			( type const x, type const y );

	inline	uint2&		operator +=		( uint2 const& other );
	inline	uint2&		operator +=		( type value );

	inline	uint2&		operator -=		( uint2 const& other );
	inline	uint2&		operator -=		( type value );

	inline	uint2&		operator *=		( type value );
	inline	uint2&		operator /=		( type value );

	inline	type&		operator [ ]	( int index );

	inline	uint2&		set				( type vx, type vy);
}; // class uint2

inline uint2			operator +		( uint2 const& left, uint2 const& right );
inline uint2			operator +		( uint2 const& left, uint2::type const& right );
inline uint2			operator -		( uint2 const& left, uint2 const& right );
inline uint2			operator -		( uint2 const& left, uint2::type const& right );
inline uint2			operator *		( uint2 const& left, uint2::type const& right );
inline uint2			operator *		( uint2::type const& left, uint2 const& right );
inline uint2			operator /		( uint2 const& left, uint2::type const& right );
inline bool				operator ==		( uint2 const& left, uint2 const& right );
inline bool				operator !=		( uint2 const& left, uint2 const& right );


} // namespace math
} // namespace xray

#include <xray/math_uint2_inline.h>

#endif // #ifndef XRAY_MATH_UINT2_H_INCLUDED