////////////////////////////////////////////////////////////////////////////
//	Created		: 19.03.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_ENGINE_ENGINE_USER_H_INCLUDED
#define XRAY_ENGINE_ENGINE_USER_H_INCLUDED

namespace xray {

namespace editor {
	struct world;
} // namespace editor

namespace animation {
	struct world;
} // namespace animation

namespace input {
	struct world;
} // namespace input

namespace render {
	struct world;

	namespace engine {
		struct command;
	} // namespace engine
} // namespace render

namespace network {
	struct engine;
	struct world;
} // namespace game

namespace sound {
	struct engine;
	struct world;
} // namespace game

namespace ui {
	struct world;
} // namespace ui

namespace physics {
	struct world;
} // namespace physics

namespace rtp {
	struct world;
} // namespace physics


namespace engine_user {

typedef memory::doug_lea_allocator_type	allocator_type;

struct XRAY_NOVTABLE engine {
	virtual	float	get_last_frame_time	( ) = 0;
	virtual	void	exit				( int exit_code ) = 0;
	virtual	void	enter_editor_mode	( ) = 0;
	virtual	bool	command_line_editor	( ) = 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( engine )
}; // class engine

struct XRAY_NOVTABLE world {
	virtual	void	enable				( bool value ) = 0;
	virtual	void	tick				( u32 current_frame_id ) = 0;
	virtual	void	clear_resources		( ) = 0;
	virtual	void 	load				( pcstr project_name ) = 0;
	virtual	void 	unload				( pcstr ) = 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( world )
}; // class world

struct XRAY_NOVTABLE module_proxy {
	virtual	world*	create_world		(
						xray::engine_user::engine& engine, 
						xray::input::world& input, 
						xray::render::world& render, 
						xray::ui::world& ui,
						xray::physics::world& physics,
						xray::rtp::world& rtp,
						xray::animation::world& animation,
						xray::sound::world& sound
					) = 0;
	virtual	void	destroy_world		( world*& world ) = 0;
	virtual	void	set_memory_allocator( allocator_type& allocator ) = 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( module_proxy )
}; // class module_proxy

} // namespace engine_user
} // namespace xray

#endif // #ifndef XRAY_ENGINE_ENGINE_USER_H_INCLUDED