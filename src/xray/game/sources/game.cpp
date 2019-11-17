////////////////////////////////////////////////////////////////////////////
//	Created 	: 11.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "game.h"
#include "game_exit_handler.h"
#include "game_world.h"
#include "main_menu.h"
#include "game_resource_manager.h"
#include "console.h"
#include "stats.h"
#include "stats_graph.h"
#include "project_cooker.h"
#include "cell_cooker.h"
#include "object_cooker.h"

#include <xray/render/world.h>
#include <xray/render/base/common_types.h>
#include <xray/render/base/debug_renderer.h>
#include <xray/render/base/engine_renderer.h>
#include <xray/render/base/game_renderer.h>
#include <xray/render/base/world.h>
#include <xray/input/world.h>
#include <xray/input/api.h>
#include <xray/ui/world.h>
#include <xray/sound/world.h>
#include <xray/collision/geometry.h>
#include <xray/console_command.h>
#include <xray/console_command_processor.h>
#include <xray/physics/shell.h>
#include <xray/physics/world.h>
#include <xray/animation/world.h>
#include <xray/rtp/world.h>
#include <xray/configs.h>

using xray::console_commands::cc_delegate;
using xray::console_commands::cc_float3;

static bool s_draw_stats_value = true;
static console_commands::cc_bool s_draw_stats("draw_stats", s_draw_stats_value);

static xray::physics::shell*	s_dbg_shell_1	= 0;
static command_line::key		s_level_key("level", "", "", "level to load");

#ifndef MASTER_GOLD
static void	make_crash				( pcstr )
{
	FATAL					( "do not report, this console command is aimed for testing bug tracking" );
}
static console_commands::cc_delegate s_crash("crash", &make_crash, false);
#endif // #ifndef MASTER_GOLD


