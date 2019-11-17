////////////////////////////////////////////////////////////////////////////
//	Created		: 06.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "test_grasping_action.h"
#include <xray/render/base/world.h>
#include <xray/render/base/debug_renderer.h>
#include <xray/configs.h>

using xray::rtp::test_grasping_action;
using xray::rtp::action_base;
using xray::float2;
using xray::rtp::grasping_space_params;
using xray::rtp::test_grasping_world_params;



test_grasping_action::test_grasping_action( 
action_base<test_grasping_action> &ab,  
float rotation0,float rotation1,
const float2 &translation0,
const float2 &translation1,
float	 duration_time,
bool  is_grasping,
const grasping_space_params &grasping_position
):abstract_action<grasping_space_params, test_grasping_world_params, test_grasping_action>( ab ),
	b_grasping			( is_grasping ),
	m_grasping_state	( grasping_position ),
	
	m_rotation0			(rotation0),
	m_translation0		(translation0),
	m_rotation1			(rotation1),
	m_translation1		(translation1),
	m_duration_time		(duration_time)
{
}

test_grasping_action::test_grasping_action( action_base<test_grasping_action> &ab )
:abstract_action<grasping_space_params, test_grasping_world_params, test_grasping_action>( ab ),
	b_grasping			( false ),
	m_grasping_state	( float2(math::QNaN,math::QNaN) ),
	
	m_rotation0			(math::QNaN),
	m_translation0		(float2(math::QNaN,math::QNaN)),
	m_rotation1			(math::QNaN),
	m_translation1		(float2(math::QNaN,math::QNaN)),
	m_duration_time		(math::QNaN)
{}


float2	test_grasping_action::mean_translate()const
{
	return  0.5f*( m_translation0 + m_translation1 );
}
float   test_grasping_action::mean_rotate			()const
{
	return  0.5f*( m_rotation0 + m_rotation1 );
}

//static const float transition_reward_scale = 1.f;
float	test_grasping_action::reward( const test_grasping_action& ta  ) const
{
	
	if( b_grasping && !ta.b_grasping )
		return -500;
	
	if( !b_grasping && ta.b_grasping )
		return 0;
	
	float2 my_mean_translate = mean_translate	();
	float2 ta_mean_translate = ta.mean_translate();
	


	//if( abs( my_mean_translate.magnitude() - ta_mean_translate.magnitude() ) > 0.1f  )
	//{
	//	return 10;
	//}
	


	if( abs(ta_mean_translate.x) - abs(my_mean_translate.x) >0.15f )
		return 300;

	if( m_rotation0 > ta.m_rotation1 || m_rotation1 < ta.m_rotation0 )
		return -1000;

	if( -abs(ta_mean_translate.x) + abs(my_mean_translate.x) >0.15f )
		return 50;
	//float my_mean_rotate =( m_rotation0 + m_rotation1 );
	//float ta_mean_rotate =( ta.m_rotation0 + ta.m_rotation1 );

	//

	//float rotation_delta = abs(my_mean_rotate - ta_mean_rotate);
	//float translation_deleta = ( my_mean_translate-ta_mean_translate ).magnitude();

	//if( rotation_delta < math::pi/6.f && 
	//	translation_deleta< 0.2f 
	//	) return 10.f;

	
	return 10.f;
	//return -( ta.m_param_delta.taget_position() - ta.m_param_delta.taget_position() ).magnitude() * transition_reward_scale ;
}

static const float transition_reward_epilon = 0.2f;
//static const float square_transition_reward_epilon = transition_reward_epilon*transition_reward_epilon;
static const float state_reward_value = 10000.f;

float	test_grasping_action::reward( const grasping_space_params& tp  ) const
{
	if( rtaget( tp ) )
		return state_reward_value;
	//const float2 &pos =  tp.taget_position();
	if( b_grasping ) // border( tp ) ||
	//{
		return  -1000;
	//}
	//if(b_grasping)
	//{
	//	float delta = ( m_grasping_state.taget_position() - pos ).square_magnitude();

	//	return 100.f * math::exp( -delta/0.1f ) - 70.f ;//square_transition_reward_epilon
	//}
	return 0;
}

bool	test_grasping_action::border( const grasping_space_params&/* tp*/ )const
{
	return false;
//	const float2 &pos =  tp.taget_position();
//return ( pos.x > grasping_space_params::bmax.x || pos.y > grasping_space_params::bmax.y
//		|| pos.x < grasping_space_params::bmin.x || pos.y < grasping_space_params::bmin.y );
}

bool	test_grasping_action::rtaget( const grasping_space_params& tp )const		
{
	 return b_grasping && ( m_grasping_state.taget_position() - tp.taget_position() ).magnitude() < transition_reward_epilon;
}

