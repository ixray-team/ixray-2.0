////////////////////////////////////////////////////////////////////////////
//	Created		: 31.05.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "navigation_params.h"

#include <xray/render/base/world.h>
#include <xray/render/base/debug_renderer.h>


namespace xray {
namespace rtp {

const navigation_params navigation_params::super::min ( -1.f,		-math::pi );
const navigation_params navigation_params::super::max ( 2.2f,	math::pi  );

const navigation_params navigation_params::super::area_min ( -1.5f,		-math::pi - 0.1f   );
const navigation_params navigation_params::super::area_max ( 3.f,	math::pi  + 0.1f   );


navigation_theta_params::navigation_theta_params	( float _theta )
{
	theta() = _theta;
}

const navigation_theta_params navigation_theta_params::super::min (	-math::pi );
const navigation_theta_params navigation_theta_params::super::max (	math::pi  );

const navigation_theta_params navigation_theta_params::super::area_min (	-math::pi - 0.1f   );
const navigation_theta_params navigation_theta_params::super::area_max (	math::pi  + 0.1f   );



navigation_params::navigation_params(float _x,  float _theta )
{
	x() = _x;
	theta() = _theta;
}


void	navigation_params::render( const navigation_params& to ,  xray::render::debug::renderer& r , u32 c  )const
{
	XRAY_UNREFERENCED_PARAMETERS(  &to, &r, c );


	float2 pos0( x(), theta() );

	float2 pos1( to.x(), to.theta() );

	const float3 p0 = render_pos( pos0 );
	const float3 p1 = render_pos( pos1 );

	r.draw_arrow( p0, p1, c );


}


void	navigation_world_base_params::get_local_params( const navigation_world_base_params& action_start_params, float &x, float &theta )const
{
		const navigation_world_base_params& w = *this;

		float4x4 inv_start; inv_start.try_invert( action_start_params.controller_pos );

		float4x4 disp =  w.controller_pos * inv_start;

		const float2 controller_direction ( -w.controller_pos.k.x, -w.controller_pos.k.z );

		const float path_val = w.path.magnitude();

		if( math::is_zero( path_val ) )
		{
			theta = 0;
			x = 0;
			return;
		}
		
		const float2 path_dir = w.path * 1.f/path_val;

		float c = path_dir | controller_direction;
		
		float s = path_dir.x * controller_direction.y - path_dir.y * controller_direction.x;

		float angle = math::atan2( s, c );
		
		//const float2 move = float2( disp.c.x, disp.c.z );


		//if( p.x()<0.f )
		//{
		//	p.x() = - p.x();
		//	angle += math::pi;
		//}

		x		= path_val + disp.c.z; 	//( w.path - move ) | path_dir ;//path_val + disp.c.z;
		theta	= math::angle_normalize_signed( angle );
}


void	navigation_theta_world_params::from_world( const navigation_theta_world_params& action_start_params, navigation_theta_params &p )const
{
	float dummy_x = 0; 
	get_local_params( action_start_params, dummy_x, p.theta() );
}

void	navigation_world_params::from_world( const navigation_world_params& action_start_params, navigation_params &p )const
{
			
	
	get_local_params( action_start_params, p.x(), p.theta() );
}


} // namespace rtp
} // namespace xray
