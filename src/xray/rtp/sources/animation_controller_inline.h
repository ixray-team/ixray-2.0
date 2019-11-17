////////////////////////////////////////////////////////////////////////////
//	Created		: 02.06.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef ANIMATION_CONTROLLER_INLINE_H_INCLUDED
#define ANIMATION_CONTROLLER_INLINE_H_INCLUDED

namespace xray {
namespace rtp {

template <class action>
animation_controller<action>::animation_controller( xray::animation::i_animation_controller_set *set ):
controller< action >( typename action::global_actions_params_type ( set ) ), 
	//m_debug_global( set ), 
	m_debug_wolk_initialized( false ),
	m_controller_position( set->position() )
{}




template <class action>
inline void	animation_controller<action>::debug_test_add_actions( )
{
		
	

	ASSERT( super::base().global_params().m_set );

	const u32 num_actions = super::base().global_params().m_set->num_actions();
	for( u32 i = 0; i < num_actions; ++i )
	{
		action *a = NEW( action )( super::base() , super::base().global_params().m_set->action(i) );
		a->construct();
	}
}



template <class action>
inline bool	animation_controller<action>::dbg_update_walk( float dt )
{ 
	
	XRAY_UNREFERENCED_PARAMETERS( dt );
	
	if( super::value().empty() )
		return false;

	if( !m_debug_wolk_initialized  )
	{
		ASSERT(  super::base().global_params().m_set );

		walk_init( start_action_id, 0, m_controller_position );
		m_debug_wolk_initialized = true;

	}

	 m_controller_position.controller_pos =   super::base().global_params().m_set->position();

	 return false;
}

extern bool b_render_learn;

template <class action>
inline void		animation_controller<action>::render( xray::render::debug::renderer& r ) const
{
	
	if( b_render_learn )
		super::render_value( r );

	super::base().global_params().m_set->render( r, super::action_time() );

}


} // namespace rtp
} // namespace xray

#endif // #ifndef ANIMATION_CONTROLLER_INLINE_H_INCLUDED