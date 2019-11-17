////////////////////////////////////////////////////////////////////////////
//	Created 	: 04.02.2008
//	Author		: Konstantin Slipchenko
//	Description : shell
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "shell.h"

#include "joint.h"
#include "shell_element.h"
#include "scheduler.h"
#include "island.h"
shell::shell( )

{
	

}

shell::~shell( )
{
	{
	physics::vector<joint*>::iterator		i = m_joints.begin();
	physics::vector<joint*>::const_iterator	e = m_joints.end();
	for( ;i!= e; ++i )
				DELETE(*i);
	}
	{
	physics::vector<shell_element*>::iterator		i = m_elements.begin();
	physics::vector<shell_element*>::const_iterator	e = m_elements.end();
	for( ;i!= e; ++i )
		DELETE(*i);
	}
}

void	shell::island_step( island &i, float time_delta )
{
	dynamic_object::island_step( i, time_delta );
	force_integrate( time_delta );
	position_integrate( i, time_delta );
}

void	shell::force_integrate( float time_delta )
{
	physics::vector<joint*>::iterator		i = m_joints.begin();
	physics::vector<joint*>::const_iterator	e = m_joints.end();
	for( ;i!= e; ++i )
		(*i)->integrate( time_delta );
}

void	shell::position_integrate( island &isl, float time_delta )
{
	
	physics::vector<shell_element*>::iterator		i = m_elements.begin();
	physics::vector<shell_element*>::const_iterator	e = m_elements.end();
	bool b_wake_up = false;
	for( ;i!= e; ++i )
		b_wake_up = (*i)->integrate( isl.step_counter(), time_delta )||b_wake_up;

	if( !awake() && b_wake_up )
		wake_up( isl.get_sheduler() );

	if(!b_wake_up)
		sleep_internal();

	
}

bool	shell::collide( island& isl,  collide_element_interface& ce )
{
	bool result = false;	
	physics::vector<shell_element*>::iterator		i = m_elements.begin();
	physics::vector<shell_element*>::const_iterator	e = m_elements.end();
	for( ;i!= e; ++i )
		result = ce.collide( isl, *(*i) ) || result;
	return result;

}

bool shell::collide_detail( island& isl,  space_object& o )
{
	bool result = false;
	physics::vector<shell_element*>::iterator		i = m_elements.begin();
	physics::vector<shell_element*>::const_iterator	e = m_elements.end();
	for( ;i!= e; ++i )
		result = o.collide( isl, *(*i) ) || result;
	return result;

}

void shell::on_island_connect		( island	&isl )
{
	{
		physics::vector<joint*>::iterator		i = m_joints.begin();
		physics::vector<joint*>::const_iterator	e = m_joints.end();
		for( ;i!= e; ++i )
			(*i)->fill_island( isl );
	}
	{
		physics::vector<shell_element*>::iterator		i = m_elements.begin();
		physics::vector<shell_element*>::const_iterator	e = m_elements.end();
		for( ;i!= e; ++i )
			(*i)->fill_island( isl );
	}
}


void	shell::render( xray::render::debug::renderer& renderer )const
{
	{
		physics::vector<shell_element*>::const_iterator		i = m_elements.begin();
		physics::vector<shell_element*>::const_iterator	e = m_elements.end();
		for( ;i!= e; ++i )
			(*i)->render( renderer );
	}
	{
		physics::vector<joint*>::const_iterator	i = m_joints.begin();
		physics::vector<joint*>::const_iterator	e = m_joints.end();
		for( ;i!= e; ++i )
			(*i)->render( renderer );
	}
}

void	shell::wake_up( scheduler &s )
{
	dynamic_object::wake_up( s );
}

math::aabb&	shell::aabb				( math::aabb& box )const
{
	
	box.invalidate();
	physics::vector<shell_element*>::const_iterator		i = m_elements.begin();
	physics::vector<shell_element*>::const_iterator		e = m_elements.end();
	for( ;i!= e; ++i )
	{
		math::aabb tmp;
		box.modify(
			(*i)->aabb( tmp ) 
		);
	}
	
	return box;
}

void	shell::aabb				( float3 &center, float3 &radius )const
{
	math::aabb box;
	aabb( box );
	center = box.center( radius );
}

bool	shell::aabb_test( math::aabb const& _aabb ) const
{
	math::aabb box;
	if( !aabb( box ).intersect( _aabb ) )
		return false;
	physics::vector<shell_element*>::const_iterator		i = m_elements.begin();
	physics::vector<shell_element*>::const_iterator		e = m_elements.end();
	for( ;i!= e; ++i )
		if( (*i)->aabb_test( _aabb ) ) 
			return true;
	return false;
}

void	shell::space_insert	( xray::collision::space_partitioning_tree &space )
{ 
	dynamic_object::space_insert( space ); 
}

void shell::space_check_validity( )const
{
	ASSERT(!m_elements.empty(),"physics shell space_check_validity fail: no elemnts in shell");
	physics::vector<shell_element*>::const_iterator		i = m_elements.begin();
	physics::vector<shell_element*>::const_iterator		e = m_elements.end();
	for( ;i!= e; ++i )
		(*i)->check_collision_validity() ;
}