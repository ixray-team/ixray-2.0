////////////////////////////////////////////////////////////////////////////
//	Created		: 31.05.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef NAVIGATION_PARAMS_H_INCLUDED
#define NAVIGATION_PARAMS_H_INCLUDED
#include "space_params.h"
#include "animation_action_params.h"

namespace xray {
namespace rtp {

struct navigation_params ;
struct navigation_params:
	public space_param< 2, navigation_params >
{
	
	typedef	space_param< 2, navigation_params > super;

			navigation_params	(float x,  float theta );
			navigation_params	( ){};
	void	render				( const navigation_params&,  xray::render::debug::renderer& , u32   )const;
	
	float			&x		()										{ return (*this)[0]; }
	float			&theta	()										{ return (*this)[1]; }
	const	float	x		()								const	{ return (*this)[0]; }
	const	float	theta	()								const	{ return (*this)[1]; }
			bool	similar	( const navigation_params& r )	const	{ return super::similar( r ); }

}; // class navigation_params


struct navigation_theta_params:
	public space_param< 1, navigation_theta_params >
{
	
	typedef	space_param< 1, navigation_theta_params > super;

			navigation_theta_params	( float theta );
			navigation_theta_params	( ){};
	void	render					( const navigation_theta_params&,  xray::render::debug::renderer& , u32   )const{};
	
	float			&theta	()												{ return (*this)[0]; }
	const	float	theta	()										const	{ return (*this)[0]; }
			bool	similar	( const navigation_theta_params& r )	const	{ return super::similar( r ); }

}; // class navigation_params



struct navigation_world_base_params:
	public animation_world_params
{
	navigation_world_base_params	(): animation_world_params(), path( 1, 0 ){}
	navigation_world_base_params	(const float4x4& pos): animation_world_params( pos ), path( 1, 0 ){}
	
	void	get_local_params		( const navigation_world_base_params& action_start_params, float &p, float &angle )const;

	float2 path;
};

struct navigation_world_params:
	public navigation_world_base_params
{
		navigation_world_params(){};
		navigation_world_params	(const float4x4& pos): navigation_world_base_params( pos ){}

		void	from_world		( const navigation_world_params& action_start_params, navigation_params &p )const;
};

struct navigation_theta_world_params:
	public navigation_world_base_params
{
		navigation_theta_world_params	() {}
		navigation_theta_world_params	( const float4x4& pos ): navigation_world_base_params( pos ){}
		
		void	from_world		( const navigation_theta_world_params& action_start_params, navigation_theta_params &p )const;
};



} // namespace rtp
} // namespace xray

#endif // #ifndef NAVIGATION_PARAMS_H_INCLUDED