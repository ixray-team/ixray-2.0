////////////////////////////////////////////////////////////////////////////
//	Created 	: 04.02.2008
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#include "pch.h"

#include "scheduler.h"
#include "step_object.h"
#include "physics_world.h"
#include "temp_storage_holder.h"
#include "contact_storage.h"
scheduler::scheduler( const physics_world	&w	):
m_world(w)
{
	
}
u32 dbg_count  = 0;
void	scheduler::tick( )
{
	//...
	// get delta and devide into fixed
	//...
	++dbg_count;
	const float time_delta	= 0.01f;
	const u32	sub_steps_num	= 1;//50;	
	if( dbg_count%1 == 0 )
		for( u32 i = 0; i < sub_steps_num ; ++i )	
			step( time_delta );
	//::render(  world().get_temp_storage_holder().contacts(), world().render_world() );
}

void	scheduler::activate	( step_object *o )
{
	ASSERT( o );
	m_active_objects.push_back( o );
}

void	scheduler::step( float time_delta )
{
	m_step_counter.step();
	physics::vector<step_object*>::reverse_iterator			ri = m_active_objects.rbegin();
	physics::vector<step_object*>::reverse_iterator			re = m_active_objects.rend();
	
	for( ;ri != re; ++ri )
	{
		
		if( !(*ri)->step( *this, time_delta ) )
		{
			physics::vector<step_object*>::iterator		ei = m_active_objects.end() - 1;
			std::swap( *ei, *ri );
			m_active_objects.erase( ei );
		}

	}

}