#if 0
static void on_lua_config_loaded	( resources::queries_result& data )
{
	XRAY_UNREFERENCED_PARAMETER		( data );
	R_ASSERT						( !data.is_failed() );

	{
		configs::lua_config_ptr	config	= configs::create_lua_config("test.lua");
		configs::lua_config_value root	= (*config)["skeleton"];
		root["hips"]["spine"]["spine_1"]["spine_2"]["neck"]["head"] = 1;
		root["hips"]["spine"]["spine_1"]["spine_2"]["neck"]["head"].create_table();
		root["hips"]["spine"]["spine_1"]["spine_2"]["neck"]["head2"] = 1;
		root["hips"]["spine"]["spine_1"]["spine_2"]["neck"].add_super_table( root["hips"] );
		root["hips"]["spine"]["spine_1"]["spine_2"]["neck"].remove_super_table( root["hips"] );
		root["hips"]["spine"]["spine_1"]["spine_2"]["neck"].add_super_table( "scripts/test_config", "new_table.mega_table" );
		configs::lua_config_value value = root["hips"]["spine"]["spine_1"]["spine_2"]["neck"]["mega_value"];
		pcstr const mega_value			= value;
		debug::printf( "%s", mega_value );
//		root["hips"]["spine"]["spine_1"]["spine_2"]["neck"].remove_super_table( "test", "hips.spine"  );
//		root["hips"]["spine"]["spine_1"]["spine_2"]["neck"]["head"] = "ooops";
		root["hips"]["spine"]["spine_1"]["spine_2"]["left_collar"]["left_arm"]["left_fore_arm"]["left_hand_roll"].create_table();
		config->save					( );
	}
	{
		configs::lua_config_ptr config_ptr = static_cast_checked<configs::lua_config*>(data[0].get_unmanaged_resource().get());
		configs::lua_config& config		= *config_ptr;
		u32 const value					= config["atest_table"]["value"];
		u32 const value2				= config["atest_table"]["value2"];
		config.save_as					( "d:/test.lua" );
	}

	{
		configs::lua_config_ptr config	= configs::create_lua_config();
		configs::lua_config_value params = *config;
		params["test"][0]["a"] 			= "a";
		params["test"][0]["b"] 			= "b";
		params["test"][1]["a"] 			= "a";
		params["test"][1]["b"] 			= "b";
		config->save_as					( "d:/test.lua" );
		configs::create_lua_config( config->get_binary_config() )->save_as("d:/test2.lua");
	}

	configs::lua_config_ptr config_ptr = static_cast_checked<configs::lua_config*>(data[0].get_unmanaged_resource().get());
	configs::lua_config& config		= *config_ptr;
	u32 u32_value					= config["test_table"]["u32_value"];
	s32 s32_value					= config["test_table"]["s32_value"];
	float float_value				= config["test_table"]["float_value"];
	pcstr string_value				= config["test_table"]["string_value"];
	math::float2 const float2_value	= config["test_table"]["float2_value"];
	math::float3 float3_value		= config["test_table"]["float3_value"];
	bool bool_value					= config["test_table"]["one_more_table"]["bool_value"];
	bool const value_exist0			= config["test_table"]["one_more_table"].value_exists("bool_value");
	bool const value_exist1			= config["test_table"]["one_more_table"].value_exists("bool_value_");
	bool const value_exist2			= config.value_exists("test_table.one_more_table.bool_value");

	config["test_table"]["u32_value"]		= 128;
	config["test_table"]["s32_value"]		= -128;
	config["test_table"]["float_value"]		= 128.1f;
	config["test_table"]["string_value"]	= "string_value";
	config["test_table"]["float2_value"]	= math::float2( 0.f, 1.f );
	config["test_table"]["float3_value"]	= math::float3( 0.f, 1.f, 2.f ).normalize();
	config["test_table"]["new_table"]["float3_value"]	= math::float3( 0.f, 128.f, 2.f ).normalize();
	config["new_table"]["new_table"]["float3_value"]	= math::float3( 2.f, 128.f, 2.f ).normalize();
	config["test_table"]["float3_value"]["new_value"]	= math::float3( 0.f, 1.f, 2.f ).normalize();

	config["test_table"]["new_table2"]["float3_value"].value_exists("float4_value");
	config["new_table2"]["new_table"]["float3_value"].value_exists("float4_value");

	config["expreimaental"]["test_super_value"]		= "super_value";

	config["test_table"]["__a"].remove_super_table	( "scripts/test_config", "test_table" );
	config["test_table"]["__a"].remove_super_table	( "scripts/test_config", "new_table" );
	config["test_table"]["__a"].add_super_table		( "scripts/test_config", "expreimaental" );
	pcstr const super_value			= config["test_table"]["__a"]["test_super_value"];
	config["test_table"]["__a"]["test_super_value"]	= "super_value";
//	config["test_table"]["__a"].add_super_table	( "scripts/test_config", "test_table" );
//	float float_value2				= config["test_table"]["__a"]["float_value"];
//	config["test_table"]["__a"].remove_super_table	( "scripts/test_config", "test_table" );
//	float_value2					= config["test_table"]["__a"]["float_value"];
//	config["test_table"]["__a"].add_super_table	( config[new_table] );
//	float3_value					= config["test_table"]["__a"]["new_table"]["float3_value"];
//	config["test_table"]["new_table"].remove_super_table( config["new_table"] );
//	float3_value					= config["test_table"]["new_table"]["new_table"]["float3_value"];

	configs::lua_config_value value	= config.get_root( );
	value							= value["test_table"];
	value							= value["float2_value"];
	value							= value["new_value"];
	value							= math::float2( 3, 1 );

	config.save_as					( "c:\\test_config.lua" );
//	config_ptr						= configs::create_lua_config( );
//	configs::lua_config& config		= *config_ptr;
//	config["test_table"]["u32_value"]		= 1;
//	config["test_table"]["string_value"]	= "aga!";
//	u32 const u32_value				= (*bc)["test_table"]["u32_value"];
//	pcstr const string_value		= (*bc)["test_table"]["string_value"];

	{
		configs::binary_config_ptr bc	= config.get_binary_config( );
		u32_value						= (*bc)["test_table"]["u32_value"];
		s32_value						= (*bc)["test_table"]["s32_value"];
		float_value						= (*bc)["test_table"]["float_value"];
		string_value					= (*bc)["test_table"]["string_value"];
		bool_value						= (*bc)["test_table"]["one_more_table"]["bool_value"];
		math::float3 const float3_value	= (*bc)["test_table"]["float3_value"]["new_value"];
	}
}
#endif // #if 0

