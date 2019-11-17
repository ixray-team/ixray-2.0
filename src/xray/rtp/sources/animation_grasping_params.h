////////////////////////////////////////////////////////////////////////////
//	Created		: 05.05.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef ANIMATION_GRASPING_PARAMS_H_INCLUDED
#define ANIMATION_GRASPING_PARAMS_H_INCLUDED

#include "grasping_space_params.h"
#include "animation_action_params.h"
//

namespace xray {
namespace rtp {





struct animation_grasping_world_params:
	public animation_world_params
{
	animation_grasping_world_params( const float4x4 &pos_controller );
	animation_grasping_world_params(  );

	void	save( xray::configs::lua_config_value config )const;
	
	void	load( const xray::configs::lua_config_value &config );

	float4x4 taget_pos;

}; // struct animation_grasping_world_params ;






typedef	 grasping_space_params		animation_grasping_space_params;

inline void	transform_to_param	( const float4x4 &transform,  animation_grasping_space_params& param );





} // namespace rtp
} // namespace xray



#include "animation_grasping_params_inline.h"

#endif // #ifndef ANIMATION_GARSPING_PARAMS_H_INCLUDED