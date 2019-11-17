////////////////////////////////////////////////////////////////////////////
//	Created		: 03.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "rtp_world.h"
#include <xray/rtp/engine.h>

//#include "animation_grasping_controller.h"
//#include "test_grasping_controller.h"

#include "base_controller.h"

#include <xray/render/world.h>
#include <xray/render/base/world.h>
#include <xray/render/base/debug_renderer.h>
#include <xray/render/base/game_renderer.h>

#include <xray/animation/world.h>
#include <xray/animation/i_animation_controller_set.h>

#include <xray/configs.h>

#include <xray/console_command.h>


namespace xray {
namespace rtp {

static bool dbg_test_disable = false;


static bool b_render = true;
static console_commands::cc_bool rtp_test_render( "rtp_test_render", b_render );

static bool 	b_pause = false;
static console_commands::cc_bool rtp_test_pause( "rtp_test_pause", b_pause );

static bool 	b_learn = false;
static console_commands::cc_bool rtp_test_learn( "rtp_test_learn", b_learn );

static bool 	b_walk = false;
static console_commands::cc_bool rtp_test_walk( "rtp_test_walk", b_walk );

static bool 	b_save = false;
static console_commands::cc_bool rtp_test_save( "rtp_test_save", b_save );

static bool 	b_save_training = true;	
static console_commands::cc_bool rtp_test_save_training( "rtp_test_save_training", b_save_training );

static bool 	b_load_training = true;
static console_commands::cc_bool rtp_test_load_training( "rtp_test_load_training", b_load_training );

static bool 	b_load = false;
static console_commands::cc_bool rtp_test_load( "rtp_test_load", b_load );

static float	time_factor = 1.f;
static console_commands::cc_float rtp_test_time_factor( "rtp_test_time_factor", time_factor, 0.f, 1.f );

bool b_learn_trees_frozen =false;
static console_commands::cc_bool learn_trees_frozen( "rtp_test_learn_trees_frozen", b_learn_trees_frozen );

bool b_learn_prune = true;
static console_commands::cc_bool learn_prune( "rtp_test_learn_prune", b_learn_prune );

bool b_render_learn = true;
static console_commands::cc_bool render_learn( "rtp_test_render_learn", b_render_learn );

bool b_camera_control = true;
static console_commands::cc_bool camera_control( "rtp_test_camera_control", b_camera_control );

xray::render::debug::renderer		*g_renderer				= 0;

xray::math::random32				xray::rtp::rtp_world::m_random;

//static const u32					num_moving_actions		= 5;

rtp_world::rtp_world( xray::rtp::engine& engines   ):
	m_renderer( engines.get_renderer_world().game().debug() ),
	m_animation_world( &engines.get_animation_world() ),
	m_resource_path( engines.get_resource_path() )
{
	if( dbg_test_disable )
		return;

	g_renderer = &m_renderer;

	init( );
	
}

rtp_world::~rtp_world	( )
{
	clear_controllers( );
}

void	rtp_world::clear_controllers( )
{
	vector< base_controller*>::iterator i = m_controllers.begin(), e = m_controllers.end();

	for( ; i != e  ; ++i )
		DELETE( *i );
	m_controllers.clear();
}

void	rtp_world::anim_set_loaded_callback( )
{
	ASSERT( m_animation_world );
	ASSERT( m_animation_world->dbg_tmp_controller_set( ) );	

	//animation_grasping_controller *ctrl =  ;
	
	m_controllers.push_back( rtp::create_navigation_controller( *m_animation_world ) );
	//m_controllers.push_back( rtp::create_animation_grasping_controller( *m_animation_world ) );

	if( b_load  ) //
	{
		load( );
		return;
	}

}



void	rtp_world::create_test_controllers	( )
{

	if( !m_animation_world->dbg_tmp_controller_set( ) )
		return;
	
	m_animation_world->dbg_tmp_controller_set( )->set_callback_on_loaded(

			boost::bind( &rtp_world::anim_set_loaded_callback, this )
		);
	
	
}




void	rtp_world::render( xray::render::debug::renderer& r ) const 
{
	XRAY_UNREFERENCED_PARAMETER( r );
	if( !b_render )
		return;

	vector< base_controller*>::const_iterator i = m_controllers.begin(), e = m_controllers.end();

	for( ; i != e  ; ++i )
		(*i)->render( r );
		
}

void	rtp_world::dbg_move_control( float4x4 &view_inverted,  input::world& input_world )
{
	vector< base_controller*>::const_iterator i = m_controllers.begin(), e = m_controllers.end();

	for( ; i != e  ; ++i )
		(*i)->dbg_move_control( view_inverted, input_world );
}




void	rtp_world::on_resources_loaded( resources::queries_result& data )
{

	const u32 sz = data.size();
	
	for( u32 i = 0; i<sz; ++i )
	{
		configs::lua_config_ptr settings = static_cast_resource_ptr< configs::lua_config_ptr >( data[i].get_unmanaged_resource() );
		configs::lua_config_value cfg = *settings;
		base_controller *ctrl =  create_controller( cfg["controller"]["type"], *m_animation_world );
		ASSERT( ctrl );
		ctrl->load( cfg );
		//if( b_load_training && cfg["controller"]["value_function"]["training"] )
			//ctrl->load_training_sets( cfg );
		m_controllers.push_back( ctrl ) ;

	}
	m_timer_continious.start();
}

void	rtp_world::init( )
{

	create_test_controllers( );

}

void rtp_world::load( )
{
	if( m_controllers.empty() )
		return;

	b_load = false;

	clear_controllers( );
	query_resources( );
	
}
void	rtp_world::query_resources( )
{

		resources::request	resources[]	= {
			//{ "resources/rtp/controllers/animation_grasping_0.lua",	xray::resources::config_lua_class },
			{ "resources/rtp/controllers/navigation_0.lua",	xray::resources::config_lua_class },
		};

		xray::resources::query_resources(
			resources,
			array_size(resources),
			boost::bind(&rtp_world::on_resources_loaded, this, _1),
			xray::rtp::g_allocator
		);
}

void rtp_world::save_controllers( const vector< base_controller* > &controllers, bool training )
{
	vector< base_controller*>::const_iterator b = controllers.begin(), e = controllers.end();
	vector< base_controller*>::const_iterator  i = b;
	for( ; i != e  ; ++i )
	{
		xray::configs::lua_config_ptr cfg		= xray::configs::create_lua_config();
		xray::configs::lua_config_value v_cfg	=*cfg;
		( *i )->save(v_cfg, training );
		pcstr tp = ( *i )->type();
		string16 n;
		xray::sprintf(n,"%d",u32( i - b ));
		pcstr name = 0;
		
		STR_JOINA		( name, m_resource_path, "/rtp/controllers/", tp, "_", n, ".lua"  );
		cfg->save_as	( name );
	}

}


const	float normal_frame_rate = 30.f;

static void walk_step_controllers(  vector< base_controller* > &controllers, float dt )
{
		const float time_scale =  normal_frame_rate;
		
		vector< base_controller*>::const_iterator b = controllers.begin(), e = controllers.end();
		vector< base_controller*>::const_iterator  i = b;	
		for( ; i != e  ; ++i )
				(*i)->dbg_update_walk( dt * time_scale );
}

void	rtp_world::tick( )
{
	if( dbg_test_disable )
		return;

	if( b_save )
	{
		save_controllers( m_controllers, b_save_training );
		b_save = false;
	}
	
	if( b_load )
		load();

	render( m_renderer );

	if( b_pause )
		return;

	(void)b_learn_trees_frozen;
	bool was_prune = b_learn_prune;


	if( b_learn  )
	{
		learn_init_controllers( m_controllers );
		learn_step_controllers( m_controllers );
	}
	else if( !b_learn )
		learn_stop_controllers( m_controllers );
	
	if( was_prune && !b_learn_prune )
		b_save  = true;

	float dt = m_timer.get_elapsed_sec() * time_factor;
	m_timer.start();

	if( b_walk )
		walk_step_controllers( m_controllers, dt );

	
}

} // namespace rtp
} // namespace xray



