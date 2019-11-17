////////////////////////////////////////////////////////////////////////////
//	Created 	: 16.02.2008
//	Author		: Konstantin Slipchenko
//	Description : element - rigid physics bone
////////////////////////////////////////////////////////////////////////////
#ifndef XRAY_PHYSICS_STATIC_ELEMENT_H_INCLUDED
#define XRAY_PHYSICS_STATIC_ELEMENT_H_INCLUDED

#include "collide_element.h"

class static_element:
	public	collide_element
{
	
private:
	virtual	bool	collide			( island& i, const collide_element_interface& o )const				;	
	virtual	bool	collide			( island& i, const xray::collision::geometry_instance& og, body& body )const;

private:
	virtual	bool	collide			( island& i, const xray::collision::geometry_instance &og )const;
	virtual	bool	integrate		( const step_counter_ref& sr, float time_delta );

private:
	virtual	void	fill_island		( island & )			{ UNREACHABLE_CODE(); }
	virtual	body	*get_body		( )						{ return 0; }

public:
	static_element		( xray::collision::geometry_instance& cf ):	collide_element( cf )		{;}

}; // class static_element

#endif