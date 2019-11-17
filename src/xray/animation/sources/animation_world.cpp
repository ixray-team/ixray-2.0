////////////////////////////////////////////////////////////////////////////
//	Created 	: 17.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "animation_world.h"
#include "skeleton_animation_data.h"
#include "skeleton_animation.h"
#include "animation_mix.h"
#include "skeleton.h"
#include "bone_names.h"
#include "animation_group.h"
#include "animation_controller_set.h"
#include "test_animation_player.h"
#include <xray/render/base/world.h>
#include <xray/render/base/debug_renderer.h>
#include <xray/render/base/game_renderer.h>
#include <xray/resources_queries_result.h>
#include <xray/configs_lua_config.h>
#include <xray/resources_fs.h>
#include "mixer.h"
#include <xray/animation/engine.h>

#ifdef	XRAY_USE_MAYA_ANIMATION
#	include <xray/maya_animation/engine.h>
#endif

using namespace xray::math;

#ifdef	DEBUG
using xray::static_cast_checked;
#endif

namespace xray {

namespace maya_animation {
	class discrete_data;
} // namespace maya_animation

namespace animation {

command_line::key	test_animation("test_animation", "", "animation", "");


animation_world::animation_world( xray::animation::engine& engine ) :
	m_engine					( engine ),
	b_data_loaded				(false),
	b_load						(false),
	skeleton_0					( 0 ),
	skeleton_animation_1		( 0 ),
	m_animation_controller_set	( 0 )
{

#ifdef	XRAY_USE_MAYA_ANIMATION
	//teporary: skip test resources loading when created by maya plugin
	xray::maya_animation::engine *pengine = dynamic_cast<xray::maya_animation::engine*>( &engine );
	if(  pengine )
				return;
#endif

	m_resources					= NEW ( resource_cookers )( *this );
	
	if( !test_animation )
		return;

	//m_test_animation_player		= NEW ( test_animation_player );
	
	m_animation_controller_set  = NEW( animation_controller_set )( );
	//load_data( );
}

i_animation_controller_set	*animation_world::create_controller_set( configs::lua_config_value	const&  ) 
{
	return  NEW( animation_controller_set )( );
}

void	animation_world::destroy_controller_set(i_animation_controller_set* &set )
{
	DELETE( set );
}

animation_world::~animation_world( )
{
	DELETE( m_test_animation_player );
	DELETE( m_resources );
	DELETE( m_animation_controller_set );
	DELETE( skeleton_animation_1 );
	DELETE( m_animation_mix );

}

void animation_world::tick( )
{
	
	if ( m_test_animation_player )
		m_test_animation_player->tick	( );

	//test_update_animation_controller_set();

	//render	(  m_engine.get_renderer_world().game().debug() );

	if( m_test_animation_player )
		m_test_animation_player->render(  m_engine.get_renderer_world().game().debug() );
}

float time( const  xray::timing::timer	&timer	 )
{
	static float time_scale = 6;//; 30;//1;//6;//30;
	float time = timer.get_elapsed_sec()*time_scale;
	
	return time;
}

void animation_world::test_update_animation_controller_set()
{

	if( !m_animation_controller_set )
		return;
	m_animation_controller_set->test_update( time( m_timer ) );
		
	m_animation_controller_set->render( m_engine.get_renderer_world().game().debug(), time( m_timer ) );
}

i_animation_controller_set* animation_world::dbg_tmp_controller_set	( )
{ 
	return m_animation_controller_set; 
}

void	render_skeleton( xray::render::debug::renderer			&renderer,
						 animation_mix							&animation,
						 xray::timing::timer					&timer		
						)
{
		
	//if( time( timer ) > animation.max_time() )
	//{
	//	timer.start();
	//	return;
	//}
	
	animation.render( renderer, float4x4().identity(), time( timer ) );
	
}

void animation_world::render( xray::render::debug::renderer& renderer ) const	
{
	XRAY_UNREFERENCED_PARAMETER	( renderer );

	if(!b_data_loaded)
		return;

	//render_skeleton( renderer, *m_animation_mix, m_timer );

	//if( m_animation_controller_set )
	//	m_animation_controller_set->render( renderer, time( m_timer ) );
}

skeleton_animation_data *animation_world::create_skeleton_animation_data( u32 num_bones )
{
		
	skeleton_animation_data * ret = NEW(skeleton_animation_data)( );
	ret->set_num_bones( num_bones );
	return ret;
}

void	animation_world::destroy( i_skeleton_animation_data	* &p )
{
	DELETE( p );
}

void animation_world::on_anim_data_loaded( xray::resources::queries_result& resource )
{
	R_ASSERT									( !resource.is_failed() );

	ptr_skeleton_0		=  resource[0].get_unmanaged_resource();

	skeleton_0			= static_cast_checked<skeleton*>( ptr_skeleton_0.c_ptr() );
	m_animation_mix		 = NEW( animation_mix )( *skeleton_0 );
	if( resource.size()==1 )
		return;
	R_ASSERT									( resource.size()>=2 );

	float factor = 1.f/( resource.size()-1);
	float fct = factor * 0.1f;
	for( u32 i = 1; i < resource.size(); ++i )
	{
		skeleton_animation_data		*dat	= static_cast_checked<skeleton_animation_data*>	( resource[i].get_unmanaged_resource().c_ptr() );
		dat->set_skeleton( skeleton_0 );
		
		skeleton_animation *test_anim = NEW( skeleton_animation )( *dat ) ;
		animation_layer bs0( test_anim, factor +  math::pow( float(-1),  int(i) ) * fct , 0, 1.f );
		m_animation_mix->add( bs0 );
		m_test_animations.push_back ( test_anim );
	}

	skeleton_animation_data		*dat	= static_cast_checked<skeleton_animation_data*>	( resource[1].get_unmanaged_resource().c_ptr() );
	skeleton_animation_1 = NEW( skeleton_animation )( *dat );
	
	

	//const bone_names* bone_names = dat->bone_names();
	//bone_names->create_index( *skeleton_0, m_index );



	//animation_layer bs0( skeleton_animation_1, 0, 0.7f );
	//animation_layer bs1( skeleton_animation_1, 10*20, 0.3f );
	//m_animation_mix->add( bs0 );
	//m_animation_mix->add( bs1 );

	//animation_layer bs0( skeleton_animation_1, 1.f, 0 );
	//m_animation_mix->add( bs0 );


	DELETE( dat );
	
	
}



void	animation_world::load_test_animations			()
{
		resources::request	resources[]	= {
			//{ "resources/animations/skeletons/human.lua",		xray::resources::config_lua_class },
			{ "resources/animations/skeletons/human.lua",	xray::resources::skeleton_class },
			//{ "resources/animations/walk_test.lua",			xray::resources::animation_data_class },
			//{ "resources/animations/test_animations/in_place_turn_180.lua",			xray::resources::animation_data_class },

			//{ "resources/animations/test_animations/test_blend/blend_test_right_30.lua",			xray::resources::animation_data_class },
			//{ "resources/animations/test_animations/test_blend/blend_test_right_90.lua",			xray::resources::animation_data_class },

			//{ "resources/animations/test_animations/walk_01/walk_01.lua",			xray::resources::animation_data_class },
			//{ "resources/animations/test_animations/walk_01/walk_01_fast.lua",		xray::resources::animation_data_class },
			//{ "resources/animations/test_animations/walk_01/walk_01_slow.lua",		xray::resources::animation_data_class },

			//{ "resources/animations/test_animations/in_place_turn_left/in_place_turn_left_30.lua",		xray::resources::animation_data_class },
			//{ "resources/animations/test_animations/in_place_turn_left/in_place_turn_left_90.lua",		xray::resources::animation_data_class },


			//{ "resources/animations/test_animations/walk_01/walk_01.lua",				xray::resources::animation_data_class },
			//{ "resources/animations/test_animations/walk_01/walk_01_fast.lua",		xray::resources::animation_data_class },
			{ "resources/animations/test_animations/walk_01/walk_01_fast.lua",		xray::resources::animation_data_class },
			
			//{ "resources/animations/test_animations/run/run_turn_left.lua",		xray::resources::animation_data_class },


			//{ "resources/animations/test_animations/walk/walk.lua",				xray::resources::animation_data_class },
			//{ "resources/animations/test_animations/walk/walk_fast.lua",		xray::resources::animation_data_class },
			//{ "resources/animations/test_animations/walk/walk_slow.lua",		xray::resources::animation_data_class },

			////{ "resources/animations/test_animations/take/take_far.lua",					xray::resources::animation_data_class },
			//{ "resources/animations/test_animations/take/take_right.lua",				xray::resources::animation_data_class },
			////{ "resources/animations/test_animations/take/take_left.lua",				xray::resources::animation_data_class },

			////{ "resources/animations/test_animations/take/take_near.lua",				xray::resources::animation_data_class },
			////{ "resources/animations/test_animations/take/take_far.lua",					xray::resources::animation_data_class },

			//{ "resources/animations/test_animations/test_export/test_rotate.lua",					xray::resources::animation_data_class },

		};

	xray::resources::query_resources(
			resources,
			array_size(resources),
			boost::bind( &animation_world::on_anim_data_loaded, this, _1 ),
			xray::animation::g_allocator
		);
	
}








void animation_world::load_data( )
{
	ASSERT( !b_load );

	b_load = true;

	load_test_animations		();


	b_data_loaded = true;

	m_timer.start();
}


} // namespace animation
} // namespace xray 