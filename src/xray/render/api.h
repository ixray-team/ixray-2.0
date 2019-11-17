////////////////////////////////////////////////////////////////////////////
//	Created 	: 28.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RENDER_API_H_INCLUDED
#define XRAY_RENDER_API_H_INCLUDED

#ifndef XRAY_RENDER_API
#	ifdef XRAY_STATIC_LIBRARIES
#		define XRAY_RENDER_API
#	else // #ifdef XRAY_STATIC_LIBRARIES
#		ifdef XRAY_RENDER_BUILDING
#			define XRAY_RENDER_API				XRAY_DLL_EXPORT
#		else // #ifdef XRAY_RENDER_BUILDING
#			ifndef XRAY_ENGINE_BUILDING
#				define XRAY_RENDER_API			XRAY_DLL_IMPORT
#			else // #ifndef XRAY_ENGINE_BUILDING
#				define XRAY_RENDER_API			XRAY_DLL_EXPORT
#			endif // #ifndef XRAY_ENGINE_BUILDING
#		endif // #ifdef XRAY_RENDER_BUILDING
#	endif // #ifdef XRAY_STATIC_LIBRARIES
#endif // #ifndef XRAY_RENDER_API

#if XRAY_PLATFORM_PS3
	typedef pvoid	HWND;
#endif // #if XRAY_PLATFORM_PS3

namespace xray {
namespace render { 

struct world;

namespace engine {
	struct wrapper;
} // namespace engine

XRAY_RENDER_API	world*	create_world	( render::engine::wrapper& engine, HWND window_handle );
XRAY_RENDER_API	void	destroy_world	( world*& world );

} // namespace render
} // namespace xray

#endif // #ifndef XRAY_RENDER_API_H_INCLUDED