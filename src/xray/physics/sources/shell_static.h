////////////////////////////////////////////////////////////////////////////
//	Created 	: 16.02.2008
//	Author		: Konstantin Slipchenko
//	Description : shell static
////////////////////////////////////////////////////////////////////////////
#ifndef XRAY_PHYSICS_SHELL_STATIC_H_INCLUDED
#define XRAY_PHYSICS_SHELL_STATIC_H_INCLUDED
#include "space_object.h"
#include <xray/physics/shell.h>

class shell_element;
class collide_element_interface;
class static_element;

class shell_static:
	public	physics::shell,
	public space_object
{
	
	shell_element			*m_element;

public:
							shell_static		( static_element &e );
							~shell_static		( );
public:
	virtual	void			collide				( island& i,  space_object& o )						;
	virtual	bool			collide_detail		( island& i,  space_object& o )						;
	virtual	bool			collide				( island& i,  collide_element_interface& e ) 		;

public:
	inline	shell_element	&get_element		( )	{ return *m_element; }

private:
	virtual	void			aabb				( float3 &center, float3 &radius )const	;
	virtual	bool			aabb_test			( math::aabb const& aabb ) const;
	virtual	void			space_check_validity( )const;
	virtual	void			render				( render::debug::renderer& renderer )const;

}; // class shell_static
#endif