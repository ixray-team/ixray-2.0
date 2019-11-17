////////////////////////////////////////////////////////////////////////////
//	Created 	: 11.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "game.h"

using xray::non_null;
using xray::raw;

namespace stalker2 {

static xray::uninitialized_reference<game>	s_world;


xray::engine_user::allocator_type*	g_allocator = NULL;

xray::engine_user::world* game_module::create_world	(
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
	XRAY_CONSTRUCT_REFERENCE	( s_world, game )	( engine, input, render, ui, physics, rtp, animation, sound );

	return						( &*s_world );
}

void game_module::destroy_world						( xray::engine_user::world*& world )
{
	R_ASSERT					( &*s_world == world );
	XRAY_DESTROY_REFERENCE		( s_world );
	world						= 0;
}

void game_module::set_memory_allocator				( xray::engine_user::allocator_type& allocator )
{
	ASSERT						( !g_allocator );
	g_allocator					= &allocator;
}

#ifndef XRAY_STATIC_LIBRARIES

static void cleanup						( )
{
	if ( !s_world.initialized() )
		return;

	LOG_ERROR					( "game::world leaked" );

	//raw< xray::engine_user::world >::ptr world = &*s_world;
	//xray::game::destroy_world	( world );
}

#include <xray/os_include.h>

BOOL APIENTRY DllMain					( HANDLE hModule, unsigned int ul_reason_for_call, LPVOID lpReserved )
{
	XRAY_UNREFERENCED_PARAMETERS( hModule, lpReserved );
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: {
			break;
		}
		case DLL_PROCESS_DETACH: {
			cleanup				( );
			break;
		}
	}
	return						( TRUE );
}

#endif // #ifndef XRAY_STATIC_LIBRARIES

} // namespace stalker2