////////////////////////////////////////////////////////////////////////////
//	Created		: 17.06.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "engine_world.h"
#include "rpc.h"
#include <xray/input/api.h>
#include <xray/input/world.h>
#include <xray/ui/api.h>
#include <xray/ui/world.h>
#include <xray/physics/api.h>
#include <xray/physics/world.h>
#include <xray/network/api.h>
#include <xray/sound/api.h>
#include <xray/sound/world.h>
#include <xray/render/base/game_renderer.h>
#include <xray/render/base/engine_renderer.h>
#include <boost/bind.hpp>
#include <xray/rtp/api.h>
#include <xray/engine/engine_user.h>
#include <xray/animation/api.h>

using xray::engine::engine_world;

bool engine_world::setup_engine_user	( xray::engine_user::module_proxy* module_proxy )
{
	ASSERT						( m_engine_user_module_proxy == NULL );
	ASSERT						( m_engine_user_world == NULL );

	m_engine_user_module_proxy	= module_proxy;
	return						true;
}

void engine_world::initialize_input		( )
{
	ASSERT						( m_initialized );
	input::memory_allocator		( m_input_allocator );

	ASSERT						( !m_input_world );
	m_input_world				= input::create_world( *this, m_main_window_handle );
	ASSERT						( m_input_world );

}

void engine_world::initialize_engine_user		( )
{
	ASSERT						( m_initialized );
	ASSERT						( m_input_world );
	ASSERT						( m_engine_user_module_proxy );
	ASSERT						( !m_engine_user_world );

	m_engine_user_module_proxy->set_memory_allocator	( m_engine_user_allocator );

	m_engine_user_world			= m_engine_user_module_proxy->create_world( 
									*this, 
									*m_input_world, 
									*m_render_world, 
									*m_ui_world, 
									*m_physics_world, 
									*m_rtp_world, 
									*m_animation_world,
									*m_sound_world 
									);

	engine_user_world().enable		( m_game_enabled );
}

void engine_world::initialize_physics	( )
{
	ASSERT						( m_initialized );
	physics::memory_allocator	( m_physics_allocator );

	ASSERT						( !m_physics_world );
	m_physics_world				= physics::create_world( *this, m_render_world->game().debug() );
	ASSERT						( m_physics_world );

}

void engine_world::initialize_rtp	( )
{
	ASSERT						( m_initialized );
	rtp::memory_allocator		( m_rtp_allocator );
	
	ASSERT						( !m_rtp_world );
	m_rtp_world					= rtp::create_world( *this  );
	ASSERT						( m_rtp_world );
}

void engine_world::initialize_ui		( )
{
	ASSERT						( m_initialized );
	ui::memory_allocator		( m_ui_allocator );

	ASSERT						( !m_ui_world );
	m_ui_world					= ui::create_world( *this, *m_input_world, *m_render_world );
	ASSERT						( m_ui_world );
}

void engine_world::initialize_animation	( )
{
	ASSERT						( m_initialized );

	ASSERT						( !m_animation_world );
	m_animation_world			= animation::create_world( *this );
	ASSERT						( m_animation_world );
}

void engine_world::initialize_logic_modules( )
{
	rpc::assign_thread_id							( rpc::logic, threading::current_thread_id( ) );

	m_input_allocator.user_current_thread_id		( );
	m_engine_user_allocator.user_current_thread_id	( );
	m_ui_allocator.user_current_thread_id			( );
	m_rtp_allocator.user_current_thread_id			( );
	m_physics_allocator.user_current_thread_id		( );
	m_animation_allocator.user_current_thread_id	( );

	render_world().game().set_command_push_thread_id( );
	m_processed_logic.set_pop_thread_id				( );

	initialize_input								( );
	
	initialize_physics								( );
	initialize_ui									( );
	initialize_animation							( );
	initialize_rtp									( );
	initialize_engine_user							( );

}

void engine_world::initialize_logic		( )
{
	ASSERT						( threading::core_count ( ) );
 	m_render_world->engine().draw_frame( );
	m_logic_frame_id			= 1;
	m_render_world->engine().test_cooperative_level();

	m_engine_user_allocator.user_current_thread_id	( );

	render_world().game().set_command_push_thread_id( );
	render_world().game().initialize_command_queue	( XRAY_NEW_IMPL(m_engine_user_allocator, command_type_impl) );

	if ( threading::core_count() == 1 ) {
		initialize_logic_modules( );
		return;
	}

	if ( m_editor && command_line_editor_singlethread() ) {
		rpc::run				( rpc::logic, boost::bind( &engine_world::initialize_logic_modules, this ), rpc::continue_process_loop, rpc::dont_wait_for_completion );
		return;
	}

	rpc::assign_thread_id		( rpc::logic, u32(-1) );
	threading::spawn			( boost::bind( &engine_world::logic, this ), 
								  "logic", 
								  "logic", 
								  0,
								  1 % threading::core_count() );
}

void engine_world::finalize_logic_modules	( )
{
	m_sound_world->clear_logic_resources( );
	m_engine_user_module_proxy->destroy_world(m_engine_user_world);
	physics::destroy_world		( m_physics_world );
	input::destroy_world		( m_input_world );
	ui::destroy_world			( m_ui_world );
	animation::destroy_world	( m_animation_world );
}

void engine_world::delete_processed_logic_orders	( bool destroying )
{
	delete_processed_orders		( m_processed_logic, m_engine_user_allocator, m_logic_frame_id, destroying );
}

static void logic_dispatch_callbacks		( )
{
	xray::resources::dispatch_callbacks		( );
	xray::network::dispatch_callbacks		( );
}

void engine_world::logic_tick				( )
{
	delete_processed_logic_orders( false );

	logic_dispatch_callbacks			( );
	xray::sound::dispatch_logic_callbacks	( );

	if ( !m_game_enabled || !app_is_active()) {
		if ( !m_game_enabled )
			++m_logic_frame_id;

 		return;
	}
	
	m_engine_user_world->tick			( m_logic_frame_id );
	render_world().game( ).draw_frame	( );
	++m_logic_frame_id;
}

void engine_world::logic				( )
{
	initialize_logic_modules	( );

	for (;;) {
		if ( m_destruction_started )
			break;

		logic_tick				( );
		
		while ( ( m_logic_frame_id > m_render_world->engine().frame_id( ) + 1 ) && !m_destruction_started ) {
			if ( !rpc::try_process_single_call( rpc::logic ) )
				threading::yield( 1 );
		}
	}

	rpc::process				( rpc::logic );
}

void engine_world::draw_frame_logic				( )
{
	render_world().game().set_command_processor_frame_id( render_world( ).engine().frame_id() + 1 );

	m_logic_frame_ended			= true;

	if ( command_line_editor() && !m_editor_frame_ended )
		return;

	render_world( ).engine().draw_frame	( );

	m_logic_frame_ended			= false;
	m_editor_frame_ended		= false;
	m_game_paused_last			= false;
	m_game_paused_last			= !m_game_enabled;
}

void engine_world::logic_clear_resources		( )
{
	logic_dispatch_callbacks		( );

	m_engine_user_world->clear_resources( );
	m_physics_world->clear_resources( );
	m_input_world->clear_resources	( );
	m_ui_world->clear_resources		( );
}