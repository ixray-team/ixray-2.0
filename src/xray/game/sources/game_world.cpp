////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "game_world.h"
#include "game.h"
#include "input_handler_free_fly.h"
#include "cell_manager.h"

#include <xray/input/world.h>
#include <xray/input/keyboard.h>
#include <xray/rtp/world.h>

namespace stalker2{

game_world::game_world( game& g ):
super			( g )
{
	m_cell_manager		= NEW (cell_manager)( *this );
	m_free_fly_camera	= NEW(free_fly_handler)( g );
}

game_world::~game_world()
{
	DELETE		( m_free_fly_camera );
	DELETE		( m_cell_manager );
}

void game_world::on_activate( )
{
	m_game.input_world().add_handler	( *m_free_fly_camera );
	super::on_activate					( );
}

void game_world::on_deactivate( )
{
	m_game.input_world().remove_handler	( *m_free_fly_camera );
	super::on_deactivate				( );
}


void	game_world::rtp_debug_update		( )
{
	float4x4 view_inverted	= m_game.get_inverted_view_matrix( );
	m_game.rtp().dbg_move_control( view_inverted, m_game.input_world() );
	m_game.set_inverted_view_matrix	( view_inverted );
}

void game_world::tick( )
{
	m_free_fly_camera->tick				( m_game.input_world() );
	

	rtp_debug_update		( );

	m_cell_manager->set_inv_view_matrix	( m_game.get_inverted_view_matrix() );
	m_cell_manager->tick				( );
}

void game_world::unload( )
{
	m_cell_manager->unload	( );
	ASSERT					( empty() );
}

bool game_world::empty( )
{
	return		m_cell_manager->empty();
}

void game_world::load( pcstr project_name )
{
	m_project_name		= project_name;

	string_path request_path;
	pcstr xprj_ext = ".xprj";
	
	if(strings::ends_with(project_name, strings::length(project_name), xprj_ext, strings::length(xprj_ext)) )
	{
		strings::copy_n( request_path, sizeof(request_path), project_name, strings::length(project_name)-strings::length(xprj_ext));
		strings::append( request_path, ".prj");
	}else
	{ // pure project name
		strings::join( request_path, "resources.converted/projects/", project_name, "/project.prj" );
	}

	xray::resources::query_resource(
		request_path,
		xray::resources::game_project_class,
		boost::bind(&game_world::on_project_files_loaded, this, _1),
		g_allocator
	);
}

void game_world::on_project_files_loaded( xray::resources::queries_result& data )
{
	R_ASSERT						( data.is_successful() );

	game_project_ptr game_project	= static_cast_resource_ptr<game_project_ptr>(data[0].get_unmanaged_resource());

	m_cell_manager->load		( game_project );

	math::float3 camera_position	= (*game_project->m_config)["camera"]["position"];
	math::float3 camera_direction	= (*game_project->m_config)["camera"]["direction"];

	float4x4 view_inverted	= get_game().get_inverted_view_matrix( );

	view_inverted.c.xyz()			= camera_position;
	view_inverted.k.xyz()			= camera_direction;

	get_game().set_inverted_view_matrix	( view_inverted );
}


} // namespace stalker2