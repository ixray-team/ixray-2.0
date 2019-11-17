////////////////////////////////////////////////////////////////////////////
//	Created 	: 20.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"

using xray::math::float4x4;
using xray::math::float3;

static float4x4 invert_impl								( float4x4 const& other, float const determinant )
{
//	ASSERT		( xray::math::similar( determinant, other.determinant( ), xray::math::epsilon_5 ) );

	float const	inverted_determinant = 1.f / determinant;

	float4x4	result;

	result.e00 	=  inverted_determinant * ( other.e11*other.e22 - other.e12*other.e21 );
	result.e01 	= -inverted_determinant * ( other.e01*other.e22 - other.e02*other.e21 );
	result.e02 	=  inverted_determinant * ( other.e01*other.e12 - other.e02*other.e11 );
	result.e03 	=  0.f;
	
	result.e10 	= -inverted_determinant * ( other.e10*other.e22 - other.e12*other.e20 );
	result.e11 	=  inverted_determinant * ( other.e00*other.e22 - other.e02*other.e20 );
	result.e12 	= -inverted_determinant * ( other.e00*other.e12 - other.e02*other.e10 );
	result.e13 	=  0.f;
	
	result.e20 	=  inverted_determinant * ( other.e10*other.e21 - other.e11*other.e20 );
	result.e21 	= -inverted_determinant * ( other.e00*other.e21 - other.e01*other.e20 );
	result.e22 	=  inverted_determinant * ( other.e00*other.e11 - other.e01*other.e10 );
	result.e23 	=  0.f;
	
	result.e30 	= -( other.e30*result.e00 + other.e31*result.e10 + other.e32*result.e20 );
	result.e31 	= -( other.e30*result.e01 + other.e31*result.e11 + other.e32*result.e21 );
	result.e32 	= -( other.e30*result.e02 + other.e31*result.e12 + other.e32*result.e22 );
	result.e33 	=  1.f;

	return		( result );
}

float4x4 xray::math::invert4x3							( float4x4 const& other )
{
	float const determinant = other.determinant();
	ASSERT		( !is_zero( determinant, epsilon_7) );
	return		( invert_impl ( other, determinant ) );
}

bool float4x4::try_invert								( float4x4 const& other )
{
	float const determinant = other.determinant();
	if ( is_zero( determinant, epsilon_7) )
		return	( false );

	*this		= invert_impl ( other, determinant );
	return		( true );
}

float3 float4x4::get_angles_xyz_INCORRECT				( ) const // zxy indeed
{
	ASSERT( !math::is_zero( i.xyz().magnitude( ) ), "vector 'i' is 0 !" );
	ASSERT( !math::is_zero( j.xyz().magnitude( ) ), "vector 'j' is 0 !" );
	ASSERT( !math::is_zero( k.xyz().magnitude( ) ), "vector 'k' is 0 !" );

	float	inv_scale_x	= 1/sqrt( i.xyz().square_magnitude( ) );
	float	inv_scale_y	= 1/sqrt( j.xyz().square_magnitude( ) );	
	float	inv_scale_z	= 1/sqrt( k.xyz().square_magnitude( ) );

	float const	horde = sqrt( sqr( j.y*inv_scale_y ) + sqr( i.y*inv_scale_x ) );//z->x ->y
    if ( horde > 16.f*type_epsilon<float>() )
		return	(
			float3(
				atan2( -k.y*inv_scale_z, horde ),
				atan2( k.x*inv_scale_z, k.z*inv_scale_z ),//y 3 
				atan2( i.y*inv_scale_x, j.y*inv_scale_y ) //z 1
			)
		);

	return		(
		float3(
			atan2( -k.y*inv_scale_z, horde ),
			atan2( -i.z*inv_scale_x, i.x*inv_scale_x ),
			0.f
		)
	);
}



