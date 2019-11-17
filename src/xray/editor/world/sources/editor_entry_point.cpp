////////////////////////////////////////////////////////////////////////////
//	Created 	: 31.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <new>
#include "editor_world.h"

#pragma unmanaged
#include <xray/editor/world/api.h>
#pragma managed

#pragma unmanaged
static void enable_fpe( bool const value );
#pragma managed

using xray::editor::editor_world;

static bool					s_initialized = false;
static char					s_world_fake[ sizeof( editor_world ) ];
static editor_world&		s_world = ( editor_world& )s_world_fake;

xray::editor::allocator_type*	xray::editor::g_allocator = 0;

static xray::editor::world* create_world		( xray::editor::engine& engine, bool fpe_enabled )
{
#ifdef DEBUG
	enable_fpe				( fpe_enabled );
#else // #ifdef DEBUG
	XRAY_UNREFERENCED_PARAMETER	( fpe_enabled );
#endif // #ifdef DEBUG

	R_ASSERT				( !s_initialized );
	s_initialized			= true;

	typedef xray::editor::editor_world	world;
	new ( &s_world ) world	( engine );
	return					( &s_world );
}

static void destroy_world						( xray::editor::world*& world )
{
	R_ASSERT				( s_initialized );
	R_ASSERT				( &s_world == world );
	s_initialized			= false;
	s_world.~editor_world	( );
	world					= 0;
}

static void install_bugtrap						( )
{
	// this is workaround on incorrect BugTrapN initialization in
	// case of call from unmanaged code: globals and statics are not initialized.
	// but when we call it from managed code first, all works properly
	IntelleSoft::BugTrap::ExceptionHandler();
}

#pragma unmanaged

#ifdef DEBUG
static void enable_fpe							( bool const value )
{
	xray::debug::enable_fpe	( value );
}
#endif // #ifdef DEBUG

xray::editor::world* xray::editor::create_world	( engine& engine )
{
#ifdef DEBUG
	bool const fpe_enabled	= xray::debug::is_fpe_enabled( );
	xray::debug::enable_fpe	( false );
#else // #ifdef DEBUG
	bool const fpe_enabled	= false;
#endif // #ifdef DEBUG
	return					::create_world( engine, fpe_enabled );
}

void xray::editor::destroy_world				( xray::editor::world*& world )
{
	::destroy_world			( world );
}

void xray::editor::memory_allocator				( allocator_type& allocator )
{
	install_bugtrap			( );

	ASSERT					( !g_allocator );
	g_allocator				= &allocator;
	allocator.user_current_thread_id();
}
#pragma managed