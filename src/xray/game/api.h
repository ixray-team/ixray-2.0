////////////////////////////////////////////////////////////////////////////
//	Created 	: 11.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_GAME_API_H_INCLUDED
#define XRAY_GAME_API_H_INCLUDED

#include <xray/engine/engine_user.h>	// for xray::engine_user::allocator_type

#ifndef XRAY_GAME_API
#	ifdef XRAY_STATIC_LIBRARIES
#		define XRAY_GAME_API
#	else // #ifdef XRAY_STATIC_LIBRARIES
#		ifdef XRAY_GAME_BUILDING
#			define XRAY_GAME_API			XRAY_DLL_EXPORT
#		else // #ifdef XRAY_GAME_BUILDING
#			define XRAY_GAME_API			XRAY_DLL_IMPORT
#		endif // #ifdef XRAY_GAME_BUILDING
#	endif // #ifdef XRAY_STATIC_LIBRARIES
#endif // #ifndef XRAY_GAME_API


namespace xray {

namespace engine_user {
	struct engine;
	struct world;
} // namespace engine_user

namespace input {
	struct world;
} // namespace input

namespace render {
	struct world;
} // namespace render

namespace ui {
	struct world;
} // namespace ui

namespace physics {
	struct world;
} // namespace physics

namespace rtp {
	struct world;
} // namespace rtp

namespace animation {
	struct world;
} // namespace animation

namespace sound {
	struct world;
} // namespace sound

} // namespace xray

namespace stalker2 {
namespace game_module {

XRAY_GAME_API	xray::engine_user::world*	create_world			( 
												xray::engine_user::engine& engine, 
												xray::input::world& input, 
												xray::render::world& render, 
												xray::ui::world& ui,
												xray::physics::world& physics,
												xray::rtp::world& rtp,
												xray::animation::world& animation,
												xray::sound::world& sound
											);

XRAY_GAME_API	void						destroy_world			( xray::engine_user::world*& world );
XRAY_GAME_API	void						set_memory_allocator	( xray::engine_user::allocator_type& allocator );

} // namespace game_module
} // namespace stalker2

#endif // #ifndef XRAY_GAME_API_H_INCLUDED