////////////////////////////////////////////////////////////////////////////
//	Created		: 06.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef GRASPING_SPACE_PARAMS_H_INCLUDED
#define GRASPING_SPACE_PARAMS_H_INCLUDED

#include "space_params.h"

#include <xray/math_randoms_generator.h>

namespace xray {

namespace configs {
	class lua_config_value;
} // namespace configs

namespace rtp {

struct test_grasping_world_params
{

			test_grasping_world_params	(const float2 &dir_, const float2 &pos_ );
			test_grasping_world_params	();

	void	transform					(  float rotate, const float2 &translate );
	void	save						( xray::configs::lua_config_value cfg )const;
	void	load						( const xray::configs::lua_config_value &cfg );
	


	float2	dir;
	float2	pos;
};

class grasping_space_params;

class grasping_space_params:
	public space_param< 2, grasping_space_params >

{
public:
	typedef	space_param< 2, grasping_space_params > super;


//static	const float2 bmin ;
//static	const float2 bmax ;

public:
	grasping_space_params	( const float2 &v ){ taget_position	( )	 = v; }
	grasping_space_params	( ) { taget_position( ) = float2( math::float_max, math::float_max ); }


	const	float2&		taget_position	( ) const	{ return	*( (const float2*)&params ); }
			float2&		taget_position	( )			{ return	*( ( float2*)&params ); }




public:
	bool	similar		( const grasping_space_params& other ) const;
	void	render		( const grasping_space_params& to,  xray::render::debug::renderer& renderer, u32 color = math::color_xrgb(  255,  255, 255 ) )const;

public:

	void	transform		(  float rotate, const float2 &translate );

public:
	void	save				( xray::configs::lua_config_value cfg )const;
	void	load				( const xray::configs::lua_config_value &cfg );



}; // class grasping_space_params

float3 render_pos(const float2& pos );

} // namespace rtp
} // namespace xray

#endif // #ifndef GRASPING_SPACE_PARAMS_H_INCLUDED