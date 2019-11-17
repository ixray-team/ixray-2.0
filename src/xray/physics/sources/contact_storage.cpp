////////////////////////////////////////////////////////////////////////////
//	Created 	: 24.02.2008
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
//	contact storage
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "contact_storage.h"
#include "contact.h"
//contact&	add					( const contact_joint_info& c );
contact& contact_storage::add( const contact_joint_info& c )
{
	m_contacts.push_back( NEW(contact)(c) );
	return *m_contacts.back();
}

void contact_storage::clear	()
{
	physics::vector<contact*>::iterator i = m_contacts.begin();
	physics::vector<contact*>::const_iterator e = m_contacts.end();
	for( ; i!=e; ++i )
		DELETE(*i);
	m_contacts.clear();
}

void		contact_storage::for_each			( contact_predicate &predicate )
{
	physics::vector<contact*>::iterator i = m_contacts.begin();
	physics::vector<contact*>::const_iterator e = m_contacts.end();
	for( ; i!=e; ++i )
		predicate(*(*i));
}


void render( contact_storage &contacts, render::debug::renderer& renderer )
{
	struct pred:
		public contact_predicate,
		private boost::noncopyable
	{
		render::debug::renderer& renderer;
		pred( render::debug::renderer& renderer ):renderer( renderer ) {}
		virtual bool operator() ( contact& c )
		{
			c.render( renderer );
			return true;
		}
	} p( renderer );
	contacts.for_each( p );
}