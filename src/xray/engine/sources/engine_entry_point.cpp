////////////////////////////////////////////////////////////////////////////
//	Created 	: 06.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/engine/api.h>
#include "engine_world.h"
#include <xray/linkage_helper.h>

#ifndef XRAY_STATIC_LIBRARIES
	DECLARE_LINKAGE_ID(engine_entry_point)
#endif // #ifndef XRAY_STATIC_LIBRARIES

typedef xray::memory::doug_lea_allocator_type	doug_lea_allocator_type;
doug_lea_allocator_type							xray::engine::g_allocator;

static xray::uninitialized_reference<xray::engine::engine_world>	s_world;

void xray::engine::preinitialize				( pcstr command_line, pcstr application, pcstr build_date )
{
	XRAY_CONSTRUCT_REFERENCE		( s_world, engine_world )	( command_line, application, build_date );
}

void xray::engine::initialize		( xray::engine_user::module_proxy* module_proxy )
{
	s_world->setup_engine_user		( module_proxy );
	s_world->initialize				( );
}

void xray::engine::finalize						( )
{
	XRAY_DESTROY_REFERENCE			( s_world );
}

void xray::engine::execute						( )
{
	s_world->run					( );
}

bool xray::engine::command_line_editor			( )
{
	return							s_world->command_line_editor( );
}

bool xray::engine::command_line_no_splash_screen( )
{
	return							s_world->command_line_no_splash_screen( );
}

int xray::engine::get_exit_code					( )
{
	return s_world->get_exit_code	( );
}
