////////////////////////////////////////////////////////////////////////////
//	Created 	: 22.12.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sound_world.h"
#include <xray/sound/api.h>
#include <xray/linkage_helper.h>

#ifndef XRAY_STATIC_LIBRARIES
	DECLARE_LINKAGE_ID(sound_entry_point)
#endif // #ifndef XRAY_STATIC_LIBRARIES

using xray::sound::sound_world;

static xray::uninitialized_reference<sound_world>	s_world;

xray::sound::allocator_type*	xray::sound::g_allocator = 0;

xray::sound::world* xray::sound::create_world	( engine& engine, base_allocator_type* logic_allocator, base_allocator_type* editor_allocator )
{
	XRAY_CONSTRUCT_REFERENCE	( s_world, sound_world )	( engine, logic_allocator, editor_allocator );
	return						( &*s_world );
}

void xray::sound::destroy_world					( xray::sound::world*& world )
{
	R_ASSERT					( &*s_world == world );
	XRAY_DESTROY_REFERENCE		( s_world );
	world						= 0;
}

void xray::sound::memory_allocator				( allocator_type& allocator )
{
	ASSERT						( !g_allocator );
	g_allocator					= &allocator;
}

void xray::sound::dispatch_logic_callbacks( )
{
	s_world->process_logic_callback_orders();
}

void xray::sound::dispatch_editor_callbacks( )
{
	s_world->process_editor_callback_orders();
}
