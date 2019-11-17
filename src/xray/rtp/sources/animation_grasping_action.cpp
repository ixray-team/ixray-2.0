////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "animation_grasping_action.h"
#include "space_params.h"

#include <xray/animation/i_animation_action.h>
#include <xray/animation/i_animation_controller_set.h>


namespace xray {
namespace rtp {


static const float taget_reward						=  1000000000;
static const float out_of_range_reward				=  0;
static const float taget_action_not_in_taget_reward = -1000000000;
static const float taget_action_exit_reward			= -1000000000;

static const float action_transition_reward			= 10;

u32 animation_grasping_action::super::max_samples = 30;

animation_grasping_action::animation_grasping_action( 
	action_base<animation_grasping_action> &ab,  
	animation::i_animation_action  const *anim ):
	super( ab, anim )
{
	


}

animation_grasping_action::animation_grasping_action( 
	xray::rtp::action_base<animation_grasping_action> &ab
	):
	super( ab )
{
	
}

void	animation_grasping_action::construct( )
{
	dbg_set_taget_params( );
	super::construct( );
}


void	animation_grasping_action::dbg_set_taget_params	( )
{
	is_taget = id() <= 3;
}

void	animation_grasping_action::save( xray::configs::lua_config_value cfg )const
{
	super::save( cfg );
	cfg["is_taget"]			= is_taget;
}

void	animation_grasping_action::load( const xray::configs::lua_config_value &cfg )
{
	super::load	( cfg );
	is_taget	= cfg["is_taget"];
}

float	animation_grasping_action::reward( const animation_grasping_action& a ) const
{
	XRAY_UNREFERENCED_PARAMETER(  a );
	if( is_taget_action() )
		return taget_action_exit_reward;

	return action_transition_reward;
}

bool	animation_grasping_action::is_taget_action		( )const
{
	return is_taget;
}

bool	animation_grasping_action::taget_blends( u32 blend[4], float factors[2], animation_grasping_space_params const &p  )const
{
	
	if( is_taget_action	( ) &&
		m_taget_space_param_devision.taget_blends( blend, factors, p )
		) return true;

	return false;
}

bool	animation_grasping_action::taget( const animation_grasping_space_params& p )const
{

	u32 blends[4] = { u32( -1 ), u32( -1 ), u32( -1 ), u32( -1 ) };
	float factors[2] = { -1, -1 };
	return taget_blends( blends, factors, p );

}

float	animation_grasping_action::reward( const animation_grasping_space_params& a ) const
{
	XRAY_UNREFERENCED_PARAMETER(  a );
	if( taget( a ) )
		return taget_reward;

	if( is_taget_action() )
		return taget_action_not_in_taget_reward;

	if( !in_range( a )  )
		return out_of_range_reward;

	return 0;
}

void	animation_grasping_action::run( animation_grasping_space_params& from, const animation_grasping_action& af, u32 from_sample, u32 blend_sample  )const
{
	
	XRAY_UNREFERENCED_PARAMETERS( &from, &af, &from_sample, &blend_sample );

	transform_to_param( m_cache_transforms[blend_sample], from );

}

void animation_grasping_action::on_walk_run( animation_grasping_world_params& wfrom, const animation_grasping_space_params& lfrom, const float4x4 &disp, buffer_vector< float > &weights  )const
{
	super::on_walk_run( wfrom, lfrom, disp, weights );

/////////////////////////////////////////////////////////////////////////////	
	u32 blends[4] = { u32( -1 ), u32( -1 ), u32( -1 ), u32( -1 ) };
	float factors[2] = { -1, -1 };
	if( taget_blends( blends, factors,  lfrom ) )
	{
		
		weight_mean( weights, blends, factors );
#ifdef	DEBUG

		 animation_grasping_space_params dbgp = get_param_taget( weights );
		 lfrom.similar( dbgp );
#endif
	}
/////////////////////////////////////////////////////////////////////////////

}

float	animation_grasping_action::run( animation_grasping_world_params& wfrom, const animation_grasping_space_params& lfrom, const animation_grasping_action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const
{
	XRAY_UNREFERENCED_PARAMETERS( &wfrom, &afrom, &from_sample, &blend_sample, &time, &step );
	
	return super::run( wfrom, lfrom, afrom, from_sample,  blend_sample, step, time );

}


void	animation_grasping_action::transform_params( animation_grasping_space_params& param, buffer_vector< float > &weights )const
{
	
	float4x4 m;
	m_animation_action->get_displacement_transform( m, weights );
	
	transform_to_param( m, param );

}

animation_grasping_space_params animation_grasping_action::get_param_taget( const buffer_vector< float > &weights )const
{
		

		u32 lh = m_animation_action->bone_index( "LeftHand" );
		u32 rh = m_animation_action->bone_index( "RightHand" );

		float4x4 ml, mr;
		m_animation_action->local_bone_matrix( ml, lh, weights );
		m_animation_action->local_bone_matrix( mr, rh, weights );

		float3 p= 0.5f * ( ml.c.xyz() +  mr.c.xyz() );
		
		return animation_grasping_space_params( float2( p.x, p.z ) );

}

void	animation_grasping_action::cache_blend_transform( u32 blend, buffer_vector< float > &weights )
{
	super::cache_blend_transform( blend, weights );

	if( is_taget )
		m_taget_space_param_devision.add( get_param_taget( weights ), blend );

}

void	animation_grasping_action::cache_transforms	( )
{
	super::cache_transforms( );
	m_taget_space_param_devision.sort();
}


//animation_grasping_world_params		&animation_grasping_global_params::current_position()
//{
//	ASSERT( m_set );
//	return m_current_position;
//}


struct cmp
{
	bool operator () ( const std::pair< float, u32 >& l, const std::pair< float, u32 >& r )
	{
		return l.first < r.first;
	}
};

void	taget_space_param_devision::sort()
{
	std::sort( m_x_sorted_taget.begin(), m_x_sorted_taget.end(), cmp() );
	std::sort( m_y_sorted_taget.begin(), m_y_sorted_taget.end(), cmp() );
}

void	taget_space_param_devision::add( animation_grasping_space_params const &p, u32 blend )
{
	m_x_sorted_taget.push_back( std::pair< float, u32 >( p.taget_position().x, blend ) );
	m_y_sorted_taget.push_back( std::pair< float, u32 >( p.taget_position().y, blend ) );
}




bool	taget_space_param_devision::taget_blends( u32 blend[4], float factors[2], animation_grasping_space_params const &p )const
{
	std::pair< float, u32 > vx( p.taget_position().x, u32(-1) ), vy(  p.taget_position().y, u32(-1) ) ;  
	
	vector< std::pair< float, u32 > >::const_iterator i_x = std::lower_bound(  m_x_sorted_taget.begin(), m_x_sorted_taget.end(), vx , cmp() );
	
	if( i_x == m_x_sorted_taget.end() || i_x == m_x_sorted_taget.begin() )
		return false;

	vector< std::pair< float, u32 > >::const_iterator i_y = std::lower_bound(  m_y_sorted_taget.begin(), m_y_sorted_taget.end(), vy,  cmp() );
	
	if( i_y == m_y_sorted_taget.end() || i_y == m_y_sorted_taget.begin() )
		return false;
	
	blend[ 0 ] = ( i_x - 1 )->second;
	blend[ 1 ] = ( i_x )->second;
	
	factors[0] =  ( vx.first - ( i_x - 1 )->first ) / ( ( i_x )->first - ( i_x - 1 )->first );
	ASSERT( factors[0] >= 0 && factors[0] <=1 );
		
	blend[ 2 ] = ( i_y - 1 )->second;
	blend[ 3 ] = ( i_y )->second;

	factors[1] =  ( vy.first - ( i_y - 1 )->first ) / ( ( i_y )->first - ( i_y - 1 )->first );
	ASSERT( factors[1] >= 0 && factors[1] <=1 );

	return true;
}

} // namespace rtp
} // namespace xray