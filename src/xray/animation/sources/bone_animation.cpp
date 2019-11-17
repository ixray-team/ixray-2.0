////////////////////////////////////////////////////////////////////////////
//	Created		: 19.01.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "bone_animation.h"

using namespace xray::math;

namespace xray {
namespace animation {

void	bone_animation::bone_matrix( float time, const float3 &parent_scale_, float4x4 &matrix, float3 &scale_  )const
{
	frame f;
	evaluate_frame( time, m_channels, f );
	frame_matrix( f, matrix, parent_scale_, scale_ );

}

} // namespace animation
} // namespace xray