////////////////////////////////////////////////////////////////////////////
//	Created		: 22.12.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "engine_world.h"
#include "rpc.h"
#include <xray/sound/world.h>
#include <xray/sound/api.h>
#include <xray/render/base/engine_renderer.h>

#if XRAY_PLATFORM_WINDOWS
#	include <xray/os_preinclude.h>
#	undef NOUSER
#	undef NOMSG
#	undef NOMB
#	include <xray/os_include.h>
#	include <objbase.h>						// for COINIT_MULTITHREADED
#endif // #if XRAY_PLATFORM_WINDOWS

using xray::engine::engine_world;

void engine_world::initialize_sound_modules	( )
{
	rpc::assign_thread_id			( rpc::sound, threading::current_thread_id( ) );
	sound::memory_allocator			( m_sound_allocator );
	m_sound_allocator.user_current_thread_id();

	m_engine_user_allocator.user_current_thread_id();
	m_editor_allocator.user_current_thread_id();
	m_sound_world					= sound::create_world( *this, &m_engine_user_allocator, command_line_editor() ? &m_editor_allocator : 0 );
}

void engine_world::initialize_sound	( )
{
	if ( threading::g_debug_single_thread ) {
		initialize_sound_modules	( );
		return;
	}

	rpc::assign_thread_id			( rpc::sound, u32(-1) );
	threading::spawn				( boost::bind( &engine_world::sound, this ), 
									  "sound", 
									  "sound", 
									  0,
									  5 % threading::core_count() );
}

static void sound_dispatch_callbacks		( )
{
	xray::resources::dispatch_callbacks	( );
//	xray::logic::dispatch_callbacks		( );
}

void engine_world::sound_tick			( )
{
	sound_dispatch_callbacks		( );
	m_sound_world->tick				( m_logic_frame_id );
}

void engine_world::sound			( )
{
#if XRAY_PLATFORM_WINDOWS
	CoInitializeEx			( 0, COINIT_APARTMENTTHREADED );
//	CoInitializeEx			( 0, COINIT_MULTITHREADED );
#endif // #if XRAY_PLATFORM_WINDOWS

	initialize_sound_modules		( );
	rpc::process					( rpc::sound );

	for (;;) {
		if ( m_destruction_started )
			break;

#if 0
		sound_tick					( );
#else
		threading::yield			( 10 );
#endif
		while ( ( m_logic_frame_id > m_render_world->engine().frame_id( ) + 1 ) && !m_destruction_started ) {
			if ( !rpc::try_process_single_call( rpc::sound ) )
				threading::yield	( 1 );
		}

	}
	
	rpc::process					( rpc::sound );
}

void engine_world::sound_clear_resources	( )
{
	resources::dispatch_callbacks	( );
	m_sound_world->clear_resources	( );
}
