////////////////////////////////////////////////////////////////////////////
//	Created		: 02.06.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef ANIMATION_CONTROLLER_H_INCLUDED
#define ANIMATION_CONTROLLER_H_INCLUDED

#include "controller.h"

namespace xray {

namespace animation {
class i_animation_controller_set;
class i_animation_action;
} // namespace animation

namespace rtp {


template <class action>
class animation_controller:
	public controller< action >
{

typedef controller< action > super;

public:
	animation_controller ( xray::animation::i_animation_controller_set *set );

public:
	virtual		bool	dbg_update_walk			( float td );
	virtual		void	render					( xray::render::debug::renderer& renderer ) const;

public:
				void	debug_test_add_actions	( );

protected:
typename action::world_space_param_type			m_controller_position;
bool											m_debug_wolk_initialized;

protected:

static const u32								start_action_id					= 4;

}; // class animation_controller

} // namespace rtp
} // namespace xray

#include "animation_controller_inline.h"

#endif // #ifndef ANIMATION_CONTROLLER_H_INCLUDED