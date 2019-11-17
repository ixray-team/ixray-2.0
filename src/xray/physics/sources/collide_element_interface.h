////////////////////////////////////////////////////////////////////////////
//	Created 	: 02.02.2008
//	Author		: Konstantin Slipchenko
//	Description : collision element interface
////////////////////////////////////////////////////////////////////////////
#ifndef XRAY_PHYSICS_COLLIDE_ELEMENT_INTERFACE_H_INCLUDED
#define XRAY_PHYSICS_COLLIDE_ELEMENT_INTERFACE_H_INCLUDED

namespace xray {
namespace render {
	enum debug_user_id;
} // namespace render
} // namespace xray

namespace xray {
namespace collision {
	struct geometry_instance;
} // namespace collision

namespace render {
	struct world;
} // namespace render

} // namespace xray

class island;
class body;

class collide_element_interface {

public:
	virtual	math::aabb	&aabb				( math::aabb &box ) const														= 0;
	virtual	bool		collide				( island& i, const collide_element_interface& o )const							= 0;	
	virtual	bool		collide				( island& i, const xray::collision::geometry_instance &og, body& body )const	= 0;
	virtual	bool		collide				( island& i, const xray::collision::geometry_instance &og )const				= 0;
	virtual	bool		aabb_test			( math::aabb const& aabb )const													= 0;
	virtual void check_collision_validity	( )const																		= 0;
	virtual	void		render				( render::debug::renderer& renderer ) const										= 0;
	
public:
	virtual	xray::collision::geometry_instance &get_geometry( )																= 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( collide_element_interface )
};
#endif