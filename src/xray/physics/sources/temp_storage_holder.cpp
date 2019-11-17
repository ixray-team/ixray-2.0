////////////////////////////////////////////////////////////////////////////
//	Created 	: 29.01.2008
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
//	all vectors that clear after every island step
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "temp_storage_holder.h"
#include "contact_storage.h"


temp_storage_holder::temp_storage_holder():
	m_contacts					( NEW( contact_storage ) ),
	m_space_collision_results	( NEW( xray::collision::objects_type )( *physics::g_allocator ) )
{
}

temp_storage_holder::~temp_storage_holder()
{
//	DELETE( physics::g_allocator, contact_storage );
	DELETE(  m_contacts ); //?? error
	DELETE( m_space_collision_results );
}