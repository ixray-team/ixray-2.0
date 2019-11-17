////////////////////////////////////////////////////////////////////////////
//	Created 	: 02.02.2008
//	Author		: Konstantin Slipchenko
//	Description : collision element interface
////////////////////////////////////////////////////////////////////////////
#include "pch.h"

#include "element.h"

#include <xray/collision/geometry_instance.h>
#include <xray/collision/contact_info.h>
#include "generate_contact.h"
#include "body.h"


element::element( body &b, xray::collision::geometry_instance &cf ):
m_body( b ),
collide_element( cf )
{
	float4x4 m;
	m_pose_in_body =  cf.get_matrix() * math::invert4x3( b.get_pose(m) );
}

element::~element( )
{
	body	*b = &m_body;
	DELETE( b );
	
}

bool	element::collide( island& i, const collide_element_interface& o )const
{
	return o.collide( i, m_geometry_instance, m_body );
}

bool	element::collide( island& i, const xray::collision::geometry_instance& og, body& body )const
{
	//contact_info_buffer	buffer;

	on_two_body_contact gc(  m_body, body,  i );

	generate_contacts( gc, og );//normal og->m_geometry_instance

	return gc.number != 0;
}

bool	element::collide( island& i, const xray::collision::geometry_instance& og )const
{
	on_one_body_contact gc(  m_body, i );

	generate_contacts( gc, og );

	return gc.number != 0;
}

bool	element::integrate( const step_counter_ref& sr, float time_delta )
{
	bool is_awake = m_body.integrate( sr, time_delta );
	float4x4 m;
	m_geometry_instance.set_matrix( m_pose_in_body * m_body.get_pose( m ) );
	return is_awake;
}

void	element::fill_island( island &i )
{
	m_body.fill_island( i );
}

void element::set_mass( float mass )
{
	XRAY_UNREFERENCED_PARAMETER(mass);	
}

