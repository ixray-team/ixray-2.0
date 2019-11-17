////////////////////////////////////////////////////////////////////////////
//	Created		: 03.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef SPACE_PARAMS_H_INCLUDED
#define SPACE_PARAMS_H_INCLUDED

#include <xray/math_randoms_generator.h>

namespace xray {
namespace rtp {

inline double random( math::random32& rnd )
{
	return ( double( rnd.random( u32(-1)) )/double (u32(-1)) );
}

inline float random( float min, float max, math::random32 &rnd )
{
	const float range = max - min;
	return min + float( random( rnd ) * range ) ;
}

inline bool in_range(float v, float min, float max )
{
	return v > min && v < max;
}

template< class space_param_type >
inline bool in_range( const space_param_type &v, const space_param_type &min, const space_param_type &max )
{
	for(u32 i=0; i < space_param_type::dimensions_number; ++i )
		if( !in_range( v[i], min[i], max[i] ) )
			return false;
	return true;
}

template<class space_param_type>
inline bool in_range( const space_param_type &v )
{
	return in_range( v, space_param_type::min, space_param_type::max );
}


template<class space_param_type>
inline bool in_area( const space_param_type &v )
{
	return in_range( v, space_param_type::area_min, space_param_type::area_max );
}

template< u32 dimension, class type >
class space_param
{

public:
	void					set_random		( );

protected:
	bool					similar			( const space_param& l )const;

public:
	void					save			( xray::configs::lua_config_value cfg )const;
	void					load			( const xray::configs::lua_config_value &cfg );


public:
	inline float operator[]		( u32 d ) const;
	inline float& operator []	( u32 d ) ;

public:
	static	const type		min, max;
	static	const type		area_min, area_max;

public:
	static	const u32		dimensions_number  = dimension;

protected:
	float	params[dimensions_number];
};


inline float3 render_pos(const float2& pos )
{
	const float3 gpos	= float3( 0, 10, 0 );
	const float3 p = gpos + float3( 0, pos.x, pos.y );
	return p;
}

//template< class space_param_type > 
//void sort_by_dimension( vector< std::pair< space_param_type, u32 > > &samples, u32 dimension )
//{
//	
//	struct cmp
//	{
//		u32 m_dimension;
//		cmp( u32 dimension ): m_dimension( dimension ){}
//		bool operator () ( const std::pair< space_param_type, u32 >& l, const std::pair< space_param_type, u32 >& r )
//		{
//			return l->first[m_dimension] < r->first[m_dimension];
//		}
//	};
//	std::sort( samples.begin(), samples.end(), cmp(dimension) );
//}


} // namespace rtp
} // namespace xray

#include "space_params_inline.h"

#endif // #ifndef SPACE_PARAMS_H_INCLUDED