////////////////////////////////////////////////////////////////////////////
//	Created		: 25.05.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "physics_world.h"
#include "shell.h"
#include "element.h"
#include "body_ode.h"

#include "joint_fixed.h"
#include "joint_ball.h"
#include "joint_euler_hinge.h"
#include "joint_wheel.h"
#include "joint_slider_hinge.h"

#include <xray/configs.h>
#include <xray/collision/api.h>
#include <xray/collision/geometry_instance.h>

#include "generic_joint_create_lua_config.h"

bool read_transform( const configs::lua_config_value &table, float4x4& res_transform, float3 &res_scale )
{
	float3				p			= table["position"];
	float3				r			= table["rotation"];

						res_scale	= table["scale"];
						p.z = -p.z;
	float4x4 rx=	math::create_rotation_x			( r.x );
	float4x4 ry=	math::create_rotation_y			( r.y );
	float4x4 rz=	math::create_rotation_z			( -r.z );

	float4x4			mrotation =		rx*ry*rz;
	float4x4			mposition = math::create_translation( p );

	res_transform =   mrotation * mposition;
	

	return				true;
}

non_null<body>::ptr	create_body( const xray::configs::lua_config_value	&table, const float4x4 &tramsform )
{
	body *b	= NEW( body_ode );
	R_ASSERT( b );
	//float		mass			= table["mass"];
	float3		mass_center		= table["mass_center"];
	mass_center.z = -mass_center.z;
	float4x4 pose =  math::create_translation( mass_center ) * tramsform;
	b->set_pose( pose );
	return b;
}


non_null<xray::collision::geometry>::ptr	create_sphere_geom( const xray::configs::lua_config_value	&cfg, const float3 &scale )
{
	XRAY_UNREFERENCED_PARAMETER		(cfg);
	return  collision::create_sphere_geometry( xray::physics::g_allocator, scale.x/2.f ) ;
}

non_null<xray::collision::geometry>::ptr	create_box_geom( const xray::configs::lua_config_value	&cfg, const float3 &scale )
{
	XRAY_UNREFERENCED_PARAMETER		(cfg);
	return collision::create_box_geometry( xray::physics::g_allocator, scale/2.f );
}

non_null<xray::collision::geometry>::ptr	create_cylinder_geom( const xray::configs::lua_config_value	&cfg, const float3 &scale )
{
	XRAY_UNREFERENCED_PARAMETER		(cfg);
	return  collision::create_cylinder_geometry( xray::physics::g_allocator, scale.x/2.f, scale.y/2.f );
}

non_null<xray::collision::geometry>::ptr	create_geom( const xray::configs::lua_config_value	&cfg, const float3 &scale )
{
	if( strings::compare( cfg["type"], "sphere" ) == 0 )
		return create_sphere_geom( cfg, scale ) ;
	else if(strings::compare( cfg["type"], "box" ) == 0 )
		return  create_box_geom( cfg, scale ) ;
	else if(strings::compare( cfg["type"], "cylinder" ) == 0 )
		return   create_cylinder_geom( cfg, scale ) ;
	UNREACHABLE_CODE(return create_sphere_geom(cfg, scale) );
}

non_null<xray::collision::geometry_instance>::ptr	create_cform_transformed( const xray::configs::lua_config_value	&cfg, physics_world& world, const float4x4 &transform,const float3 &scale )
{
	non_null<xray::collision::geometry>::ptr geom =create_geom( cfg, scale ) ;
	world.add_to_created_geoms( &*geom );
	return collision::create_geometry_instance( xray::physics::g_allocator, transform, geom );
}

non_null<xray::collision::geometry_instance>::ptr	create_cform( const xray::configs::lua_config_value	&cfg, physics_world& world  )
{
	
	float4x4 transform; 
	float3 scale;
	read_transform	(cfg, transform, scale );
	return create_cform_transformed( cfg, world, transform, scale );

}

non_null<xray::collision::geometry_instance>::ptr	create_cform( const xray::configs::lua_config_value	&cfg,physics_world& world, const float4x4 &ptransform )
{
	
	float4x4 ltransform; 
	float3 scale;
	read_transform	(cfg, ltransform, scale );
	float4x4 transform = ltransform * ptransform;
	return create_cform_transformed( cfg,world, transform, scale );
}

non_null<element>::ptr create_element(const configs::lua_config_value &table, physics_world &world )
{
	
	
	float4x4 transform; 
	float3 scale;
	read_transform	(table, transform, scale );


	//table["name"]		= node.name().asChar();

	non_null<body>::ptr  body = create_body( table, transform );

	const configs::lua_config_value collision_table = table["collision"] ;
	u32 num_collisions = collision_table.size();
	ASSERT( num_collisions > 0, pcstr(table["name"]) );


	//if(num_collisions==1)
	//{
	//	non_null<element>::ptr  e = NEW( element )( *body, *create_cform ( collision_table[0], transform ) );
	//	e->set_mass( table["mass"] );
	//	return e;
	//}

	vectora< collision::geometry_instance* > vec( xray::physics::g_allocator );

	for(u32 i=0; i<num_collisions; ++i)
	{
		collision::geometry_instance &gi= *create_cform ( collision_table[i], world );
		world.add_to_created_geometry_instances( &gi );
		vec.push_back( &gi );
	}
	non_null<xray::collision::geometry>::ptr g = collision::create_compound_geometry( xray::physics::g_allocator, vec );
	world.add_to_created_geoms( &*g );
	non_null<xray::collision::geometry_instance>::ptr cf = collision::create_geometry_instance( xray::physics::g_allocator, transform, g ); 
	world.add_to_created_geometry_instances( &*cf );
	non_null<element>::ptr  e = NEW( element )( *body, *cf );
	
	e->set_mass( table["mass"] );
	
	
//								= table["anim_joint"]	;

return e;

}

