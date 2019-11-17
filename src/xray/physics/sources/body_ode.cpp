	
////////////////////////////////////////////////////////////////////////////
//	Created 	: 09.02.2008
//	Author		: Konstantin Slipchenko
//	Description : body - physics bone (no collision)
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "body_ode.h"

#include "island.h"

#include "ode_include.h"
#include "ode_convert.h"

body_ode::body_ode			():
m_body( dBodyCreate(0) )
{
	//update_pose( );
}
body_ode::~body_ode			()
{
	dBodyDestroy(m_body);
	m_body = 0;
}
void	body_ode::fill_island				( island	&i )
{
	i.ode().add_body( m_body );
}
bool	body_ode::integrate				( const step_counter_ref& sr, float time_delta )
{
	//update_pose( );
	float4x4 m = get_pose( m );
	float3 a_vel = cast_xr( dBodyGetAngularVel( m_body ) );
	autosleep_update_params dp( 
		
		cast_xr( dBodyGetLinearVel( m_body ) ), 
		a_vel,
		a_vel*time_delta,
		m, 
		sr );
	//return true;
	return m_auto_sleep.update( dp );
}
//void body_ode::update_pose( )
//{
//	ASSERT( m_body );
//	float4x4 m;
//	d_body_matrix( m, m_body );
//	m_pose_anchor.set( m );
//}
float4x4 body_ode::get_pose( )
{
	float4x4 m;
	return get_pose( m );
}
float4x4	&body_ode::get_pose	( float4x4 &pose )
{
	d_body_matrix( pose, m_body );
	return pose;
}

void body_ode::set_position			( const float3 &p )
{
	dBodySetPosition( m_body, p.x, p.y, p.z );
	m_auto_sleep.reset( get_pose(), cast_xr( dBodyGetLinearVel( m_body ) ), cast_xr( dBodyGetAngularVel( m_body ) ) );
}

void body_ode::set_pose( const float4x4 &pose )
{
	dMatrix3 R; dVector3 p;
	d_matrix( R, p, pose );

	dBodySetRotation( m_body, R );
	dBodySetPosition( m_body, p[0], p[1], p[2] );

	m_auto_sleep.reset( get_pose(), cast_xr( dBodyGetLinearVel( m_body ) ), cast_xr( dBodyGetAngularVel( m_body ) ) );
}