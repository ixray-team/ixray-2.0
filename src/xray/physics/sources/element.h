////////////////////////////////////////////////////////////////////////////
//	Created 	: 02.02.2008
//	Author		: Konstantin Slipchenko
//	Description : element - rigid physics bone
////////////////////////////////////////////////////////////////////////////
#ifndef XRAY_PHYSICS_ELEMENT_H_INCLUDED
#define XRAY_PHYSICS_ELEMENT_H_INCLUDED

#include "collide_element.h"

class body;
class collide_form;
class contact_ifo;
class element:
	public	collide_element,
	private boost::noncopyable
{

private:
	body			&m_body;
	float4x4		m_pose_in_body;
	typedef			collide_element	inherited;

public:
			element( body &b, xray::collision::geometry_instance &cf  );

protected:
			~element( );

public:
	virtual	void	set_mass				( float mass );

private:

	virtual	bool	collide			( island& i, const collide_element_interface& o )const;	
	virtual	bool	collide			( island& i, const xray::collision::geometry_instance &og, body& body )const;
	virtual	bool	collide			( island& i, const xray::collision::geometry_instance &og )const;

	virtual	bool	integrate		( const step_counter_ref& sr, float time_delta );
	virtual	void	fill_island		( island &i );
	virtual	body	*get_body		(){ return &m_body; }

}; // class element

#endif