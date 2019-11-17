////////////////////////////////////////////////////////////////////////////
//	Created 	: 29.01.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef PHYSICS_WORLD_H_INCLUDED
#define PHYSICS_WORLD_H_INCLUDED

#include <xray/physics/world.h>

namespace xray {

namespace physics {
	struct engine;
	struct shell;
} // namespace physics

namespace collision {
	struct space_partitioning_tree;
	struct geometry;
	struct geometry_instance;
}//collision


} // namespace xray
class step_object;
class scheduler;
class shell_static;
class temp_storage_holder;
class physics_world :
	public xray::physics::world,
	private boost::noncopyable
{

public:
	physics_world	( xray::physics::engine& engine, xray::render::debug::renderer& renderer );
	virtual			~physics_world	( );

public:
	virtual	void							tick				( );
	virtual	void							clear_resources		( );
	virtual	non_null<physics::shell>::ptr	create_shell		( const configs::binary_config_value	&cfg );
	virtual	non_null<physics::shell>::ptr	create_shell_static	( const configs::binary_config_value	&cfg );
	virtual	non_null<physics::shell>::ptr	create_shell		( const configs::lua_config_value		&cfg );
	
	virtual	non_null<physics::shell>::ptr	create_shell_static	( non_null<collision::geometry_instance>::ptr g_instance );
	virtual					void			destroy_shell		( non_null<xray::physics::shell>::ptr  sh	 );

	virtual					void			set_ready			( bool v ){ m_b_ready = v; }
public:
							void			activate_object		( step_object	&so );
private:
			xray::physics::engine			&m_engine;
	xray::render::debug::renderer			&m_renderer;

private:	
	non_null<xray::collision::space_partitioning_tree>::ptr m_space;

private:
		scheduler							*m_sheduler;
mutable	temp_storage_holder					*m_temp_storage_holder;

public:
		temp_storage_holder&				get_temp_storage_holder				( )const { return *m_temp_storage_holder ;}

private:
	virtual	void							render								( xray::render::debug::renderer& renderer ) const ;

public:
	inline xray::render::debug::renderer&	renderer						( ) const{ return m_renderer; }
			void							add_to_created_geoms				( xray::collision::geometry* g );
			void							add_to_created_geometry_instances	( xray::collision::geometry_instance* g );
private:
	shell_static*							dbg_create_shell_static				( );
	shell_static*							m_shell_static;
	bool									m_b_ready;

	physics::vector< xray::collision::geometry* >				m_created_geoms;//for destroy
	physics::vector< xray::collision::geometry_instance* >		m_created_geometry_instances;//for destroy

}; // class world

#endif // #ifndef PHYSICS_WORLD_H_INCLUDED