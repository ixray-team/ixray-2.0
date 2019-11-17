////////////////////////////////////////////////////////////////////////////
//	Created 	: 21.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "engine_world.h"
#include "rpc.h"
#include <xray/input/world.h>
#include <xray/editor/world/world.h>
#include <xray/render/base/engine_renderer.h>
#include <xray/core/core.h>
#include <boost/bind.hpp>

#include <xray/os_preinclude.h>
#undef NOUSER
#undef NOMSG
#undef NOWINSTYLES
#undef NOWINMESSAGES
#undef NOSYSMETRICS
#undef NOSHOWWINDOW
#undef NOWINOFFSETS
#define _WIN32_WINNT				0x0500
#include <xray/os_include.h>

#include <objbase.h>				// for COINIT_MULTITHREADED

#pragma comment( lib, "delayimp.lib" )

using xray::engine::engine_world;

static pcstr s_window_id					= XRAY_ENGINE_ID " DX9 Renderer Window";
static pcstr s_window_class_id				= XRAY_ENGINE_ID " DX9 Renderer Window Class ID";

static WNDCLASSEX s_window_class;

static xray::engine::engine_world*			s_world	= 0;

static LRESULT APIENTRY message_processor	( HWND window_handle, UINT message_id, WPARAM w_param, LPARAM l_param )
{
	switch ( message_id ) {
		case WM_DESTROY: {
			PostQuitMessage	( 0 );

			return			( 0 );
		}
		case WM_ACTIVATE: {

			u16 fActive						= LOWORD(w_param);
			BOOL fMinimized					= (BOOL) HIWORD(w_param);
			bool bActive					= ((fActive!=WA_INACTIVE) && (!fMinimized)) ? true : false;

			if (bActive != s_world->app_is_active())
			{
				if (bActive)
				{
					s_world->on_app_activate( );
					while (	ShowCursor( FALSE ) >= 0 );
				}else
				{
					s_world->on_app_deactivate( );
					while (	ShowCursor( TRUE ) < 0 );
				}
			}
			//if ( (w_param == WA_ACTIVE) || (w_param == WA_CLICKACTIVE) ) {
			//	if ( !s_world->editor_world() )
			//		while (	ShowCursor( FALSE ) >= 0 );

			//	s_world->on_app_activate( );
			//	
			//	break;
			//}

			//ASSERT			( w_param == WA_INACTIVE );
			//if ( !s_world->editor_world() )
			//	while (	ShowCursor( TRUE ) < 0 );

			break;
		}
	}
	
	return					( DefWindowProc( window_handle, message_id, w_param, l_param ) );
}

static void create_window			( HWND& result )
{
	WNDCLASSEX const temp	=
	{
		sizeof( WNDCLASSEX ),
		CS_CLASSDC,
		&message_processor,
		0L,
		0L, 
		GetModuleHandle( 0 ),
		NULL,
		NULL,
		NULL,
		NULL,
		s_window_class_id,
		NULL
	};

	s_window_class			= temp;
	RegisterClassEx			( &s_window_class );

	u32 const screen_size_x	= GetSystemMetrics( SM_CXSCREEN );
	u32 const screen_size_y	= GetSystemMetrics( SM_CYSCREEN );

	DWORD const	window_style = WS_OVERLAPPED;// | WS_CAPTION;

	u32 window_size_x		= 0;
	u32 window_size_y		= 0;

	u32 const window_sizes_x []	= { 1024, 800, 640 };
	u32 const window_sizes_y []	= { 768, 600, 480 };
	for ( u32 i=0; i<xray::array_size(window_sizes_x); ++i ) {
		if ( window_sizes_x[i] < screen_size_x && 
			 window_sizes_y[i] < screen_size_y )
		{
			window_size_x	= window_sizes_x[i];
			window_size_y	= window_sizes_y[i];
			break;
		}
	}

	R_ASSERT				(window_size_x);

	RECT		window_size = { 0, 0, window_size_x, window_size_y };
	AdjustWindowRect		(&window_size, window_style, false);

	result					= 
		CreateWindow (
			s_window_class_id,
			s_window_id,
			window_style,
			( screen_size_x - window_size.right ) / 2,
			( screen_size_y - window_size.bottom ) / 2,
			window_size.right - window_size.left,
			window_size.bottom - window_size.top,
			GetDesktopWindow( ),
			0,
			s_window_class.hInstance,
			0
		);

	R_ASSERT				( result );
}

