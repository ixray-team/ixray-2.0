////////////////////////////////////////////////////////////////////////////
//	Created 	: 20.11.2008
//	Author		: Dmitriy Iassenev
//	Description : default entry point functions
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "physics_world.h"
#include <xray/physics/api.h>

xray::physics::allocator_type*	xray::physics::g_allocator = 0;
static XRAY_DEFAULT_ALIGN char	s_world_fake[ sizeof( physics_world ) ];
static physics_world&			s_world			= ( physics_world& )s_world_fake;
static bool						s_initialized	= false;


//xray::physics::world* xray::physics::create_world	( xray::physics::engine& engine, xray::collision::world &w)
namespace xray {
namespace physics{

xray::physics::world* create_world( xray::physics::engine& engine, xray::render::debug::renderer& renderer )
{
	R_ASSERT					( !s_initialized );
	s_initialized				= true;
	new ( &s_world ) physics_world	( engine, renderer );
	return						( &s_world );
}

} // namespace physics
} // namespace xray

void xray::physics::destroy_world( xray::physics::world*& world )
{
	R_ASSERT					( s_initialized );
	R_ASSERT					( &s_world == world );
	s_initialized				= false;
	s_world.~physics_world		( );
	world						= 0;
}

void xray::physics::memory_allocator( allocator_type& allocator )
{
	ASSERT						( !g_allocator );
	g_allocator					= &allocator;
}