bool	test_grasping_action::is_taget_action		( )const 
{
	return b_grasping;
}

bool	test_grasping_action::taget( const grasping_space_params& tp )const		
{
	 return  rtaget( tp ) ;//border( tp ) ||
}

u32 ns = 50;
u32		test_grasping_action::num_blend_samples( )const 
{
	return ns;
}

void	test_grasping_action::sample( u32 blend_sample, float &rotate, float2 &translate ) const
{
	float k = (float(blend_sample)/float(ns));
	rotate		= m_rotation0 * ( 1.f - k ) + m_rotation1 * k;
	translate	= m_translation0 * ( 1.f - k ) + m_translation1 * k; 
}

void	test_grasping_action::run(  grasping_space_params& from, const test_grasping_action& af, u32 from_sample, u32 blend_sample )const
{
	XRAY_UNREFERENCED_PARAMETERS	( &af, &from_sample );

	float rotate; float2 translate;
	sample( blend_sample, rotate, translate );


	from.transform( rotate, translate );
}

float	test_grasping_action::run( test_grasping_world_params& from, const grasping_space_params& , const test_grasping_action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const	
{
	XRAY_UNREFERENCED_PARAMETERS	( &afrom, &from_sample, &step );
	
	test_grasping_world_params to ;//= gl_to.current_position();

	ASSERT(m_duration_time>0);
	float rotate; float2 translate;
	sample( blend_sample, rotate, translate );
	to = from;
	float time_k = 1.f;
	if(m_duration_time > time)
			time_k = time/m_duration_time;

	rotate		*= -time_k;// "-" - apply to controller  ( world_params assume controller (not taget) position )
	translate	*= -time_k;

	to.transform( rotate, translate );
	from = to;
	return m_duration_time - time;
}

//bool	test_grasping_action::completed( float time )const
//{
//	if( time >= m_action_time )
//		return true;
//	return false;
//}

void	test_grasping_action::render( const test_grasping_world_params& a,  xray::render::debug::renderer& renderer ) const
{
	u32 c = 0;
	if( b_grasping )
		c = math::color_xrgb( 255, 0, 0 );
	else if( mean_translate().magnitude() > 0.1f )
			c = math::color_xrgb( 255, 255, 0 );
		else 
			c = math::color_xrgb(0, 0, 255 );
	renderer.draw_sphere( render_pos( a.pos ), 0.02f, c );
}

void	test_grasping_action::render( const grasping_space_params& a,  xray::render::debug::renderer& renderer ) const
{
	grasping_space_params to = a;

	float2 pos0 = a.taget_position();
	run( to, *this, 0, 0 );
	float2 pos1 = to.taget_position();

	//const float3 pos	= float3( 0, 10, 0 );
	//const float3 p0 = pos + float3( 0, pos0.x, pos0.y );
	//const float3 p1 = pos + float3( 0, pos1.x, pos1.y );

	const float3 p0 = render_pos( pos0 );
	const float3 p1 = render_pos( pos1 );
	
	
	float2 translate	=( m_translation0  + m_translation1 ) * 0.5f; 

	//renderer.draw_arrow( p0, p1, math::color_xrgb( 120 + u8( 120.f * abs( m_rotation0 + m_rotation1 )/2.f/math::pi ),  u8( 255.f * abs( translate.x )/0.3f ), 50 );//
	if(b_grasping)
		renderer.draw_sphere( p0, 0.02f,  math::color_xrgb( 255,0,0 ) );//
	else
		renderer.draw_sphere( p0, 0.01f,  math::color_xrgb( 120 + u8( 120.f * abs( m_rotation0 + m_rotation1 )/2.f/math::pi ),  u8( 255.f * abs( translate.x )/0.3f ), 50  ) );//

}


void	test_grasping_action::save( xray::configs::lua_config_value cfg )const
{
	super::save( cfg );
	cfg["grasping"] = b_grasping;
	m_grasping_state.save(cfg["grasping_state"]);
	
	cfg["rotation0"]		= m_rotation0;
	cfg["translation0"]		= m_translation0;
	cfg["rotation1"]		= m_rotation1;
	cfg["translation1"]		= m_translation1;
	cfg["duration_time"]	= m_duration_time;

}
void	test_grasping_action::load( const xray::configs::lua_config_value &cfg )
{
	super::load( cfg );
	b_grasping	= cfg["grasping"];
	m_grasping_state.load(cfg["grasping_state"]);
	
	m_rotation0		= cfg["rotation0"];
	m_translation0	= cfg["translation0"];
	m_rotation1		= cfg["rotation1"];
	m_translation1	= cfg["translation1"];
	m_duration_time	= cfg["duration_time"];
}