////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RTP_ANIMATION_GRASPING_CONTROLLER_H_INCLUDED
#define XRAY_RTP_ANIMATION_GRASPING_CONTROLLER_H_INCLUDED

#include "animation_controller.h"
#include "animation_grasping_action.h"

namespace xray {
namespace rtp {

class animation_grasping_controller:
	public animation_controller< animation_grasping_action > 
{
	typedef animation_controller< animation_grasping_action > super;

public:
	animation_grasping_controller ( xray::animation::i_animation_controller_set *set );

public:
	virtual		bool	dbg_update_walk			( float td );

public:
	virtual	void		render					( xray::render::debug::renderer& renderer ) const;

private:
	virtual		pcstr	type					( )const { return "animation_grasping"; }

private:
	virtual		void	from_world				( const world_space_param_type& w, space_param_type &p )const;

private:
				void	dbg_set_random_taget	();

private:

}; // class animation_grasping_controller

} // namespace rtp
} // namespace xray

#endif // #ifndef XRAY_RTP_ANIMATION_GRASPING_CONTROLLER_H_INCLUDED