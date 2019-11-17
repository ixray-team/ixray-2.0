////////////////////////////////////////////////////////////////////////////
//	Created		: 31.05.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "navigation_action.h"

namespace xray {
namespace rtp {

u32 navigation_action::super::max_samples = 50;
u32 navigation_theta_action::super::max_samples = 50;


static const float action_transition_reward			= 10;

float	navigation_action::reward( const navigation_action& a ) const
{
	XRAY_UNREFERENCED_PARAMETERS(  &a );
	return action_transition_reward;
}
float	navigation_action::reward( const navigation_params& a ) const
{
	return - a.theta() * a.theta() * 1000 -  a.x() *  1000; //a.x() *
}

float	navigation_action::run( navigation_world_params& wfrom, const navigation_params& lfrom, const navigation_action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const
{
	return super::run( wfrom, lfrom, afrom, from_sample, blend_sample, step, time );
}


void	navigation_action::run( navigation_params& from, const navigation_action& af, u32 from_sample, u32 blend_sample  )const
{
	XRAY_UNREFERENCED_PARAMETERS(  &af, &from_sample, &blend_sample );


	const float3 anles = m_cache_transforms[blend_sample].get_angles_xyz();
	
	const float3 disp  = m_cache_transforms[blend_sample].c.xyz();

	from.x()	+= disp.z;
	

	from.theta() += anles.y;

	
	
	//if( from.x() < 0 )
	//{
	//	from.theta() += math::pi;
	//	from.x() = - from.x();
	//}

	from.theta() = math::angle_normalize_signed( from.theta() );

}


float	navigation_theta_action::reward( const navigation_theta_action& a ) const
{
	XRAY_UNREFERENCED_PARAMETERS(  &a );
	return action_transition_reward;
}
float	navigation_theta_action::reward( const navigation_theta_params& a ) const
{
	return - math::abs( a.theta() )  * 1000; // -  a.x() *  1000; //a.x() *
}

float	navigation_theta_action::run( navigation_theta_world_params& wfrom, const navigation_theta_params& lfrom, const navigation_theta_action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const
{
	return super::run( wfrom, lfrom, afrom, from_sample, blend_sample, step, time );
}


void	navigation_theta_action::run( navigation_theta_params& from, const navigation_theta_action& af, u32 from_sample, u32 blend_sample  )const
{
	XRAY_UNREFERENCED_PARAMETERS(  &af, &from_sample, &blend_sample );


	const float3 anles = m_cache_transforms[blend_sample].get_angles_xyz();
	
	const float3 disp  = m_cache_transforms[blend_sample].c.xyz();

//	from.x()	+= disp.z;
	

	from.theta() += anles.y;

	
	
	//if( from.x() < 0 )
	//{
	//	from.theta() += math::pi;
	//	from.x() = - from.x();
	//}

	from.theta() = math::angle_normalize_signed( from.theta() );

}




} // namespace rtp
} // namespace xray