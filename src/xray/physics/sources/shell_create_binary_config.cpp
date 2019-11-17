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

#include "joint_ball.h"

#include <xray/configs.h>
#include <xray/collision/api.h>
#include <xray/collision/geometry_instance.h>

non_null<body>::ptr	create_body( const xray::configs::binary_config_value	&cfg, const xray::configs::binary_config_value	&root )
{
	XRAY_UNREFERENCED_PARAMETER	( root );
	body *b	= NEW( body_ode );
	//const xray::configs::binary_config_value& x=   ;
	//const xray::configs::binary_config_value& y=  ;
	//const xray::configs::binary_config_value& z=   ;
	float4x4 pose	= math::create_rotation_INCORRECT(cfg["angles"]);
	pose.c.xyz()= ( cfg["mass_center"] );
	b->set_pose( pose );

	return b;
}

non_null<xray::collision::geometry_instance>::ptr	create_sphere_cform( const xray::configs::binary_config_value	&cfg , const xray::configs::binary_config_value &root )
{
	XRAY_UNREFERENCED_PARAMETER	( root );

	float4x4 pose = float4x4().identity();//

	pose.c.xyz() = cfg["position"] ;
	return collision::create_geometry_instance( xray::physics::g_allocator, pose, collision::create_sphere_geometry( xray::physics::g_allocator, cfg["radius"] ) );
}

non_null<xray::collision::geometry_instance>::ptr	create_box_cform( const xray::configs::binary_config_value	&cfg , const xray::configs::binary_config_value &root )
{
	XRAY_UNREFERENCED_PARAMETER	( root );
	float4x4 pose =float4x4().identity(); //float4x4().identity();
	pose.c.xyz() = cfg["position"] ;
	return collision::create_geometry_instance( xray::physics::g_allocator, pose, collision::create_box_geometry( xray::physics::g_allocator, cfg["half_sides"] ) );
}

non_null<xray::collision::geometry_instance>::ptr	create_cylinder_cform( const xray::configs::binary_config_value	&cfg , const xray::configs::binary_config_value &root )
{
	XRAY_UNREFERENCED_PARAMETER	( root );
	float4x4 pose =float4x4().identity(); //float4x4().identity();
	pose.c.xyz() = cfg["position"] ;
	return collision::create_geometry_instance( xray::physics::g_allocator, pose, collision::create_cylinder_geometry( xray::physics::g_allocator, cfg["radius"], cfg["half_length"] ) );
}

non_null<xray::collision::geometry_instance>::ptr	create_cform( const xray::configs::binary_config_value	&cfg , const xray::configs::binary_config_value &root)
{
	if( strings::compare( cfg["type"], "sphere" ) == 0 )
		return create_sphere_cform( cfg,root);
	else if(strings::compare( cfg["type"], "box" ) == 0 )
		return create_box_cform( cfg,root);
	else if(strings::compare( cfg["type"], "cylinder" ) == 0 )
		return create_cylinder_cform( cfg,root);
	UNREACHABLE_CODE(return create_sphere_cform(cfg,root));
}

non_null<element>::ptr	create_element( const xray::configs::binary_config_value	&cfg, const xray::configs::binary_config_value	&root)
{
	
	non_null<body>::ptr body = create_body( root[ pcstr(cfg["body"]) ], root );

	non_null<xray::collision::geometry_instance>::ptr cf = create_cform( root[ pcstr( cfg["geometry"] ) ] , root );
	
	non_null<element>::ptr  e = NEW( element )( *body, *cf );
	
	e->set_mass( cfg["mass"] );
	return e;
}

static u32 find_index( const xray::configs::binary_config_value	&section, pcstr element )
{
	xray::configs::binary_config_value::const_iterator b = section.begin();
	xray::configs::binary_config_value::const_iterator i = section.begin();
	xray::configs::binary_config_value::const_iterator e = section.end();
	for( ; i!=e; ++i )
		if( strings::compare( pcstr(*i), element ) )
			return u32( i - b );
		
	ASSERT(false);
	return u32(-1);
}

joint* create_joint(const xray::configs::binary_config_value &cfg, const xray::configs::binary_config_value &elements, shell &sh )
{
	shell_element& e0 = sh.get_element( find_index( elements, pcstr(cfg["e0"]) ) );
	shell_element& e1 = sh.get_element( find_index( elements, pcstr(cfg["e1"]) ) );
	joint* res = NEW(joint_ball)();
	body* b0 = e0.get_body();
	body* b1 = e1.get_body();
	ASSERT( b0 );
	ASSERT( b1 );
	//res->set_anchor( cfg["position"] );
	res->connect( b0, b1 );
	res->set_anchor( cfg["position"] );
//	sh.add_joint( res );
	return res;
}

non_null<xray::physics::shell>::ptr	physics_world::create_shell	( const xray::configs::binary_config_value	&root  )
{
	
	const xray::configs::binary_config_value cfg = root["shell"];

	shell	*s =NEW( shell );
	const xray::configs::binary_config_value	&elements = root[pcstr(cfg["elements"])];
	{
	
	xray::configs::binary_config_value::const_iterator i = elements.begin();
	xray::configs::binary_config_value::const_iterator e = elements.end();
	for( ; i!=e; ++i )
		s->add_element( create_element( root[pcstr(*i)], root ) );
	}
	if( strings::compare( pcstr( cfg["joints"] ), "none" ) !=0 )
	{
		const xray::configs::binary_config_value	&joints = root[pcstr(cfg["joints"])];
		xray::configs::binary_config_value::const_iterator i = joints.begin();
		xray::configs::binary_config_value::const_iterator e = joints.end();
		for( ; i!=e; ++i )
			s->add_joint( create_joint(  root[pcstr(*i)], elements, *s ) );
	}

	s->space_insert( *m_space );
	s->wake_up( *m_sheduler );
	return s;
}


