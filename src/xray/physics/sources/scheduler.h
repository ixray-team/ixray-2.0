////////////////////////////////////////////////////////////////////////////
//	Created 	: 04.02.2008
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#ifndef PHYSICS_SHEDULER_H_INCLUDED
#define PHYSICS_SHEDULER_H_INCLUDED

#include "step_counter.h"

class	physics_world;
class	step_object;

class scheduler:
	private boost::noncopyable
{
	step_counter						m_step_counter;
	const physics_world					&m_world;
	physics::vector<step_object*>		m_active_objects;
public:
								scheduler	( const physics_world	&w	);
		void					activate	( step_object *o );
		void					tick		( );
inline	const physics_world&	world		( )const	{return m_world ;}
inline	const step_counter_ref	step_counter( )const	{ return step_counter_ref( m_step_counter ); }
private:
	void	step		( float time_delta );

}; // class scheduler

#endif