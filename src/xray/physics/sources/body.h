////////////////////////////////////////////////////////////////////////////
//	Created 	: 09.02.2008
//	Author		: Konstantin Slipchenko
//	Description : body - physics bone (no collision)
////////////////////////////////////////////////////////////////////////////
#ifndef XRAY_PHYSICS_BODY_H_INCLUDED
#define XRAY_PHYSICS_BODY_H_INCLUDED
//#include "pose_anchor.h"


class island_object;
class island;
struct dxBody;
class step_counter_ref;
class body:
	private boost::noncopyable

{

public:



virtual	void				fill_island				( island	&i )									=0;
virtual	bool				integrate				( const step_counter_ref& sr, float time_delta )	=0;
virtual	void				set_position			( const float3 &p )									=0;
virtual	void				set_pose				( const float4x4 &pose )							=0;
virtual	float4x4			&get_pose				( float4x4 &pose )									=0;
virtual	float4x4			get_pose				( )													=0;
virtual	dxBody				*ode					( )													=0;
//inline	const pose_anchor	&pose_anchor			( )	const					{ return m_pose_anchor; }

							body					( )	{}	;
virtual						~body					( )	{}	;



};

#endif
