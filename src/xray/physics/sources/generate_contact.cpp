////////////////////////////////////////////////////////////////////////////
//	Created 	: 03.02.2008
//	Author		: Konstantin Slipchenko
//	Description : gen_contact
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "generate_contact.h"
#include "contact_joint_info.h"
#include "island.h"
#include "contact.h"

static inline bool create_contact( island& i, const xray::collision::contact_info &c, body& body )
{
	const contact_joint_info jc = { c, i.integration_step() };
	//contact contact_joint( jc );
	//contact &ref_contact_joint =i.add_contact( contact_joint );
	//newref_contact_joint contact( jc );
	contact &ref_contact_joint = i.add_contact( jc );
	ref_contact_joint.connect( &body );
	ref_contact_joint.fill_island( i );
	return true;
}

static inline bool create_contact( island& i, const xray::collision::contact_info &c, body& body_0, body& body_1 )
{
	const contact_joint_info jc = { c, i.integration_step() };
	//contact contact( jc );
	//contact.connect( &body_0, &body_1 );
	//i.add_contact( contact );
	contact &ref_contact_joint =i.add_contact( jc );
	ref_contact_joint.connect( &body_0, &body_1 );
	ref_contact_joint.fill_island( i );
	return true;
}

void on_two_body_contact::operator () ( const xray::collision::contact_info& c ) 
{
	if( create_contact( isl, c, body_0, body_1 ) )
		++number;
}

void on_one_body_contact::operator () ( const xray::collision::contact_info& c )
{
	if( create_contact( isl, c, b ) )
		++number;
}