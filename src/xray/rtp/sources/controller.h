////////////////////////////////////////////////////////////////////////////
//	Created		: 03.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include "action.h"
#include "training_value_function.h"
#include "value_function.h"
#include "action_base.h"
#include "space_params.h"
#include "base_controller.h"

#include <xray/threading_extensions.h>

namespace xray {

namespace render {
namespace debug {
	struct renderer;
} // namespace debug
} // namespace render

namespace configs {
	class lua_config_value;
} // namespace configs

namespace rtp {

class player_object;



template <class action>
class controller: public base_controller {

public:
	typedef	typename action::space_param_type			space_param_type;
	typedef	typename action::world_space_param_type		world_space_param_type;
	typedef	typename action::global_actions_params_type global_actions_params_type;

private:	
	typedef	controller<action>							self_type;

public:
						controller			( const global_actions_params< action > &par );
	bool				update				( world_space_param_type &out_w, float	td );

private:
	void				next_action			( const world_space_param_type& w, space_param_type	const &params );

public:
action_base<action>						&base	( ){ return m_action_base; }
action_base<action>				const	&base	( )const{ return m_action_base; }
training_value_function<action>	const	&value	( ){ return m_value_function; }

public:
	virtual	void						learn						( );
	virtual	float						learn_step					( );
	virtual	void						step_logic					( bool learn );
	virtual	void						learn_init					( );

	virtual	void						learn_stop					( );

	void								walk_init					( u32 action_id, u32 blend_state, const world_space_param_type& start_position );
	
	const action						*current_action				( )const	{return m_action; }
	virtual	void						save						( xray::configs::lua_config_value cfg, bool training_tries )const;
	virtual	void						load						( const xray::configs::lua_config_value &cfg );
	virtual	void						save_training_sets			( xray::configs::lua_config_value cfg )const;
	virtual	void						load_training_sets			( const xray::configs::lua_config_value &cfg );

public:
	virtual	void						render_value				( xray::render::debug::renderer& renderer ) const;

protected:
inline	float							action_time					()const { return m_action_time; }
inline	u32								step						()const { return m_step; }
inline 	const world_space_param_type	&action_start_world_pos		()const { return m_action_start_world_pos; }
inline	void							set_action_start_world_pos	( const world_space_param_type& pos ) { m_action_start_world_pos = pos; }

private:
	virtual	void						from_world			( const world_space_param_type& w, space_param_type &p )const		= 0;
	virtual	void						on_new_walk_action	( world_space_param_type& w, space_param_type	&params );

private:
	world_space_param_type				m_action_start_world_pos;

	space_param_type					m_action_start_params;

	const action						*m_action;
	u32									m_blend_state;

	const action						*m_previous_action;
	u32									m_previous_blend_state;

	float								m_action_time;
	float								m_global_time;

private:
	training_value_function<action>		m_value_function;
	action_base<action>					m_action_base;
	threading::atomic32_type			m_b_learn_initialized;
	u32									m_step;
	
	

private:
	typedef typename threading::atomic_type_helper< float, boost::is_volatile<threading::atomic32_type>::value >::result residual_type;
	residual_type						m_last_residual;
	//threading::mutex					m_value_mutex;
}; // class controller

} // namespace rtp
} // namespace xray

#include "controller_inline.h"

#endif // #ifndef CONTROLLER_H_INCLUDED