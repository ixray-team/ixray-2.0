////////////////////////////////////////////////////////////////////////////
//	Created 	: 29.01.2008
//	Author		: Dmitriy Iassenev
//	Description : world class
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "physics_world.h"
#include "shell.h"
#include "element.h"

#include "shell_static.h"
#include "static_element.h"


#include "body_ode.h"
#include "scheduler.h"
#include "joint_ball.h"

#include <xray/configs.h>
#include <xray/collision/api.h>
#include <xray/collision/geometry_instance.h>

#include <xray/collision/space_partitioning_tree.h>
#include "temp_storage_holder.h"
#include "ode_include.h"
#include <ode/memory.h>

namespace xray {
namespace physics {

	struct shell;

} // namespace physics
} // namespace xray

void *ode_malloc(size_t size)
{
	return MALLOC( (u32)size, "ode" );	
}

void *ode_realoc( void* p, size_t /*oldsize*/, size_t newsize )
{
	return REALLOC( p, (u32)newsize,  "ode" );	
}

void ode_free( void* p, size_t /*size*/ )
{
	FREE( p );	
}


physics_world::physics_world( xray::physics::engine& engine,
								 xray::render::debug::renderer &renderer
					) :
	m_engine				( engine ),
	m_space					( collision::create_space_partitioning_tree( xray::physics::g_allocator, 2.f, 100 ) ),
	m_temp_storage_holder	( NEW(temp_storage_holder)() ),
	m_shell_static			( 0 ),
	m_renderer				( renderer ),
	m_b_ready				(false)
{
	dSetAllocHandler		( ode_malloc );
	dSetReallocHandler		( ode_realoc );
	dSetFreeHandler			( ode_free );

	m_sheduler				= NEW( scheduler )( *this );
	//m_shell_static = dbg_create_shell_static	();
//

//	dWorldSetERP(phWorld, erp(world_spring,world_damping) );
//	dWorldSetCFM(phWorld, CFM(world_spring,world_damping));

	dWorldSetERP( 0, default_erp );
	dWorldSetCFM( 0, default_cfm );
	dWorldSetGravity( 0, 0, -default_gravity, 0 );

}

physics_world::~physics_world( )
{
	DELETE( m_temp_storage_holder );
	if(m_shell_static)
	{
		xray::collision::geometry_instance* temp_gi = &m_shell_static->get_element().get_geometry	( );
		//xray::collision::geometry*			temp_g	= &(*temp_gi->get_geometry());
		//m_collision_world.destroy_geometry( temp_g );
		collision::destroy( temp_gi );
		DELETE( m_shell_static );
	}
	{
		physics::vector<xray::collision::geometry*>::iterator i = m_created_geoms.begin(), e = m_created_geoms.end();
		for( ; i!=e ; ++i )
			collision::destroy( *i );
	}
		//DELETE(*i);
	
	{
		physics::vector<xray::collision::geometry_instance*>::iterator i = m_created_geometry_instances.begin(), e = m_created_geometry_instances.end();
		for( ; i!=e ; ++i )
			collision::destroy( *i );
	}
	DELETE( m_sheduler );
	collision::destroy( &*m_space );
}

void physics_world::tick( )
{
	if( !m_b_ready )
		return;
	ASSERT( m_sheduler );
	m_sheduler->tick();
}

void physics_world::clear_resources( )
{
}

void	physics_world::add_to_created_geoms		( xray::collision::geometry* g )
{
	m_created_geoms.push_back( g );
}

void		physics_world::add_to_created_geometry_instances( xray::collision::geometry_instance* g )
{
	m_created_geometry_instances.push_back( g );
}

//pcstr const physics_shell_ltx_source =
//"		[shell]						\n"
//"		type		= dynamic		\n"
//"		elements	= elements		\n"
//"		joints		= joints		\n"
//"		[elements]					\n"	
//"		e0							\n"
//"		e1							\n"
//"		[e0]						\n"
//"		mass		= 10.f			\n"
//"		geometry	= sphere0		\n"
//"		body		= b0			\n"
//"		[e1]						\n"
//"		mass		= 10.f			\n"
//"		geometry	= sphere0		\n"
//"		body		= b0			\n"
//"		[joints]					\n"	
//"		j0							\n"
//"		[j0]						\n"
//"		type		= ball			\n"
//"		e0			= e0			\n"
//"		e1			= e1			\n"
//"		position	= 0.0,1.0,0.0	\n"
//"		[sphere0]					\n"
//"		type	= sphere			\n"
//"		position	= 0.0,1.0,0.0	\n"
//"		radius	= 1.0				\n"
//"		[b0]						\n"
//"		mass_center	= 0.0,1.0,0.0	\n"
//"";


void	physics_world::destroy_shell( non_null<xray::physics::shell>::ptr  sh )
{
	xray::physics::shell* temp = &*sh;
	DELETE( temp );
}


shell_static* physics_world::dbg_create_shell_static()
{
	float4x4 pose = float4x4().identity();
	pose.c.xyz().set(0,-1.f,0) ;
	non_null<xray::collision::geometry>::ptr geom = collision::create_sphere_geometry( xray::physics::g_allocator, 1.f );

	//non_null<xray::collision::geometry>::ptr

	shell_static* res = NEW( shell_static )( 
		 *NEW( static_element )( 
			(*collision::create_geometry_instance( xray::physics::g_allocator, pose, geom  ) )
			) 
		);
	m_created_geoms.push_back( &*geom );
	res->space_insert( *m_space );
	return res;
}

non_null<physics::shell>::ptr	physics_world::create_shell_static( non_null<collision::geometry_instance>::ptr g_instance )
{
	shell_static* res = NEW( shell_static )( 
		 *NEW( static_element )( 
			(*g_instance )
			) 
		);
	
	res->space_insert( *m_space );
	return res;
}

non_null<physics::shell>::ptr	physics_world::create_shell_static( const configs::binary_config_value	&cfg )
{
	XRAY_UNREFERENCED_PARAMETER	( cfg );
	return dbg_create_shell_static();
}

void	physics_world::render( xray::render::debug::renderer& renderer ) const 
{
	if( m_space )
		m_space->render( renderer );
}