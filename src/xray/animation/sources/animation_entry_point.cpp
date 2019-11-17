////////////////////////////////////////////////////////////////////////////
//	Created 	: 17.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "animation_world.h"
#include <xray/animation/api.h>
#include <xray/linkage_helper.h>

#ifndef XRAY_STATIC_LIBRARIES
	DECLARE_LINKAGE_ID(animation_entry_point)
#endif // #ifndef XRAY_STATIC_LIBRARIES

using xray::animation::animation_world;

static xray::uninitialized_reference< animation_world >	s_world;
xray::animation::allocator_type*	xray::animation::g_allocator = 0;

xray::animation::world* xray::animation::create_world( engine& engine )
{
	XRAY_CONSTRUCT_REFERENCE	( s_world, animation_world ) ( engine );
	return						( &*s_world );
}

void xray::animation::destroy_world( xray::animation::world*& world )
{
	R_ASSERT					( &*s_world == world );
	XRAY_DESTROY_REFERENCE		( s_world );
	world						= 0;
}

void xray::animation::memory_allocator( allocator_type& allocator )
{
	ASSERT						( !g_allocator );
	g_allocator					= &allocator;
}