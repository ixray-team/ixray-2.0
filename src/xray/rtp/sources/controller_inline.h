////////////////////////////////////////////////////////////////////////////
//	Created		: 13.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#include "action.h"
#include "value_function.h"
#include "action_base.h"
#include "space_params.h"

namespace xray {
namespace rtp {

template<class action>
void	controller<action>::next_action( const world_space_param_type& w, space_param_type	const &params )
{
			

			world_space_param_type	world_pos		=	w;
			space_param_type		local_params	=	params;
			
			on_new_walk_action			( world_pos, local_params  );


			m_action_start_world_pos	=	world_pos;
			m_action_start_params		=	local_params;

			u32 new_blend_state			=	m_blend_state;

			m_previous_action			=	m_action;

			m_action					=	m_value_function.pi( *m_action, m_blend_state, m_action_start_params, new_blend_state );

			LOG_DEBUG( "next action: %s, id: %d, blend_id: %d",  m_action->name(), m_action->id(), new_blend_state  );

			m_previous_blend_state		=	m_blend_state;
			m_blend_state				=	new_blend_state;
			
			

			++m_step;
			

			ASSERT( m_action );
}

template<class action>
inline void	controller<action>::on_new_walk_action	( world_space_param_type& w, space_param_type	&params )
{
	XRAY_UNREFERENCED_PARAMETERS( &w, &params );
}

template<class action>
bool	controller<action>::update( world_space_param_type &out_w, float td )
{
	m_action_time+=td;
	m_global_time+=td;
	float time = m_global_time;

	out_w = m_action_start_world_pos;

	do
	{
		float time_left = m_action->run( out_w, m_action_start_params, *m_previous_action, m_previous_blend_state, m_blend_state, m_step, time ) ;



		if( time_left <= 0.f )
		{

			m_action_time			=	-time_left;

			if( m_action->taget( m_action_start_params ) )
				return true;
			
			if( m_action->is_taget_action() )
			{
				LOG_DEBUG("taget not reached");
				return true;
			}

			space_param_type			params;
			from_world					( out_w, params );

#ifdef		DEBUG
			space_param_type			start_params;								//debug
			from_world					( m_action_start_world_pos, start_params );	//debug

			ASSERT( m_action_start_params.similar		( start_params )  );	

			m_action->run				(  start_params, *m_previous_action, m_previous_blend_state,  m_blend_state );//debug
			ASSERT( params.similar		( start_params )  );							//debug
#endif
			
			next_action( out_w, params );

			continue;
		}
		
	} while( xray::identity(false) );

	return false;
}

template< class action >
controller< action >::controller( const global_actions_params< action > &par ):
m_action( 0 ),
m_blend_state( 0 ),
m_previous_action( 0 ),
m_previous_blend_state( 0 ),
m_action_time( 0 ),
m_global_time( 0 ),
m_action_base( par ),
m_b_learn_initialized( false ),
m_value_function( m_action_base ),
m_step( 0 ),
m_last_residual( -1.f )
{
	;
};

template< class action >
inline void controller< action >::learn( )
{

	long initialized = false;
	ASSERT( m_b_learn_initialized );
	do{
		m_last_residual = learn_step(); 

	initialized = m_b_learn_initialized;


	}while ( initialized );
	

	//m_value_function.learn();
}

template< class action >
inline void	controller< action >::learn_init		( )
{


	if( m_b_learn_initialized )
		return;

	ASSERT( !m_b_learn_initialized );
	
	m_value_function.learn_init();
	threading::thread_function_type const  fun = boost::bind(  &self_type::learn, this );
	m_b_learn_initialized =true; 
	
	threading::spawn( fun , "rtp_learn", "rtp_learn", 0, 0 );

}

template< class action >
inline	void		controller< action >::step_logic( bool  )
{
	if( !m_b_learn_initialized )
		return;
	if( m_value_function.samples_step() )
		LOG_DEBUG("rtp residual: %f ", m_last_residual );
}

template< class action >
inline float	controller< action >::learn_step		( )
{

	float ret = m_value_function.learn_step();
	ASSERT(sizeof(long)==sizeof(float));
	threading::interlocked_exchange( (threading::atomic32_type&)m_last_residual, (long)ret );
	return ret;

}

template< class action >
inline void controller< action >::learn_stop			( )
{
	threading::interlocked_exchange			( m_b_learn_initialized, false );
}


template<class action>
inline void		controller<action>::render_value			( xray::render::debug::renderer& renderer ) const
{
	 m_value_function.render( renderer );
}

template<class action>
void		controller<action>::walk_init		(  u32 action_id, u32 blend_state, const world_space_param_type& start_position   )
{
	ASSERT( action_id < m_action_base.size() );
	m_action = m_action_base.get( action_id );
	m_previous_action = m_action;
	m_action_start_world_pos = start_position;
	from_world( m_action_start_world_pos, m_action_start_params );

	m_action_time = 0;
	m_global_time = 0;
	m_previous_blend_state = blend_state;
	m_blend_state = blend_state;
	m_step = 0;
}

template<class action>
inline void	controller<action>::save( xray::configs::lua_config_value config, bool training_tries )const
{
		xray::configs::lua_config_value cfg = config["controller"];
		cfg["type"]					= type ();
		m_action_start_world_pos	.save(cfg["world_pos"]);
		cfg["action_time"]			= m_action_time;
		cfg["blend_state"]			= m_blend_state;
		m_action_base				.save(cfg["action_base"]);
		cfg["action_id"]			= !m_action ? u32(-1) : m_action->id();
		m_value_function			.save(cfg["value_function"],training_tries);
}

template<class action>
inline void	controller<action>::load( const xray::configs::lua_config_value &config )
{
		const xray::configs::lua_config_value cfg = config["controller"];
		m_action_start_world_pos	.load(cfg["world_pos"]);
		m_action_time				=cfg["action_time"];
		m_global_time				= 0;
		m_blend_state				=cfg["blend_state"];
		m_action_base				.load(cfg["action_base"]);
		u32 id						=cfg["action_id"];
		m_action					= ( id == u32(-1) ) ? 0 : m_action_base.get( id );
		m_value_function			.load(cfg["value_function"]);
}

template<class action>
inline void	controller<action>::save_training_sets	( xray::configs::lua_config_value cfg )const
{
	m_value_function.save_training_sets( cfg );
}

template<class action>
inline void	controller<action>::load_training_sets	( const xray::configs::lua_config_value &cfg )
{
	m_value_function.load_training_sets( cfg["controller"]["value_function"] );
}

} // namespace rtp
} // namespace xray
