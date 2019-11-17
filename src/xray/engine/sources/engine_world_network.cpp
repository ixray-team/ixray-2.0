////////////////////////////////////////////////////////////////////////////
//	Created		: 09.11.2009
//	Author		: 
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "engine_world.h"
#include "rpc.h"
#include <xray/network/world.h>
#include <xray/network/api.h>
#include <xray/render/base/engine_renderer.h>

using xray::engine::engine_world;

void engine_world::initialize_network_modules	( )
{
	rpc::assign_thread_id			( rpc::network, threading::current_thread_id( ) );
	m_network_allocator.user_current_thread_id	( );
	network::memory_allocator		( m_network_allocator );
	m_network_world					= network::create_world( *this );
}

void engine_world::initialize_network	( )
{
	if ( threading::g_debug_single_thread ) {
		initialize_network_modules	( );
		return;
	}

	rpc::assign_thread_id			( rpc::network, u32(-1) );
	threading::spawn				( boost::bind( &engine_world::network, this ), 
									  "network", 
									  "network", 
									  0,
									  3 % threading::core_count() );
}

void engine_world::network_tick			( )
{
	resources::dispatch_callbacks	( );
	m_network_world->tick			( m_logic_frame_id );
}

void engine_world::network				( )
{
	initialize_network_modules		( );

	for (;;) {
		if ( m_destruction_started )
			break;

//		network_tick				( );

		while ( ( m_logic_frame_id > m_render_world->engine().frame_id( ) + 1 ) && !m_destruction_started ) {
			if ( !rpc::try_process_single_call( rpc::network ) )
				threading::yield	( 1 );
		}
		threading::yield	( 10 );
	}
	
	rpc::process					( rpc::network );
}

void engine_world::network_clear_resources	( )
{
	resources::dispatch_callbacks	( );
	m_network_world->clear_resources( );
}