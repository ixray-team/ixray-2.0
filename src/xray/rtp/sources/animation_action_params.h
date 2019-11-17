////////////////////////////////////////////////////////////////////////////
//	Created		: 02.06.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef ANIMATION_ACTION_PARAMS_H_INCLUDED
#define ANIMATION_ACTION_PARAMS_H_INCLUDED

namespace xray {

namespace animation
{
	class i_animation_controller_set;
} // namespace animation

namespace rtp {

class animation_global_actions_params
{

public:
	animation_global_actions_params( xray::animation::i_animation_controller_set *set );
	

public:
	xray::animation::i_animation_controller_set		*m_set;

};

struct animation_world_params
{
	animation_world_params( const float4x4 &pos_controller ): controller_pos(pos_controller)
	{}
	animation_world_params( ):controller_pos(float4x4().identity()){}
	
	void	save( xray::configs::lua_config_value  )const{};
	
	void	load( const xray::configs::lua_config_value & ){};

	float4x4 controller_pos;
};

} // namespace rtp
} // namespace xray

#endif // #ifndef ANIMATION_ACTION_PARAMS_H_INCLUDED