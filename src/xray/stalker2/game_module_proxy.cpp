////////////////////////////////////////////////////////////////////////////
//	Created		: 23.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/stalker2/game_module_proxy.h>
#include <xray/game/api.h>

using stalker2::game_module_proxy;
using xray::engine_user::world;

world* game_module_proxy::create_world			(
		xray::engine_user::engine& engine, 
		xray::input::world& input, 
		xray::render::world& render, 
		xray::ui::world& ui,
		xray::physics::world& physics,
		xray::rtp::world& rtp,
		xray::animation::world& animation,
		xray::sound::world& sound
	)
{
	return
		stalker2::game_module::create_world(
			engine, 
			input, 
			render, 
			ui,
			physics,
			rtp,
			animation,
			sound
		);
}

void game_module_proxy::destroy_world			( xray::engine_user::world*& world )
{
	stalker2::game_module::destroy_world		( world );
}

void game_module_proxy::set_memory_allocator	( xray::engine_user::allocator_type& allocator )
{
	stalker2::game_module::set_memory_allocator( allocator );
}