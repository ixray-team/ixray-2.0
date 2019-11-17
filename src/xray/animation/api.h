////////////////////////////////////////////////////////////////////////////
//	Created 	: 17.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_ANIMATION_API_H_INCLUDED
#define XRAY_ANIMATION_API_H_INCLUDED

#ifndef XRAY_ANIMATION_API
#	ifdef XRAY_STATIC_LIBRARIES
#		define XRAY_ANIMATION_API
#	else // #ifdef XRAY_STATIC_LIBRARIES
#		ifdef XRAY_ANIMATION_BUILDING
#			define XRAY_ANIMATION_API				XRAY_DLL_EXPORT
#		else // #ifdef XRAY_ANIMATION_BUILDING
#			ifndef XRAY_ENGINE_BUILDING
#				define XRAY_ANIMATION_API			XRAY_DLL_IMPORT
#			else // #ifndef XRAY_ENGINE_BUILDING
#				define XRAY_ANIMATION_API			XRAY_DLL_EXPORT
#			endif // #ifndef XRAY_ENGINE_BUILDING
#		endif // #ifdef XRAY_ANIMATION_BUILDING
#	endif // #ifdef XRAY_STATIC_LIBRARIES
#endif // #ifndef XRAY_ANIMATION_API

namespace xray {
namespace animation {

struct engine;
struct world;

typedef memory::doug_lea_allocator_type		allocator_type;

XRAY_ANIMATION_API	world*	create_world	( engine& engine );
XRAY_ANIMATION_API	void	destroy_world	( world*& world );
XRAY_ANIMATION_API	void	memory_allocator( allocator_type& allocator );

} // namespace animation
} // namespace xray

#endif // #ifndef XRAY_ANIMATION_API_H_INCLUDED