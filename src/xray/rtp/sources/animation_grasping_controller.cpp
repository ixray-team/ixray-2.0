////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "animation_grasping_controller.h"

#include <xray/animation/i_animation_controller_set.h>
#include <xray/animation/i_animation_action.h>
#include <xray/render/base/debug_renderer.h>

namespace xray {
namespace rtp {



animation_grasping_controller::animation_grasping_controller ( xray::animation::i_animation_controller_set *set ):
super( set )
{
		dbg_set_random_taget	();
}


void	animation_grasping_controller::dbg_set_random_taget	()
{
	space_param_type rnd;
	rnd.set_random();
	m_controller_position.taget_pos.identity();
	
	float scale_param_area = 0.75f;

	m_controller_position.taget_pos.c.xyz() = m_controller_position.controller_pos.c.xyz();
	m_controller_position.taget_pos.c.xyz() += float3( rnd.taget_position().x * scale_param_area, 0, rnd.taget_position().y * scale_param_area );
}



void		animation_grasping_controller::from_world( const world_space_param_type& w, space_param_type &p )const
{
	XRAY_UNREFERENCED_PARAMETERS( &w, &p );	

	float4x4 iw; R_ASSERT( iw.try_invert( w.controller_pos ) );

	float4x4 t = m_controller_position.taget_pos * iw;

	p.taget_position().x = t.c.x;
	p.taget_position().y = t.c.z;

	//transform_to_param( t, p );

}

void		animation_grasping_controller::render( xray::render::debug::renderer& r ) const
{

	super::render( r );
	r.draw_sphere( m_controller_position.taget_pos.c.xyz(), 0.3f, math::color_xrgb( 255,0,0 ) );
}



bool	animation_grasping_controller::dbg_update_walk			( float dt )
{ 
	
	super::dbg_update_walk( dt );
	
	if( super::value().empty() )
		return false;

	if( update( m_controller_position, dt ) || step() > 20   )
	{
		dbg_set_random_taget( );

		walk_init( start_action_id, 0, m_controller_position );
		
		//m_debug_global.m_set->reset();
		LOG_DEBUG( "animation_grasping_controller taget!" );
		return true;
	
	}
	
	
	return false; 
}


} // namespace rtp
} // namespace xray