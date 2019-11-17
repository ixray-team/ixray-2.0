////////////////////////////////////////////////////////////////////////////
//	Created		: 24.12.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef ANIM_TRACK_COMMON_H_INCLUDED
#define ANIM_TRACK_COMMON_H_INCLUDED

namespace xray {
namespace animation {

enum enum_channel_id
{
	channel_translation_x = 0,
	channel_translation_y,
	channel_translation_z,
	channel_rotation_x,
	channel_rotation_y,
	channel_rotation_z,
	channel_scale_x,
	channel_scale_y,
	channel_scale_z,
	channel_max,
	channel_unknown
};

struct	frame
{
#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable:4201)
#endif // #ifdef _MSC_VER
	union{
		struct{
			xray::math::float3_pod translation;
			xray::math::float3_pod rotation;
			xray::math::float3_pod scale;
		};
		float channels[channel_max];
	};
#ifdef _MSC_VER
#	pragma warning(pop)
#endif // #ifdef _MSC_VER
}; // struct frame

//\li matrix = [S] * [RO] * [R] * [JO] * [IS] * [T]



inline void frame_matrix( const frame &f, float4x4 &matrix_, const float3 &parent_scale_, float3 &scale_ )
{
	
	XRAY_UNREFERENCED_PARAMETER( parent_scale_);

	scale_ = f.scale;

	float4x4 rotation_		=			create_rotation		( f.rotation );

	
	//matrix =	scale*rotation*translation*parent_scale*translation ;
	matrix_ = rotation_;
	matrix_.c.xyz() = f.translation;
}

inline void frame_matrix( const frame &f, float4x4 &matrix )
{
	float3 scale;
	float3 parent_scale( 1, 1, 1 );
	animation::frame_matrix( f, matrix, parent_scale, scale );
}

inline float4x4 frame_matrix( const frame &f )
{
	float4x4 ret;
	frame_matrix( f, ret );
	return ret;
}

inline  frame matrix_to_frame( const float4x4 &m )
{
	frame f;
	f.rotation = m.get_angles_xyz( );
	f.translation = m.c.xyz();
	f.scale = float3(1,1,1);
	return f;
}

static	const frame zero = { float3().set(0,0,0),float3().set(0,0,0),float3().set(0,0,0) };
static	const float	default_fps	= 30.f;




} // namespace  animation
} // namespace  xray

#endif // #ifndef ANIM_TRACK_COMMON_H_INCLUDED