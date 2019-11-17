////////////////////////////////////////////////////////////////////////////
//	Created 	: 14.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_MATH_FUNCTIONS_H_INCLUDED
#define XRAY_MATH_FUNCTIONS_H_INCLUDED

#include <xray/math_constants.h>
#include <limits>

namespace xray {
namespace math {

template < typename T >
inline typename T::type	dot_product		( T const& left, typename T::type const& right );

inline float			abs				( float left );
inline int				abs				( int left );
inline s64				abs				( s64 left );

template < typename T >
inline T				sqr				( T const& left );

template < typename T >
inline T				min				( T const& left, T const& right );

template < typename T >
inline T				min				( T const& value1, T const& value2, T const& value3 );

template < typename T >
inline T				max				( T const& left, T const& right );

template < typename T >
inline T				max				( T const& value1, T const& value2, T const& value3 );

template < typename T >
inline bool				similar			( T const& left, T const& right, float const epsilon = epsilon_5 );

template < typename T >
inline bool				similar			( T const& left, T const& right, typename T::type const& epsilon );

template < typename T >
inline bool				is_zero			( T const& left, T const& epsilon = epsilon_5 );

inline float			cos				( float angle );
inline float			sin				( float angle );
inline float			tan				( float angle );
inline float			acos			( float value );
inline float			asin			( float value );
inline float			atan			( float value );
inline float			atan2			( float x, float y );
inline float			sqrt			( float value );
inline float			log				( float value );
inline float			exp				( float power );

inline float			pow				( float base, float power );
inline float			pow				( float base, int power );
inline float			pow				( float base, u32 power );
inline u32				pow				( u32 base, u32 power );

inline char				sign			( float value );

inline bool				valid			( float value );

template < typename T >
inline int				floor			( float value );

template < typename T >
inline int				ceil			( float value );

struct sine_cosine {
	float	sine;
	float	cosine;
	inline				sine_cosine		( float const angle );
}; // struct sine_cosine

template < typename T >
inline T				type_epsilon	( );

inline bool				negative		( float value );

inline float			deg2rad			( float value );
inline float			rad2deg			( float value );

template < typename T >
inline T				clamp_r			( T value, T min, T max );

template < typename T >
inline void				clamp			( T& value_and_result, T const min, T const max );

inline float	angle_normalize_always	( float angle );
inline float	angle_normalize			( float angle );
inline float	angle_normalize_signed	( float angle );

enum intersection {
	intersection_none,
	intersection_inside,
	intersection_outside,
	intersection_intersect,
}; // enum intersection

template <class Ordinal>
Ordinal					align_down		( Ordinal value, Ordinal align_on );
template <class Ordinal>
Ordinal					align_up		( Ordinal value, Ordinal align_on );

float   XRAY_CORE_API	table_sin		( int angle_in_degrees );
float   XRAY_CORE_API	table_cos		( int angle_in_degrees );

} // namespace math
} // namespace xray

#include <xray/math_functions_inline.h>

#endif // #ifndef XRAY_MATH_FUNCTIONS_H_INCLUDED