float3 float4x4::get_angles_xyz				( ) const
{
	ASSERT( !math::is_zero( i.xyz().magnitude( ) ), "vector 'i' is 0 !" );
	ASSERT( !math::is_zero( j.xyz().magnitude( ) ), "vector 'j' is 0 !" );
	ASSERT( !math::is_zero( k.xyz().magnitude( ) ), "vector 'k' is 0 !" );

	float	inv_scale_x	= 1/sqrt( i.xyz().square_magnitude( ) );
	float	inv_scale_y	= 1/sqrt( j.xyz().square_magnitude( ) );	
	float	inv_scale_z	= 1/sqrt( k.xyz().square_magnitude( ) );

	float const	horde = sqrt( sqr( j.z*inv_scale_y ) + sqr( k.z*inv_scale_x ) );     //x->y ->z


		//if ( horde > epsilon_7   )  
	return	(
			float3(
			
				atan2( -j.z*inv_scale_y, k.z*inv_scale_z ), //x 1
				atan2( i.z*inv_scale_x, horde ),			//y 2
				atan2( -i.y, i.x   )						//z 3 //*inv_scale_y  
				
			)
		);

	//float const	horde1 =  sqrt( sqr( k.y*inv_scale_z ) + sqr( j.y*inv_scale_y ) );
	//return		( 
	//	float3(
	//		atan2(  k.y*inv_scale_z ,j.y*inv_scale_y ),

	//		atan2( horde1, i.y*inv_scale_x  ),

	//		//atan2( i.z*inv_scale_x, i.x*inv_scale_x ),
	//		atan2( j.x, j.y )
	//	)
	//);
}





float4x4 xray::math::mul4x3						( float4x4 const& left, float4x4 const& right )
{
	float4x4	result;

	result.e00 	= left.e00*right.e00 + left.e01*right.e10 + left.e02*right.e20;
	result.e01 	= left.e00*right.e01 + left.e01*right.e11 + left.e02*right.e21;
	result.e02 	= left.e00*right.e02 + left.e01*right.e12 + left.e02*right.e22;
	result.e03 	= left.e00*right.e03 + left.e01*right.e13 + left.e02*right.e23;

	result.e10 	= left.e10*right.e00 + left.e11*right.e10 + left.e12*right.e20;
	result.e11 	= left.e10*right.e01 + left.e11*right.e11 + left.e12*right.e21;
	result.e12 	= left.e10*right.e02 + left.e11*right.e12 + left.e12*right.e22;
	result.e13	= left.e10*right.e03 + left.e11*right.e13 + left.e12*right.e23;

	result.e20 	= left.e20*right.e00 + left.e21*right.e10 + left.e22*right.e20;
	result.e21 	= left.e20*right.e01 + left.e21*right.e11 + left.e22*right.e21;
	result.e22 	= left.e20*right.e02 + left.e21*right.e12 + left.e22*right.e22;
	result.e23 	= left.e30*right.e03 + left.e21*right.e13 + left.e22*right.e23;

	result.e30 	= left.e30*right.e00 + left.e31*right.e10 + left.e32*right.e20 + right.e30;
	result.e31 	= left.e30*right.e01 + left.e31*right.e11 + left.e32*right.e21 + right.e31;
	result.e32 	= left.e30*right.e02 + left.e31*right.e12 + left.e32*right.e22 + right.e32;
	result.e33 	= left.e30*right.e03 + left.e31*right.e13 + left.e32*right.e23 + right.e33;

	return		( result );
}

float4x4 xray::math::mul4x4						( float4x4 const& left, float4x4 const& right )
{
	float4x4	result;

	result.e00 	= left.e00*right.e00 + left.e01*right.e10 + left.e02*right.e20 + left.e03*right.e30;
	result.e01 	= left.e00*right.e01 + left.e01*right.e11 + left.e02*right.e21 + left.e03*right.e31;
	result.e02 	= left.e00*right.e02 + left.e01*right.e12 + left.e02*right.e22 + left.e03*right.e32;
	result.e03 	= left.e00*right.e03 + left.e01*right.e13 + left.e02*right.e23 + left.e03*right.e33;
																							 
	result.e10 	= left.e10*right.e00 + left.e11*right.e10 + left.e12*right.e20 + left.e13*right.e30;
	result.e11 	= left.e10*right.e01 + left.e11*right.e11 + left.e12*right.e21 + left.e13*right.e31;
	result.e12 	= left.e10*right.e02 + left.e11*right.e12 + left.e12*right.e22 + left.e13*right.e32;
	result.e13	= left.e10*right.e03 + left.e11*right.e13 + left.e12*right.e23 + left.e13*right.e33;
																							 
	result.e20 	= left.e20*right.e00 + left.e21*right.e10 + left.e22*right.e20 + left.e23*right.e30;
	result.e21 	= left.e20*right.e01 + left.e21*right.e11 + left.e22*right.e21 + left.e23*right.e31;
	result.e22 	= left.e20*right.e02 + left.e21*right.e12 + left.e22*right.e22 + left.e23*right.e32;
	result.e23 	= left.e30*right.e03 + left.e21*right.e13 + left.e22*right.e23 + left.e23*right.e33;
																							 
	result.e30 	= left.e30*right.e00 + left.e31*right.e10 + left.e32*right.e20 + left.e33*right.e30;
	result.e31 	= left.e30*right.e01 + left.e31*right.e11 + left.e32*right.e21 + left.e33*right.e31;
	result.e32 	= left.e30*right.e02 + left.e31*right.e12 + left.e32*right.e22 + left.e33*right.e32;
	result.e33 	= left.e30*right.e03 + left.e31*right.e13 + left.e32*right.e23 + left.e33*right.e33;

	return		( result );
}

