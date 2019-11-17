////////////////////////////////////////////////////////////////////////////
//	Created 	: 11.02.2008
//	Author		: Konstantin Slipchenko
//	Description : joint_ball
////////////////////////////////////////////////////////////////////////////
#ifndef XRAY_PHYSICS_JOINT_ODE_H_INCLUDED
#define XRAY_PHYSICS_JOINT_ODE_H_INCLUDED

struct dxJoint;

dxJoint	*create_ode_ball			( );
dxJoint *create_ode_hinge			( );
dxJoint *create_ode_angular_motor	( );
dxJoint	*create_ode_hinge2			( );
dxJoint	*create_ode_slider			( );
dxJoint	*create_ode_fixed			( );
void	destroy_joint				( dxJoint	* &j );


#endif