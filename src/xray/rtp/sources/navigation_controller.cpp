////////////////////////////////////////////////////////////////////////////
//	Created		: 02.06.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "navigation_controller.h"

#include <xray/render/base/world.h>
#include <xray/render/base/debug_renderer.h>
#include <xray/input/world.h>
#include <xray/input/keyboard.h>
#include <xray/input/mouse.h>

namespace xray {
namespace rtp {

navigation_controller::navigation_controller	( xray::animation::i_animation_controller_set *set ):
super( set ),
m_control_path( float2( 0, 0.3f ) ),
m_dbg_cam_height ( 2.f ),
m_dbg_cam_dist( 5.f )
{}


void	navigation_controller::from_world( const world_space_param_type& w, space_param_type &p )const
{
		
		w.from_world( action_start_world_pos(), p );
	
		//XRAY_UNREFERENCED_PARAMETERS(  &w, &p );

		////const float3 angles = w.controller_pos.get_angles_xyz();

		//float4x4 inv_start; inv_start.try_invert( action_start_world_pos().controller_pos );

		//float4x4 disp =  w.controller_pos * inv_start;

		//const float2 controller_direction ( -w.controller_pos.k.x, -w.controller_pos.k.z );

		//const float path_val = w.path.magnitude();

		//if( math::is_zero( path_val ) )
		//{
		//	p.theta() = 0;
		//	p.x() = 0;
		//	return;
		//}
		//
		//const float2 path_dir = w.path * 1.f/path_val;

		//float c = path_dir | controller_direction;
		//
		//float s = path_dir.x * controller_direction.y - path_dir.y * controller_direction.x;

		//float angle = math::atan2( s, c );
		//
		////const float2 move = float2( disp.c.x, disp.c.z );

		//p.x()		= path_val + disp.c.z; 	//( w.path - move ) | path_dir ;//path_val + disp.c.z;
		//
		////if( p.x()<0.f )
		////{
		////	p.x() = - p.x();
		////	angle += math::pi;
		////}
		//
		//p.theta()	= math::angle_normalize_signed( angle );
		

}


bool	navigation_controller::dbg_update_walk( float dt )
{
	super::dbg_update_walk( dt );

	if( !m_debug_wolk_initialized )
		return false;

	return update( m_controller_position, dt );
}

void	navigation_controller::on_new_walk_action( world_space_param_type& w, space_param_type	&params )
{
	w.path = m_control_path;
	
	set_action_start_world_pos( w );

	from_world( w, params );
	
	//params.x() = w.path.magnitude() * 0.3f;
	
}


void	navigation_controller::render( xray::render::debug::renderer& r ) const
{
	super::render( r );
	
	ASSERT ( super::base().global_params().m_set );

	float4x4 m;
	super::base().global_params().m_set->get_bone_world_matrix( m, 0 );

	const float length_factor = 0.5f;

	const float3 pos2 = m.c.xyz();
	const float3 pos4 = m.c.xyz() + float3(m_control_path.x, 0, m_control_path.y ) * length_factor * 3.f;



	r.draw_arrow( pos2, pos4, math::color_xrgb( 255, 0 , 255 ), math::color_xrgb( 0, 255, 0 ) );

	const float3 pos5 = m.c.xyz() - float3( m.k.x, 0, m.k.z ) * length_factor * 0.8f;

	r.draw_arrow( pos2, pos5, math::color_xrgb( 0, 255 , 0 ), math::color_xrgb( 255, 0 , 255 ) );
	
}

extern bool b_camera_control;

void	navigation_controller::dbg_move_control( float4x4 &view_inverted, input::world& input_world )
{

	XRAY_UNREFERENCED_PARAMETERS(  &view_inverted, &input_world  );

	non_null<input::keyboard const>::ptr const keyboard	= input_world.get_keyboard();

	raw<input::mouse const>::ptr const	mouse		= input_world.get_mouse();
	input::mouse::state const&			state		= mouse->get_state( );
	

	const float factor = 0.01f;
		
	if ( state.buttons & input::mouse_button_left )
		m_dbg_cam_dist					-= factor * 3.f;
	
	if ( state.buttons & input::mouse_button_right )
		m_dbg_cam_dist					+= factor * 3.f;
	
	math::clamp ( m_dbg_cam_dist, 0.1f, 35.f );

	float2 move = float2(0,0);


	if ( keyboard->is_key_down( input::key_leftarrow ) )
		move.x				-= factor;
	
	if ( keyboard->is_key_down( input::key_rightarrow ) )
		move.x				+= factor;
	
	if ( keyboard->is_key_down( input::key_uparrow ) )
		move.y				+= factor;
	
	if ( keyboard->is_key_down( input::key_downarrow ) )
		move.y				-= factor;

	if ( keyboard->is_key_down( input::key_w ) )
		m_dbg_cam_height				+= factor;
	
	if ( keyboard->is_key_down( input::key_s ) )
		m_dbg_cam_height				-= factor;

	

	math::clamp ( m_dbg_cam_height, -5.f, 5.f );
	
	if( b_camera_control )
	{
		float4x4 m;
		super::base().global_params().m_set->get_bone_world_matrix( m, 0 );
		view_inverted.c.xyz() = m.c.xyz() + float3(0,1,0) * m_dbg_cam_height - view_inverted.k.xyz() * m_dbg_cam_dist; //view_inverted.i.xyz() * 0.3;
	}
	//float3 path = ( m.i.xyz() * move.x ) - ( m.k.xyz() * move.y )  ;

	float path_mag = m_control_path.magnitude();
	
	math::clamp( path_mag, math::epsilon_6, path_mag );
	
	float ipath_mag = 1.f / path_mag;


	float2 dir = m_control_path * ipath_mag;
	
	float2 cros_dir = float2().cross_product( dir );

	//m_control_path += dir * move.y + cros_dir * move.x;
	m_control_path += cros_dir * move.x;


	//m_control_path += move;// float2( path.x, path.z ); //float2( view_inverted.k.x, view_inverted.k.z ) + move;
}

} // namespace rtp
} // namespace xray