static u32 find_index( const xray::configs::lua_config_value	&section, pcstr element )
{
	//xray::configs::lua_config_value::const_iterator b = section.begin();
	//xray::configs::lua_config_value::const_iterator i = section.begin();
	//xray::configs::lua_config_value::const_iterator e = section.end();
	u32 num = section.size();
	for( u32 i = 0; i<num; ++i )
		if( strings::compare( pcstr(section[i]["name"] ), element ) == 0 )
			return i;
		
	ASSERT(false);
	return u32(-1);
}

joint* create_fixed_joint(const xray::configs::lua_config_value &cfg, body* b0, body* b1, const float4x4 &transform  )
{
	XRAY_UNREFERENCED_PARAMETERS	(&cfg, &transform);
	joint*	res = NEW(joint_fixed);
	res->connect( b0, b1 );
	return	res;

}

joint* create_ball_joint(const xray::configs::lua_config_value &cfg, body* b0, body* b1, const float4x4 &transform  )
{
	XRAY_UNREFERENCED_PARAMETERS	(&cfg, &transform);
	joint*	res = NEW(joint_ball)();
	res->connect( b0, b1 );
	return	res;
}

joint* create_wheel_joint(const xray::configs::lua_config_value &cfg, body* b0, body* b1, const float4x4 &transform  )
{
	XRAY_UNREFERENCED_PARAMETERS	(&cfg, &transform);
	joint*	res = NEW(joint_wheel)();
	res->connect( b0, b1 );
	return	res;
}
joint* create_slider_joint(const xray::configs::lua_config_value &cfg, body* b0, body* b1, const float4x4 &transform  )
{
	XRAY_UNREFERENCED_PARAMETERS	(&cfg, &transform);
	joint*	res =  NEW(joint_slider_hinge)();
	res->connect( b0, b1 );
	return	res;
}

joint* create_joint(const xray::configs::lua_config_value &cfg, body* b0, body* b1, const float4x4 &transform )
{
	if( strings::compare( cfg["type"], "fixed" ) == 0 )
		return create_fixed_joint( cfg, b0, b1, transform  ) ;
	else if(strings::compare( cfg["type"], "ball" ) == 0 )
		return  create_ball_joint( cfg, b0, b1, transform  ) ;
	else if(strings::compare( cfg["type"], "generic" ) == 0 )
		return  create_generic_joint( cfg, b0, b1, transform  ) ;
	else if(strings::compare( cfg["type"], "wheel" ) == 0 )
		return   create_wheel_joint( cfg, b0, b1, transform  ) ;
	else if(strings::compare( cfg["type"], "slider" ) == 0 )
		return   create_slider_joint( cfg, b0, b1, transform ) ;
	UNREACHABLE_CODE(return create_fixed_joint(cfg, b0, b1, transform) );
}

joint* create_joint( const xray::configs::lua_config_value &cfg, const xray::configs::lua_config_value &elements, shell &sh )
{
	
	float4x4 transform; 
	float3 scale;
	read_transform	(cfg, transform, scale );

	shell_element& e0 = sh.get_element( find_index( elements, pcstr( cfg["body0"] ) ) );
	shell_element& e1 = sh.get_element( find_index( elements, pcstr( cfg["body1"] ) ) );
	body* b0 = e0.get_body();
	body* b1 = e1.get_body();
	ASSERT( b0 );
	ASSERT( b1 );

	joint* res = create_joint( cfg, b0, b1, transform );

	res->set_anchor( transform.c.xyz() );
//	sh.add_joint( res );
	return res;
}

non_null<physics::shell>::ptr	physics_world::create_shell( const configs::lua_config_value	&cfg )
{
	shell	*s =NEW( shell );
	
	const configs::lua_config_value bodies_table	= cfg["bodies"];
	const configs::lua_config_value joints_table	= cfg["joints"];
	u32 bodies_counter				= bodies_table.size();

	ASSERT(bodies_counter>0,"Bad physics shell settings: no bodies!");

	for(u32 bi = 0; bi < bodies_counter; ++bi )
	{
		s->add_element( create_element( bodies_table[bi], *this ) );
	}

	u32 joints_counter				= joints_table.size();
	for(u32 ji = 0; ji < joints_counter; ++ji )
	{
		s->add_joint( create_joint( joints_table[ji], bodies_table, *s  ) );
	}
	s->space_insert( *m_space );
	s->wake_up( *m_sheduler );
	return s;
}

