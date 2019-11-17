////////////////////////////////////////////////////////////////////////////
//	Created 	: 10.02.2008
//	Author		: Konstantin Slipchenko
//	Description : ode_convert
////////////////////////////////////////////////////////////////////////////
#ifndef XRAY_PHYSICS_ODE_CONVERT_H_INCLUDED
#define XRAY_PHYSICS_ODE_CONVERT_H_INCLUDED

#include "ode_include.h"

//convert ode position and rotation to xray matrix return	&out_m
inline float4x4	&d_matrix( float4x4	&out_m, const float *ode_rotation, const float *ode_position )
{
	//ode_rotation : 4x3 is 12 max index 11 3,7,11 unused
	out_m.i.set( ode_rotation[0], ode_rotation[4], ode_rotation[8],  0 );
	out_m.j.set( ode_rotation[1], ode_rotation[5], ode_rotation[9],  0 );
	out_m.k.set( ode_rotation[2], ode_rotation[6], ode_rotation[10], 0 );

	out_m.c.set( ode_position[0], ode_position[1], ode_position[2],  0 );
	
	return out_m;
}


inline void	d_matrix(  float *out_ode_rotation, float *out_ode_position, const float4x4	&in_m )
{
	//ode_rotation : 4x3 is 12 max index 11 3,7,11 unused

	out_ode_rotation[0] = in_m.i[0]; out_ode_rotation[4] = in_m.i[1]; out_ode_rotation[8] =  in_m.i[2];
	out_ode_rotation[1] = in_m.j[0]; out_ode_rotation[5] = in_m.j[1]; out_ode_rotation[9] =  in_m.j[2];
	out_ode_rotation[2] = in_m.k[0]; out_ode_rotation[6] = in_m.k[1]; out_ode_rotation[10] = in_m.k[2];
	
	out_ode_rotation[3] = out_ode_rotation[7] = out_ode_rotation[11] = 0;

	out_ode_position[0] = in_m.c[0]; out_ode_position[1]=in_m.c[1]; out_ode_position[2]=in_m.c[2]; out_ode_position[3] = 0;
}

inline float4x4	&d_body_matrix( float4x4	&out_m, dxBody* b )
{
	return d_matrix( out_m, dBodyGetRotation( b ), dBodyGetPosition( b ) );
}

inline xray::math::float3	&cast_xr( float *v )
{
	return *((xray::math::float3*)v);
}

inline const xray::math::float3	&cast_xr( const float *v )
{
	return *((xray::math::float3*)v);
}

#endif