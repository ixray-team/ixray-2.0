////////////////////////////////////////////////////////////////////////////
//	Created		: 23.09.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef COLLISION_STATIC_H_INCLUDED
#define COLLISION_STATIC_H_INCLUDED

#include <xray/collision/geometry.h>
#include <xray/physics/shell.h>

namespace stalker2{

class collision_static {

public:
			collision_static	();
	void	create				( xray::resources::managed_resource_ptr const mesh_resource, physics::world &w  );
	void	destroy				( physics::world &w );
private:
	xray::collision::geometry*	m_collision;
	xray::collision::object*	m_collision_object;
	xray::physics::shell*		m_statics_shell;
}; // class collision_static

} // namespace stalker2

#endif // #ifndef COLLISION_STATIC_H_INCLUDED