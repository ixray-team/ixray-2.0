////////////////////////////////////////////////////////////////////////////
//	Created 	: 21.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "engine_world.h"
#include <xray/input/api.h>
#include <xray/input/world.h>
#include <xray/animation/api.h>
#include <xray/render/base/engine_renderer.h>
#include <xray/resources_fs.h>
#include <xray/core/core.h>
#include "rpc.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <xray/network/api.h>
#include <xray/sound/api.h>
#include <xray/fs_utils.h>

using xray::engine::engine_world;

static xray::command_line::key			s_editor_key	("editor", "", "", "run editor");
static xray::command_line::key			s_editor_st_key	("editor_single_thread", "editor_st", "", "run editor in single thread");

static xray::command_line::key			s_no_splash_screen_key	("no_splash_screen", "", "application", "suppress splash screen on startup");

engine_world::engine_world				( pcstr command_line, pcstr application, pcstr build_date ) :
	m_input_world						( NULL ),
	m_network_world						( NULL ),
	m_sound_world						( NULL ),
	m_render_world						( NULL ),
	m_window_handle						( NULL ),
	m_main_window_handle				( NULL ),
	m_last_frame_time					( 0 ),
	m_logic_frame_id					( 0 ),
	m_editor_frame_id					( 0 ),
	m_destruction_started				( 0 ),
	m_early_destruction_started			( false ),
	m_exit_code							( 0	),
	m_logic_frame_ended					( false ),
	m_editor_frame_ended				( false ),
	m_initialized						( false ),
	m_game_enabled						( true ),
	m_game_paused_last					( false ),
	m_engine_user_module_proxy			( NULL ),
	m_engine_user_world					( NULL ),
	m_app_active						( false )
{
	core::preinitialize					( this, 
										  command_line, 
										  command_line::contains_application_true, 
										  application,
										  build_date
										);

	g_allocator.do_register				(  64*Kb,	"engine"	);
	m_input_allocator.do_register		(  64*Kb,	"input"		);
	m_engine_user_allocator.do_register	(	1*Mb,	"game"		);
	m_render_allocator.do_register		(	8*Mb,	"render"	);
	m_ui_allocator.do_register			(  64*Kb,	"ui"		);
	m_physics_allocator.do_register		(  64*Kb,	"physics"	);
	m_network_allocator.do_register		(  64*Kb,	"network"	);
#if !XRAY_PLATFORM_XBOX_360
	m_rtp_allocator.do_register			(  32*Mb,	"rtp"		);
#else // #if !XRAY_PLATFORM_XBOX_360
	m_rtp_allocator.do_register			(  64*Kb,	"rtp"		);
#endif // #if !XRAY_PLATFORM_XBOX_360
	m_sound_allocator.do_register		(	1*Mb,	"sound"		);
	m_animation_allocator.do_register	(  16*Mb,	"animation"	);

#ifdef XRAY_STATIC_LIBRARIES
	R_ASSERT_BOX						(!s_editor_key && !s_editor_st_key, "Editor is not supported in static configurations!");
#endif // #ifdef XRAY_STATIC_LIBRARIES

	if ( command_line_editor() )
		m_editor_allocator.do_register	( 512*Mb,	"editor"	);

	memory::allocate_region				( );

	m_processed_logic.set_push_thread_id	( );
	m_processed_logic.push_null_node		( XRAY_NEW_IMPL( m_engine_user_allocator, command_type_impl) );

	if ( command_line_editor() ) {
		m_processed_editor.set_push_thread_id	( );
		m_processed_editor.push_null_node		( XRAY_NEW_IMPL( m_editor_allocator, command_type_impl) );
	}
}

static bool s_resource_callbacks_dispatched	= false;

static void thread_dispatch_callbacks	( )
{
	while ( !s_resource_callbacks_dispatched ) {
		xray::resources::dispatch_callbacks	( );
		xray::threading::yield				( 0 );
	}
}

struct empty {
	static void function( ){}// we need this function for synchronization purposes
}; // struct empty

