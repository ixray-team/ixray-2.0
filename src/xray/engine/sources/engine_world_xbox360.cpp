////////////////////////////////////////////////////////////////////////////
//	Created 	: 21.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "engine_world.h"
#include <xray/render/api.h>
#include <xray/core/core.h>

using xray::engine::engine_world;

xray::command_line::key		s_force_xbox_log("force_xbox_log", "", "logging", "forces logging even on xbox release and gold", "");

void engine_world::initialize_core	( )
{
#ifdef DEBUG
	core::initialize		( "render", core::create_log, core::perform_debug_initialization );
#else // #ifdef DEBUG
	core::initialize		( "render", (s_force_xbox_log.is_set() ? core::create_log : core::no_log), core::perform_debug_initialization );
#endif // #ifdef DEBUG
}

void engine_world::create_render	( )
{
	m_render_world			= render::create_world( *this, NULL );
	ASSERT					( m_render_world );
}

void engine_world::destroy_render	( )
{
	render::destroy_world	( m_render_world );
}

void engine_world::post_initialize	( )
{
	on_app_activate			( );
}

void engine_world::run				( )
{
	m_timer.start			( );

	for (;;) {
		if ( m_destruction_started )
			return;

		tick				( );
	}
}

void engine_world::initialize_editor	( )
{
}

void engine_world::draw_frame_editor	( )
{
	UNREACHABLE_CODE();
}

void engine_world::enter_editor_mode	( )
{
	UNREACHABLE_CODE();
}

bool engine_world::on_before_editor_tick( )
{
	UNREACHABLE_CODE(return true);
}

void engine_world::on_after_editor_tick	( )
{
	UNREACHABLE_CODE();
}

void engine_world::unload_editor		( )
{
	UNREACHABLE_CODE();
}

void engine_world::delete_processed_editor_orders ( bool const destroying )
{
	XRAY_UNREFERENCED_PARAMETER	( destroying );
}

pcstr engine_world::get_resource_path ( ) const
{
	return					"game:/resources";
}

pcstr engine_world::get_mounts_path		( ) const
{
	return					"game:/mounts";
}

pcstr engine_world::get_underscore_G_path ( ) const
{
	return					"game:/resources/scripts";
}

void engine_world::editor_clear_resources	( )
{
	UNREACHABLE_CODE();
}

void engine_world::enable_game				( bool value )
{
	enable_game_impl		( value );
}
