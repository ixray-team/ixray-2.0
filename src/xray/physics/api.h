////////////////////////////////////////////////////////////////////////////
//	Created 	: 20.11.2008
//	Author		: Dmitriy Iassenev
//	Description : physics API
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_PHYSICS_API_H_INCLUDED
#define XRAY_PHYSICS_API_H_INCLUDED

#ifndef XRAY_PHYSICS_API
#	ifdef XRAY_STATIC_LIBRARIES
#		define XRAY_PHYSICS_API
#	else // #ifdef XRAY_STATIC_LIBRARIES
#		ifdef XRAY_PHYSICS_BUILDING
#			define XRAY_PHYSICS_API				XRAY_DLL_EXPORT
#		else // #ifdef XRAY_PHYSICS_BUILDING
#			ifndef XRAY_ENGINE_BUILDING
#				define XRAY_PHYSICS_API			XRAY_DLL_IMPORT
#			else // #ifndef XRAY_ENGINE_BUILDING
#				define XRAY_PHYSICS_API			XRAY_DLL_EXPORT
#			endif // #ifndef XRAY_ENGINE_BUILDING
#		endif // #ifdef XRAY_PHYSICS_BUILDING
#	endif // #ifdef XRAY_STATIC_LIBRARIES
#endif // #ifndef XRAY_PHYSICS_API

namespace xray {

namespace render {
	namespace debug {
		struct renderer;
	} // namespace debug
	struct world;
} // namespace render

namespace physics {

struct engine;
struct world;

typedef memory::doug_lea_allocator_type		allocator_type;

XRAY_PHYSICS_API	world*	create_world	( xray::physics::engine& engine, xray::render::debug::renderer& renderer );
XRAY_PHYSICS_API	void	destroy_world	( world*& world );
XRAY_PHYSICS_API	void	memory_allocator( allocator_type& allocator );

} // namespace physics
} // namespace xray

#endif // #ifndef XRAY_PHYSICS_API_H_INCLUDED