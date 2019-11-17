////////////////////////////////////////////////////////////////////////////
//	Created		: 06.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "grasping_space_params.h"

#include "rtp_world.h"

#include <xray/render/base/world.h>
#include <xray/render/base/debug_renderer.h>
#include <xray/configs.h>

using xray::rtp::grasping_space_params;
using xray::rtp::test_grasping_world_params;
using xray::float2;
using xray::float3;



const grasping_space_params grasping_space_params::super::min ( float2(-3.f,-3.f) );
const grasping_space_params grasping_space_params::super::max ( float2 (3.f, 3.f) );

const grasping_space_params grasping_space_params::super::area_min ( float2(-4.f,-4.f) );
const grasping_space_params grasping_space_params::super::area_max ( float2 (4.f, 4.f) );





void	grasping_space_params::render( const grasping_space_params& to,  xray::render::debug::renderer& renderer, u32 color/* = math::color_xrgb(  255,  255, 255 ) */ )const
{
	float2 pos0 = taget_position();
	float2 pos1 = to.taget_position();

	

	//const float3 p0 = pos + float3( 0, pos0.x, pos0.y );
	//const float3 p1 = pos + float3( 0, pos1.x, pos1.y );

	const float3 p0 = render_pos( pos0 );
	const float3 p1 = render_pos( pos1 );

	renderer.draw_arrow( p0, p1, color );
}

void	grasping_space_params::transform(  float rotate, const float2 &translate )
{

	float2 tp = taget_position();
	float2 tp_new;
	float cs = cos(  rotate  );
	float sn = sin(  rotate  );

	tp_new.x = tp.x*cs - tp.y*sn;
	tp_new.y = tp.y*cs + tp.x*sn;
	tp_new += translate;//assume post translate
	taget_position() = tp_new;

}

test_grasping_world_params::test_grasping_world_params(const float2 &dir_, const float2 &pos_ ):
	dir(dir_), 
	pos(pos_)
{
}
	test_grasping_world_params::test_grasping_world_params	()
{
	 test_grasping_world_params( float2(1,0), float2(0,0) );
}
void	test_grasping_world_params::transform(  float rotate, const float2 &translate )
	{
		float cs = cos(  rotate  );
		float sn = sin(  rotate  );

		float2 new_dir;

		new_dir.x = dir.x*cs - dir.y*sn;
		new_dir.y = dir.y*cs + dir.x*sn;
		dir = new_dir;
		float2 new_right_dir = float2( -dir.y, dir.x );
		//assume post translate
		pos +=( dir * translate.x + new_right_dir * translate.y );
	}


bool	grasping_space_params::similar(const grasping_space_params& other ) const
{
	return ( taget_position() - other.taget_position() ).magnitude() < math::epsilon_5;
}

void	grasping_space_params::save				( xray::configs::lua_config_value cfg )const
{
	//cfg["position"] = taget_position();
	super::save( cfg );
}
void	grasping_space_params::load				( const xray::configs::lua_config_value &cfg )
{
	//taget_position() = cfg["position"] ;
	super::load( cfg );
}


void	test_grasping_world_params::save( xray::configs::lua_config_value cfg )const
{
	cfg["dir"]	=	dir;
	cfg["pos"]	=	pos;
}
void	test_grasping_world_params::load( const xray::configs::lua_config_value &cfg )
{
	dir = cfg["dir"];
	pos = cfg["pos"];
}