engine_world::~engine_world		( )
{
	if ( m_early_destruction_started )
	{
		if ( command_line_editor() ) {
			m_processed_editor.set_pop_thread_id( );
			delete_processed_editor_orders	( true );
		}

		m_processed_logic.set_pop_thread_id	( );
		delete_processed_logic_orders	( true );

		core::finalize					( );
		return;
	}

	rpc::run							( rpc::logic,	boost::bind(&empty::function), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run							( rpc::network,	boost::bind(&empty::function), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run							( rpc::sound,	boost::bind(&empty::function), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run							( rpc::editor,	boost::bind(&empty::function), rpc::continue_process_loop, rpc::dont_wait_for_completion );

	rpc::wait							( rpc::logic );
	rpc::wait							( rpc::network );
	rpc::wait							( rpc::sound );
	rpc::wait							( rpc::editor );

	// clear resources
	rpc::run							( rpc::logic,	boost::bind(&engine_world::logic_clear_resources, this), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run							( rpc::network,	boost::bind(&engine_world::network_clear_resources, this), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run							( rpc::sound,	boost::bind(&engine_world::sound_clear_resources, this), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	if ( m_editor )
		rpc::run						( rpc::editor,	boost::bind(&engine_world::editor_clear_resources, this), rpc::continue_process_loop, rpc::dont_wait_for_completion );

	m_render_world->clear_resources		( );

	rpc::wait							( rpc::logic );
	rpc::wait							( rpc::network );
	rpc::wait							( rpc::sound );
	rpc::wait							( rpc::editor );

	if ( command_line_editor() )
		m_render_world->editor().purge_orders ( );
	m_render_world->engine().purge_orders ( );

	if ( command_line_editor() )
		rpc::run						( rpc::editor,	boost::bind(&engine_world::delete_processed_editor_orders, this, true), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run							( rpc::logic,	boost::bind(&engine_world::delete_processed_logic_orders, this, true), rpc::continue_process_loop, rpc::dont_wait_for_completion );

	s_resource_callbacks_dispatched		= false;

	rpc::run_remote_only				( rpc::logic,	boost::bind(&thread_dispatch_callbacks), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run_remote_only				( rpc::network,	boost::bind(&thread_dispatch_callbacks), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run_remote_only				( rpc::sound,	boost::bind(&thread_dispatch_callbacks), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run_remote_only				( rpc::editor,	boost::bind(&thread_dispatch_callbacks), rpc::continue_process_loop, rpc::dont_wait_for_completion );

	resources::wait_and_dispatch_callbacks ( true );

	s_resource_callbacks_dispatched		= true;

	rpc::wait							( rpc::logic );
	rpc::wait							( rpc::network );
	rpc::wait							( rpc::sound );
	rpc::wait							( rpc::editor );

	rpc::run							( rpc::logic,	boost::bind(&engine_world::finalize_logic_modules, this), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run_remote_only				( rpc::logic,	boost::bind(&resources::finalize_thread_usage, false), rpc::break_process_loop, rpc::dont_wait_for_completion );

	rpc::run							( rpc::network,	boost::bind(&network::destroy_world, m_network_world), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run_remote_only				( rpc::network,	boost::bind(&resources::finalize_thread_usage, false), rpc::break_process_loop, rpc::dont_wait_for_completion );

	rpc::run							( rpc::sound,	boost::bind(&sound::destroy_world, m_sound_world), rpc::continue_process_loop, rpc::dont_wait_for_completion );
	rpc::run_remote_only				( rpc::sound,	boost::bind(&resources::finalize_thread_usage, false), rpc::break_process_loop, rpc::dont_wait_for_completion );

	if ( m_editor ) {
		rpc::run						( rpc::editor,	boost::bind(&engine_world::unload_editor, this), rpc::continue_process_loop, rpc::dont_wait_for_completion );
		rpc::run_remote_only			( rpc::editor,	boost::bind(&resources::finalize_thread_usage, false), rpc::continue_process_loop, rpc::wait_for_completion );
	}

	rpc::wait							( rpc::logic );
	rpc::wait							( rpc::network );
	rpc::wait							( rpc::sound );
	rpc::wait							( rpc::editor );

	// for resources which were allocated in render thread,
	// but lived in other threads
 	resources::wait_and_dispatch_callbacks ( true );

	destroy_render						( );

	rpc::run_remote_only				( rpc::editor,	boost::bind(&empty::function), rpc::break_process_loop, rpc::wait_for_completion );

	core::finalize						( );
}

void engine_world::initialize_render	( )
{
	ASSERT								( m_initialized );

	render::memory_allocator			( m_render_allocator, m_engine_user_allocator, m_editor_allocator );

	ASSERT								( !m_render_world );
	create_render						( );
	ASSERT								( m_render_world );
}

static void on_mounted_disk				( bool const result )
{
	XRAY_UNREFERENCED_PARAMETER			( result );
	R_ASSERT							( result );
}

#ifdef DEBUG
static xray::timing::timer				s_debug_engine_timer;
#endif // #ifdef DEBUG

void log_fs_callback ( xray::resources::fs_iterator iterator )
{
	xray::resources::log_fs_iterator	( iterator );
}

void engine_world::initialize		( )
{
	ASSERT								( !m_initialized );
	m_initialized						= true;

	initialize_core						( );

	if ( m_destruction_started )
	{
		m_early_destruction_started		= true;
		return;
	}

	R_ASSERT_BOX						(fs::get_path_info(NULL, get_resource_path()) == fs::path_info::type_folder,
										"resources are not found on path: '%s'! running from wrong working directory?", 
										get_resource_path());

 	resources::query_mount				( "resources", on_mounted_disk, &g_allocator );

	rpc::initialize						( );

	animation::memory_allocator	( m_animation_allocator );

	initialize_editor					( );
	initialize_render					( );
	initialize_network					( );
	initialize_sound					( );

	rpc::run							( rpc::sound,	boost::bind(&empty::function), rpc::break_process_loop, rpc::wait_for_completion);

	initialize_logic					( );

	rpc::wait							( rpc::editor );
	rpc::wait							( rpc::logic );

#ifndef XRAY_STATIC_LIBRARIES
//	command_line::check_keys			( );
//	command_line::handle_help_key		( );
#endif // #ifndef XRAY_STATIC_LIBRARIES

	post_initialize						( );

	enable_game							( !m_editor );

#ifdef DEBUG
	s_debug_engine_timer.start			( );
#endif // #ifdef DEBUG
}

void engine_world::tick					( )
{
	if ( threading::g_debug_single_thread )
		resources::tick					( );

	resources::dispatch_callbacks		( );

	if ( threading::g_debug_single_thread )
		network_tick					( );

	if ( threading::core_count( ) == 1 ) {
		logic_tick						( );
		m_render_world->engine().test_cooperative_level();
	}

	bool result							= false;
	if( !m_logic_frame_ended )
		result							= !m_render_world->engine().run( m_game_enabled );

	bool result_editor					= false;
	if( (m_logic_frame_ended || !m_game_enabled || m_game_enabled == m_game_paused_last) && !m_editor_frame_ended )
		result_editor					= !m_editor || !m_render_world->engine().run_editor_commands( true );
		 
	if ( result || result_editor )
		m_last_frame_time				= m_timer.get_elapsed_sec ( );

	//static int i = 0;
	//if ( (i++ % 600) != 0)
	//	return;

	//printf								( "time: %f\r\n", m_timer.get_elapsed_sec() );
	//m_engine_user_allocator.dump_statistics	( );
}

void engine_world::delete_processed_orders		( command_list_type& list, doug_lea_allocator_type& allocator, u32 const frame_id, bool const destroying )
{
	XRAY_UNREFERENCED_PARAMETER			( frame_id );

	command_type*						processed;
	while ( list.pop_front(processed) ) {
		R_ASSERT						( destroying || (processed->remove_frame_id < frame_id) );
		XRAY_DELETE_IMPL				( allocator, processed );
	}

	if ( !destroying )
		return;

	processed							= list.pop_null_node( );
	XRAY_DELETE_IMPL					( allocator, processed );
}

bool engine_world::command_line_editor	( )
{
#ifdef XRAY_STATIC_LIBRARIES
	static bool s_editor_mode			= false;
#else // #ifndef XRAY_STATIC_LIBRARIES
	static bool s_editor_mode			= s_editor_key || s_editor_st_key;
#endif // #ifndef XRAY_STATIC_LIBRARIES

	return								s_editor_mode;
}

namespace xray {
namespace testing {
	bool   run_tests_command_line ( );
} // namespace testing
} // namespace xray

bool engine_world::command_line_no_splash_screen	( )
{
	if ( xray::command_line::show_help() )
		return				true;

	if ( xray::testing::run_tests_command_line() )
		return				true;

	return					s_no_splash_screen_key.is_set( );
}

bool engine_world::command_line_editor_singlethread	( )
{
#ifndef XRAY_STATIC_LIBRARIES
	static bool s_editor_single_thread	=	s_editor_st_key && 
											threading::core_count() == 1;
	return					s_editor_single_thread;
#else // #ifndef XRAY_STATIC_LIBRARIES
	return								false;
#endif // #ifndef XRAY_STATIC_LIBRARIES
}

void engine_world::run_in_window_thread	( xray::render::engine::window_thread_callback_type const& callback )
{
	rpc::run						( rpc::editor, callback, rpc::continue_process_loop, rpc::wait_for_completion );
}

void engine_world::load_level				( pcstr project_name )
{
	if ( command_line_editor_singlethread() ) {
		engine_user_world().load			( project_name );
		return;
	}

	rpc::run							(
		rpc::logic,
		boost::bind( &xray::engine_user::world::load, &engine_user_world(), project_name ),
		rpc::continue_process_loop,
		rpc::wait_for_completion
	);
}

void engine_world::unload_level		( )
{
	if ( command_line_editor_singlethread() ) {
		engine_user_world().unload		( "" );
		return;
	}

	rpc::run							(
		rpc::logic,
		boost::bind( &xray::engine_user::world::unload, &engine_user_world(), "" ),
		rpc::continue_process_loop,
		rpc::wait_for_completion
	);
}

bool engine_world::app_is_active( )
{
	return m_app_active;
}

void engine_world::on_app_activate( )
{
	m_app_active		= true;
	if ( m_input_world )
		m_input_world->on_activate		( );

	LOG_INFO("--app Activate--");
}

void engine_world::on_app_deactivate( )
{
	m_app_active		= false;
	if ( m_input_world )
		m_input_world->on_deactivate	( );

	LOG_INFO("--app DeActivate--");
}

void engine_world::enable_game_impl		( bool const value )
{
	if ( m_engine_user_world )
	{
		if ( command_line_editor_singlethread() ) 
		{
			engine_user_world().enable		( value );
		}
		else
		{
			rpc::run							(
				rpc::logic,
				boost::bind( &xray::engine_user::world::enable, &engine_user_world(), value ),
				rpc::continue_process_loop,
				rpc::wait_for_completion
			);
		}
	}

	m_game_enabled					= value;
}