float4x4 xray::math::create_projection			( float4x4::type field_of_view, float4x4::type aspect_ratio, float4x4::type near_plane, float4x4::type far_plane )
{
	return		( create_projection_hat	( tan( field_of_view*.5f ), aspect_ratio, near_plane, far_plane ) );
}

float4x4 xray::math::create_projection_hat		( float4x4::type hat, float4x4::type aspect_ratio, float4x4::type near_plane, float4x4::type far_plane )
{
	ASSERT		( abs( far_plane - near_plane ) > epsilon_5 );
	ASSERT		( abs( hat ) > epsilon_5 );
	
	float const	cotan = 1.f/hat;
	float const	q = far_plane/( far_plane - near_plane );
	
	float4x4	result;

	result.i	= float4 ( aspect_ratio*cotan,	0.f,	0.f,			0.f );
	result.j	= float4 ( 0.f,					cotan,	0.f,			0.f );
	result.k	= float4 ( 0.f,					0.f,	q,				1.f );
	result.c	= float4 ( 0.f,					0.f,	-q*near_plane,	0.f );

	return		( result ); 
}

float4x4 xray::math::create_projection_orthogonal	( float4x4::type w, float4x4::type h, float4x4::type near_plane, float4x4::type far_plane )
{
	float const	distance = far_plane - near_plane;

	float4x4	result;

	result.i	= float4( 2.f/w,	0.f,	0.f,					0.f );
	result.j	= float4( 0.f,		2.f/h,	0.f,					0.f );
	result.k	= float4( 0.f,		0.f,	1.f/distance,			0.f );
	result.c	= float4( 0.f,		0.f,	-near_plane/distance,	1.f );

	return		( result );
}

float4x4 xray::math::create_camera_direction		( float3 const& from, float3 const& view, float3 const& world_up )
{
	float3 const up = ( view*-(world_up | view ) + world_up ).normalize( );
	float3 const right = up ^ view;

	float4x4	result;

	result.i	= float4(	right.x,			up.x,			view.x,			0.f );
	result.j	= float4(	right.y,			up.y,			view.y,			0.f );
	result.k	= float4(	right.z,			up.z,			view.z,			0.f );
	result.c	= float4(	-(from | right),	-(from | up),	-(from | view), 1.f );

	return		( result );
}

float4x4 xray::math::create_camera_at				( float3 const& from, float3 const& at, float3 const& up )
{
	return		( create_camera_direction ( from, ( at - from ).normalize( ), up ) );
}

float4x4 xray::math::create_rotation				( float3 const& axis, float const angle )
{
	float4x4	result;

	sine_cosine	temp( angle );
	float const inv_cosine = 1.f - temp.cosine;
	float const sqr_x = sqr( axis.x );
	float const sqr_y = sqr( axis.y );
	float const sqr_z = sqr( axis.z );
	float const xy_inv_cosine = axis.x*axis.y*inv_cosine;
	float const yz_inv_cosine = axis.y*axis.z*inv_cosine;
	float const xz_inv_cosine = axis.x*axis.z*inv_cosine;
	float const xsine = axis.x*temp.sine;
	float const ysine = axis.y*temp.sine;
	float const zsine = axis.z*temp.sine;

	result.i.set( sqr_x + (1.f - sqr_x)*temp.cosine,	xy_inv_cosine - zsine,				xz_inv_cosine + ysine,				0.f );
	result.j.set( xy_inv_cosine + zsine,				sqr_y + (1.f - sqr_y)*temp.cosine,	yz_inv_cosine - xsine,				0.f );
	result.k.set( xz_inv_cosine - ysine,				yz_inv_cosine + xsine,				sqr_z + (1.f - sqr_z)*temp.cosine,	0.f );
	result.c.set( 0.f,									0.f,								0.f,								1.f );

	return		result;
}