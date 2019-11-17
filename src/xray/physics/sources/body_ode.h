//	Created 	: 09.02.2008
//	Author		: Konstantin Slipchenko
//	Description : ode_body 
////////////////////////////////////////////////////////////////////////////
#ifndef XRAY_PHYSICS_BODY_ODE_H_INCLUDED
#define XRAY_PHYSICS_BODY_ODE_H_INCLUDED
#include "body.h"
#include "auto_sleep.h"
struct dxBody;
class body_ode:
	public	body
{
	dxBody* m_body;
	//pose_anchor					m_pose_anchor;
	//float4x4						m_pose;
	autosleep_full					m_auto_sleep;
public:
				body_ode			();
	virtual 	~body_ode			();
private:
	virtual		void				fill_island				( island	&i );
	virtual		bool				integrate				( const step_counter_ref& sr, float time_delta );
	virtual		void				set_position			( const float3 &p );
	virtual		void				set_pose				( const float4x4 &pose );
	virtual		float4x4			&get_pose				( float4x4 &pose );
	virtual		float4x4			get_pose				( );
	virtual		dxBody				*ode					( ){ return m_body; }
private:
//				void				update_pose				( );

};
#endif