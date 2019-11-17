////////////////////////////////////////////////////////////////////////////
//	Created		: 23.04.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "rtp_world.h"
#include <xray/rtp/api.h>

using xray::rtp::rtp_world;

static xray::uninitialized_reference<rtp_world>	s_world;
xray::rtp::allocator_type*	xray::rtp::g_allocator = 0;

xray::rtp::world* xray::rtp::create_world	( engine& engine )
{
	XRAY_CONSTRUCT_REFERENCE	( s_world, rtp_world )	( engine );
	return						( &*s_world );
}

void xray::rtp::destroy_world					( xray::rtp::world*& world )
{
	R_ASSERT					( &*s_world == world );
	XRAY_DESTROY_REFERENCE		( s_world );
	world						= 0;
}

void xray::rtp::memory_allocator				( memory::doug_lea_allocator_type&  )
{
	ASSERT						( !g_allocator );
	//g_allocator					= &allocator;
	g_allocator = &memory::g_mt_allocator;
}