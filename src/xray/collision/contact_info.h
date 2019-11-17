///////////////////////////////////////////////////////////////////////////
//	Created 	: 05.02.2008
//	Author		: Konstantin Slipchenko
//	Description : contact info
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_COLLISION_CONTACT_INFO_H_INCLUDED
#define XRAY_COLLISION_CONTACT_INFO_H_INCLUDED

namespace xray {
namespace render {
namespace debug {
	struct renderer;
} // namespace debug
} // namespace render

namespace collision {

struct contact_info
{
	float3	position;
	float3	normal;
	float	depth;
	u32		material_id;
}; // struct contact_info

struct XRAY_NOVTABLE on_contact
{
	virtual void						operator( ) ( const contact_info& c )	= 0;
	virtual	memory::base_allocator*		alocator( )								= 0;
#ifdef	DEBUG
	virtual render::debug::renderer&	renderer( )								= 0;
#endif // #ifdef DEBUG

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( on_contact )
}; // struct on_contact

} // namespace collision
} // namespace xray

#endif // #ifndef XRAY_COLLISION_CONTACT_INFO_H_INCLUDED