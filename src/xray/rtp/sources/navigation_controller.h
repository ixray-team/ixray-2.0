////////////////////////////////////////////////////////////////////////////
//	Created		: 02.06.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef NAVIGATION_CONTROLLER_H_INCLUDED
#define NAVIGATION_CONTROLLER_H_INCLUDED

#include "animation_controller.h"
#include "navigation_action.h"


namespace xray {
namespace rtp {

class navigation_controller :
	public animation_controller< navigation_action > 
{
	typedef animation_controller< navigation_action > super;

public:
						navigation_controller	( xray::animation::i_animation_controller_set *set );

public:
	virtual		bool	dbg_update_walk			( float td );
	virtual		void	render					( xray::render::debug::renderer& renderer ) const;
	virtual	void		dbg_move_control		( float4x4 &view_inverted, input::world& input_world  );
private:
	virtual		pcstr	type					( )const { return "navigation"; }

private:
	virtual		void	from_world				( const world_space_param_type& w, space_param_type &p )const;
	virtual		void	on_new_walk_action		( world_space_param_type& w, space_param_type	&params );

private:
	float2				m_control_path;


private:
	float				m_dbg_cam_height;
	float				m_dbg_cam_dist;

}; // class navigation_controller

} // namespace rtp
} // namespace xray

#endif // #ifndef NAVIGATION_CONTROLLER_H_INCLUDED