void engine_world::initialize_core	( )
{
	//if ( true )//command_line_editor( ) )
	//	CoInitializeEx		( 0, COINIT_APARTMENTTHREADED );
	//else
	//	CoInitializeEx		( 0, COINIT_MULTITHREADED );
		
	pcstr debug_thread_id	= 0;
	if ( threading::g_debug_single_thread )
		debug_thread_id		= "main";
	else if ( threading::core_count() == 1 ) {
		if ( command_line_editor() )
			debug_thread_id	= "editor + logic + render";
		else
			debug_thread_id	= "logic + render";
	}
	else
		debug_thread_id		= "render";

	if ( !command_line_editor() )
		core::initialize	( debug_thread_id, core::create_log, core::perform_debug_initialization );
	else
		core::initialize	( debug_thread_id, core::create_log, core::delay_debug_initialization );
}

void engine_world::create_render	( )
{
	s_world					= this;

	rpc::wait				( rpc::editor );

	if ( !m_editor ) {
		create_window		( m_window_handle );
		m_main_window_handle= m_window_handle;
	}

	m_render_world		= render::create_world( *this, m_window_handle );

// 	if ( !s_dx10 )
// 		m_render_world		= render_dx9::create_world( *this, m_window_handle );
// 	else
// 		m_render_world		= render::dx10::create_world( *this, m_window_handle );

	ASSERT					( m_render_world );
}

void engine_world::destroy_render	( )
{
	render::destroy_world( m_render_world );

	ShowWindow				( m_window_handle, SW_HIDE );

	ASSERT					( s_world );
	s_world					= 0;
}

void engine_world::show_window		( )
{
	if ( m_editor ) {
		SetForegroundWindow	( m_editor->main_handle( ) );
		return;
	}

	ShowWindow				( m_window_handle, SW_SHOW );
	SetForegroundWindow		( m_window_handle );
//	on_activate				( );
}

bool engine_world::process_messages( )
{
	ASSERT					( !m_editor );

	MSG						message;
	while ( PeekMessage( &message, 0, 0, 0, PM_REMOVE ) ) {
		TranslateMessage	( &message );
		DispatchMessage		( &message );

		if ( message.message == WM_QUIT )
			return			( false );
	}

	return					( true );
}

void engine_world::run				( )
{
	if ( m_early_destruction_started )
		return;

	m_timer.start			( );

	if ( m_editor && (threading::core_count( ) == 1) ) {
		m_editor->run		( );

		if ( !m_destruction_started )
			exit			( 0 );

		return;
	}

	for (;;) {
		if ( !m_editor && !process_messages() ) {
			if ( !m_destruction_started )
				exit		( 0 );

			return;
		}

		if ( m_destruction_started )
			return;

		tick				( );
	}
}

pcstr engine_world::get_resource_path		( ) const
{
	return								"../../resources";
}

pcstr engine_world::get_mounts_path		( ) const
{
	return								"../../mounts";
}

pcstr engine_world::get_underscore_G_path	( ) const
{
	return								"../../resources/scripts";
}

void engine_world::enable_game						( bool value )
{
	bool bdraw_editor	= !value;
	bool bdraw_game		= value;
	render_world().set_editor_render_mode(bdraw_editor, bdraw_game);
	enable_game_impl	( value );
}

void engine_world::post_initialize	( )
{
	if ( command_line_editor() )
		rpc::run			( rpc::editor, boost::bind( &engine_world::initialize_editor_thread_ids, this), rpc::continue_process_loop, rpc::wait_for_completion );

	rpc::run				( rpc::editor, boost::bind( &engine_world::show_window, this), rpc::break_process_loop, rpc::wait_for_completion );
}