namespace stalker2 {

void register_cookers( bool beditor_present );
void unregister_cookers( );

game::game(
		xray::engine_user::engine& engine,
		xray::input::world& input,
		xray::render::world& render,
		xray::ui::world& ui,
		xray::physics::world& physics,
		xray::rtp::world& rtp,
		xray::animation::world& animation,
		xray::sound::world& sound
	) :
	m_engine				( engine ),
	m_input_world			( input ),
	m_render_world			( render ),
	m_ui_world				( ui ),
	m_physics_world			( physics ),
	m_animation_world		( animation ),
	m_rtp_world				( rtp ),
	m_sound_world			( sound ),
	m_last_frame_time		( 0 ),
	m_active_scene			( NULL ),
	m_game_world			( NULL ),
	m_main_menu				( NULL ),
	m_enabled				( false ),
	m_project_path			( "" )
{
	register_cookers		( );
	m_timer.start			( );

	m_game_world			= NEW(game_world)( *this );
 	m_main_menu				= NEW(main_menu)( *this );
	m_console				= NEW(console)( *this );
	m_stats					= NEW(stats)( ui );
	m_fps_graph				= NEW(stats_graph)( 1.f, math::infinity, 30.f, 60.f );

	XRAY_UNREFERENCED_PARAMETER	( rtp );

	set_view_matrix(math::create_camera_direction(
						float3( 16.f, 10.f, 0.f ),
						float3( -1.f,  -1.f, 0.f ).normalize( ),
						float3( 0.f,  1.f, 0.f ))
					);

	m_projection				= math::create_projection( math::pi_d4, 1/(4.f/3.f), .2f, 5000.0f );

	
	register_console_commands	( );

	fixed_string512				project_path;
	bool const load_level		= s_level_key.is_set_as_string(&project_path);

	if( load_level && !m_engine.command_line_editor() )
	{
		load				( project_path.c_str() );
	}
}

game::~game				( )
{
	console_commands::save		( "startup.cfg" );

	if( m_active_scene )
		m_active_scene->on_deactivate ( );

	DELETE						( m_game_world );
	DELETE						( m_main_menu );
	DELETE						( m_console );
	DELETE						( m_stats );
	DELETE						( m_fps_graph );
	unregister_cookers			( );
}

fs::path_string const& game::project_path( ) const
{
	return m_project_path;
}

void game::enable( bool value )
{
	m_enabled = value;

	if ( value )
	{
		m_input_world.acquire		( );

		// select active scene
		scene* scene_to_activate = m_game_world;
#if 0		
		if(m_game_world->empty())
			scene_to_activate		= m_main_menu;
#endif
		switch_to_scene				( scene_to_activate );
	}else
	{
		m_input_world.unacquire		( );
	}


}

void on_config_loaded( resources::queries_result& data )
{
	if( data.is_failed( ) )
	{
		LOG_ERROR("config file loading FAILED");
		return;
	}

	resources::pinned_ptr_const<u8> pinned_data	(data[ 0 ].get_managed_resource( ));
	memory::reader				F( pinned_data.c_ptr( ), pinned_data.size( ) );
	console_commands::load		( F );
}

void load_config_query		( pcstr cfg_name )
{
	resources::request	resources[]	= { cfg_name,	xray::resources::raw_data_class };

	xray::resources::query_resources	(
		resources,
		array_size(resources),
		boost::bind(&on_config_loaded, _1),
		g_allocator
	);
}

void game::register_console_commands	( )
{
	static cc_float3 cc_cam_pos	("cam_pos", m_inverted_view.c.xyz(), float3(-1000,-1000,-1000), float3(1000,1000,1000));
	cc_cam_pos.serializable		(false);

	static exit_handler		game_exit_handler	( *this );
	m_input_world.add_handler	( game_exit_handler );

	static cc_delegate		game_exit_cc( "quit", cc_delegate::Delegate(this, &game::exit), false );
	static cc_delegate		cfg_load_cc( "cfg_load", cc_delegate::Delegate(&load_config_query), true );
	cfg_load_cc.execute		("resources/startup.cfg");

 	static cc_delegate		cfg_load_level( "level_load",   cc_delegate::Delegate(this, &game::load), true );
 	static cc_delegate		cfg_unload_level( "level_unload", cc_delegate::Delegate(this, &game::unload), false );
}

void game::switch_to_scene( scene* scene )
{
	ASSERT			( scene );
//	ASSERT			( m_active_scene != scene );

	if( m_active_scene )
		m_active_scene->on_deactivate ( );
	
	m_active_scene	= scene;
	m_active_scene->on_activate ( );
}

u32 game::time_ms			( )
{
	return					( m_timer.get_elapsed_ms( ) );
}

void game::set_view_matrix( float4x4 const& view_matrix)
{
	m_inverted_view = math::invert4x3( view_matrix );
}

void game::set_inverted_view_matrix( float4x4 const& inverted_view )
{
	m_inverted_view							= inverted_view;
	m_sound_world.set_listener_properties	( m_inverted_view, g_allocator );
}

float4x4 const&	game::get_inverted_view_matrix( ) const
{
	return m_inverted_view;
}

void game::toggle_console			( )
{
	if ( m_console->get_active() )
		m_console->on_deactivate	( );
	else
		m_console->on_activate		( );
}

void game::exit( pcstr str )
{
	unload						( str );
	engine().exit				( 0 );
}

void game::tick					( u32 const current_frame_id )
{
	if(!m_enabled)
	{
	
		LOG_WARNING("game::tick raised, but m_enabled==false. fix it");
		return;
	}
	if(m_active_scene)
	{
		m_input_world.tick				( );
		m_active_scene->tick			( );	
	}

	test								( );

	if( m_console->get_active() )
		m_console->tick					( );
	
	update_stats						( current_frame_id );

	m_render_world.engine().set_view_matrix			( math::invert4x3( m_inverted_view ) );
	m_render_world.engine().set_projection_matrix	( m_projection );

	m_ui_world.tick				( );
	m_physics_world.tick		( );

	m_rtp_world.tick			( );

	m_animation_world.tick		( );
}

void game::update_stats( u32 const current_frame_id )
{
	float const last_frame_time			= m_engine.get_last_frame_time();
	m_fps_graph->add_value				( last_frame_time, math::is_zero(last_frame_time - m_last_frame_time) ? math::infinity : 1.f/(last_frame_time - m_last_frame_time) );
	m_last_frame_time					= last_frame_time;

	if ( s_draw_stats_value ) {
		m_stats->set_fps_stats			( m_fps_graph->average_value() );
		m_stats->set_camera_stats		( m_inverted_view.c.xyz(), m_inverted_view.k.xyz() );
		m_stats->draw					( m_render_world );
	}

	static bool draw_stats_graph		= false;
	static console_commands::cc_bool	fps_graph( "draw_fps_graph", draw_stats_graph );
	fps_graph.serializable				(false);

	if ( draw_stats_graph ) {
		m_fps_graph->set_time_interval	( 5.f );
		m_fps_graph->render				( m_render_world, current_frame_id, 10, 768 - 128 - 2, 1024 - 2*10, 128 );
	}
	else {
		m_fps_graph->stop_rendering		( );
		m_fps_graph->set_time_interval	( 1.f );
	}
}

void game::clear_resources( )
{
	if( !m_game_world->empty( ) )
		m_game_world->unload	( );

}

void game::load( pcstr path )
{
	m_project_path			= path;
	m_game_world->load		( path );
	switch_to_scene			( m_game_world );
}

void game::unload( pcstr )
{
	m_project_path			= "";
	ASSERT					( m_game_world );

	m_game_world->unload	( );
	switch_to_scene			( m_main_menu );
}

void game::main_menu_close_query	( )
{
	switch_to_scene		( m_game_world );
}

void game::game_world_close_query			( )
{
	switch_to_scene		( m_main_menu );
}

void game::register_cookers( )
{
	m_project_cooker		= NEW( project_cooker )( engine().command_line_editor() );
	m_cell_cooker			= NEW( cell_cooker )( );
	m_object_cooker			= NEW( object_cooker )( *this );
	m_solid_visual_cooker	= NEW( solid_visual_cooker )();
	register_cook			( m_project_cooker );
	register_cook			( m_cell_cooker );
	register_cook			( m_object_cooker );
	register_cook			( m_solid_visual_cooker );
}

void game::unregister_cookers( )
{
	DELETE					( m_project_cooker );
	DELETE					( m_cell_cooker );
	DELETE					( m_object_cooker );
	DELETE					( m_solid_visual_cooker );
}

} // namespace stalker2
