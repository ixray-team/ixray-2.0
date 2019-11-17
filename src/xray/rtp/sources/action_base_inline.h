////////////////////////////////////////////////////////////////////////////
//	Created		: 06.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "animation_grasping_action.h"

namespace xray {
namespace rtp {

template< class action >
u32		action_base<action>::add		( action* a	)
{
	actions.push_back( a );
	return actions.size() - 1;
}

template< class action >
const	action*	action_base<action>::get 	( u32 id ) const	
{
	ASSERT( id<actions.size() );
	return actions[id];
}

template< class action >
action_base<action>::action_base( const global_actions_params< action > &par ): m_global_params( par )
{}


template< class action >
action_base<action>::~action_base()
{
	clear();
}

template< class action >
void action_base<action>::clear()
{
	const u32 sz = size();
	for( u32 i = 0; i<sz; ++i )
		DELETE(actions[i]);
	actions.clear();
}

template< class action >
void action_base<action>::save	( xray::configs::lua_config_value cfg )const
{
	const u32 sz = actions.size();
	cfg["size"] = sz;
	for(u32 i = 0; i < sz ; ++i )
		actions[i]->save(cfg["actions"][i]);

}

template< class action >
void action_base<action>::load	( const xray::configs::lua_config_value &cfg )
{
	clear();
	
	u32 sz = cfg["size"];
	//actions.resize( sz, 0 );
	for(u32 i = 0; i < sz ; ++i )
	{
		NEW( action_type )(*this);//actions[i] = 
		actions[i]->load(cfg["actions"][i]);
		ASSERT( actions[i]->id() == i );
	}
}

} // namespace rtp
} // namespace xray