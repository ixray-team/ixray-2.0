////////////////////////////////////////////////////////////////////////////
//	Created		: 17.06.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "engine_world.h"
#include "rpc.h"
#include <xray/render/base/engine_renderer.h>
#include <xray/editor/world/api.h>
#include <xray/editor/world/world.h>
#include <xray/editor/world/library_linkage.h>
#include <boost/bind.hpp>
#include <xray/core/core.h>
#include <xray/sound/world.h>

#if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
#	include <xray/os_preinclude.h>
#	undef NOUSER
#	undef NOMSG
#	undef NOMB
#	include <xray/os_include.h>
#	include <objbase.h>							// for COINIT_MULTITHREADED

	static HMODULE								s_editor_module	= 0;
	static xray::editor::create_world_ptr		s_create_world = 0;
	static xray::editor::destroy_world_ptr		s_destroy_world = 0;
	static xray::editor::memory_allocator_ptr	s_memory_allocator = 0;
	static xray::editor::property_holder*		s_holder = 0;
#endif // #if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360

using xray::engine::engine_world;

void engine_world::try_load_editor			( )
{
#if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
	R_ASSERT			( !s_editor_module );
	s_editor_module		= LoadLibrary( XRAY_EDITOR_FILE_NAME );

	if (!s_editor_module) {
		LOG_WARNING		( "cannot load library \"%s\"", XRAY_EDITOR_FILE_NAME );
		return;
	}

#if XRAY_PLATFORM_WINDOWS_32 && defined(DEBUG)
	xray::debug::enable_fpe	( false );
#endif // #if XRAY_PLATFORM_WINDOWS_32

	R_ASSERT			( !s_create_world );
	s_create_world		= (xray::editor::create_world_ptr)GetProcAddress(s_editor_module, "create_world");
	R_ASSERT			( s_create_world );

	R_ASSERT			( !s_destroy_world );
	s_destroy_world		= (xray::editor::destroy_world_ptr)GetProcAddress(s_editor_module, "destroy_world");
	R_ASSERT			( s_destroy_world );

	R_ASSERT			( !s_memory_allocator);
	s_memory_allocator	= (xray::editor::memory_allocator_ptr)GetProcAddress(s_editor_module, "memory_allocator");
	R_ASSERT			( s_memory_allocator );

	s_memory_allocator	( m_editor_allocator );

	// this function cannot be called before s_memory_allocator function called
	// because of a workaround of BugTrapN initialization from unmanaged code
	debug::change_bugtrap_usage	( core::debug::error_mode_verbose, core::debug::managed_bugtrap );

	R_ASSERT			( !m_editor );
	m_editor			= s_create_world ( *this );
	R_ASSERT			( m_editor );

	R_ASSERT			( !m_window_handle );
	m_window_handle		= m_editor->view_handle( );
	R_ASSERT			( m_window_handle );

	R_ASSERT			( !m_main_window_handle );
	m_main_window_handle = m_editor->main_handle( );
	R_ASSERT			( m_main_window_handle );

	m_editor->load		( );
#else // #if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
	UNREACHABLE_CODE	( );
#endif // #if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
}

void engine_world::unload_editor			( )
{
#if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
	ASSERT				( m_editor );

	ASSERT				( s_destroy_world );
	s_destroy_world		( m_editor );
	ASSERT				( !m_editor );

	ASSERT				( s_editor_module );
	FreeLibrary			( s_editor_module );
	s_editor_module		= 0;

	s_destroy_world		= 0;
	s_create_world		= 0;
#else // #if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
	UNREACHABLE_CODE	( );
#endif // #if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
}

void engine_world::initialize_editor		( )
{
	if( !command_line_editor() )
		return;

	m_game_enabled				= false;

	if( threading::core_count( ) == 1 ) {
		try_load_editor			( );
		return;
	}

	rpc::assign_thread_id		( rpc::editor, u32(-1) );

	threading::spawn			(
		boost::bind( &engine_world::editor, this ),
		!command_line_editor_singlethread( ) ? "editor" : "editor + logic",
		!command_line_editor_singlethread( ) ? "editor" : "editor + logic",
		0,
		0
	);

	rpc::run			(
		rpc::editor,
		boost::bind( &engine_world::try_load_editor, this),
		rpc::break_process_loop,
		rpc::dont_wait_for_completion
	);
}

void engine_world::initialize_editor_thread_ids	( )
{
	m_editor_allocator.user_current_thread_id			( );

	m_processed_editor.set_pop_thread_id				( );
	render_world().editor().set_command_push_thread_id	( );
	render_world().editor().initialize_command_queue	( XRAY_NEW_IMPL( m_editor_allocator, command_type_impl) );
}

void engine_world::editor				( )
{
#if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
	CoInitializeEx			( 0, COINIT_APARTMENTTHREADED );
#endif // #if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360

	rpc::assign_thread_id	( rpc::editor, threading::current_thread_id( ) );
	rpc::process			( rpc::editor );
	rpc::process			( rpc::editor );
	m_editor->run			( );

	if (!m_destruction_started)
		exit				( 0 );

	if ( rpc::is_same_thread(rpc::logic) )
		rpc::process		( rpc::logic );

	rpc::process			( rpc::editor );
}

void engine_world::draw_frame_editor		( )
{
	render_world().editor().set_command_processor_frame_id( render_world( ).engine().frame_id() + 1 );
	m_editor_frame_ended = true;

	if( m_logic_frame_ended || !m_game_enabled || m_game_enabled == m_game_paused_last )
	{
		render_world( ).engine().draw_frame	( );
		m_logic_frame_ended = false;
		m_editor_frame_ended = false;
		m_game_paused_last = !m_game_enabled;
	}
}

void engine_world::delete_processed_editor_orders	( bool destroying )
{
	delete_processed_orders		( m_processed_editor, m_editor_allocator, m_editor_frame_id, destroying );
}

bool engine_world::on_before_editor_tick	( )
{
	if ( threading::core_count( ) == 1 ) 	
		tick						( );
	else {
		m_render_world->engine().test_cooperative_level();

		static bool editor_singlethreaded = command_line_editor_singlethread ( ); 
		if ( editor_singlethreaded ) {
			logic_tick				( );
		}
		else {
			if ( m_editor_frame_id > m_render_world->engine().frame_id() + 1 )
				return				false;
		}
	}

	delete_processed_editor_orders	( false );
	return							true;
}

void engine_world::on_after_editor_tick	( )
{
	if ( threading::core_count( ) == 1 ) {
		++m_editor_frame_id;
		return;
	}

	if( !command_line_editor_singlethread() ) {
		++m_editor_frame_id;
		return;
	}

	while ( ( m_logic_frame_id > m_render_world->engine().frame_id( ) + 1 ) && !m_destruction_started ) {
		m_render_world->engine().test_cooperative_level();
		threading::yield			( 0 );
	}

//	++m_logic_frame_id;
	++m_editor_frame_id;
}

void engine_world::enter_editor_mode	( )
{
	if ( !m_editor )
		return;

	m_editor->editor_mode	( true );
}

void engine_world::editor_clear_resources	( )
{
	if ( !m_editor )
		return;

	resources::dispatch_callbacks	( );
//	m_editor->clear_resources		( );
	m_sound_world->clear_editor_resources( );
}