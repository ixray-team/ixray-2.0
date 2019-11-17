////////////////////////////////////////////////////////////////////////////
//	Created		: 23.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef GAME_MODULE_PROXY_H_INCLUDED
#define GAME_MODULE_PROXY_H_INCLUDED

#include <xray/engine/engine_user.h>

namespace stalker2 {

class game_module_proxy : public xray::engine_user::module_proxy {
private:
	virtual	xray::engine_user::world*	create_world	(
						xray::engine_user::engine& engine, 
						xray::input::world& input, 
						xray::render::world& render, 
						xray::ui::world& ui,
						xray::physics::world& physics,
						xray::rtp::world& rtp,
						xray::animation::world& animation,
						xray::sound::world& sound
					);
	virtual	void					destroy_world		( xray::engine_user::world*& world );
	virtual	void					set_memory_allocator( xray::engine_user::allocator_type& allocator );
}; // struct game_proxy

} // namespace stalker2

#endif // #ifndef GAME_MODULE_PROXY_H_INCLUDED