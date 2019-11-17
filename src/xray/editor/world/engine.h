////////////////////////////////////////////////////////////////////////////
//	Created 	: 31.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game \ - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_EDITOR_ENGINE_H_INCLUDED
#define XRAY_EDITOR_ENGINE_H_INCLUDED

#include <xray/render/base/editor_renderer.h>

namespace xray {
	
class shared_string;

namespace math{
	class float4x4;	
} //namespace math

namespace collision {
	struct world;
} // namespace collision

namespace configs{
	class lua_config_value;
} // namespace configs

namespace render {
	struct world;
} // namespace render

namespace sound {
	struct world;
} // namespace sound

namespace input{
	struct handler;
} // namespace input

namespace editor {

struct XRAY_NOVTABLE engine {
	virtual	bool			on_before_editor_tick	( ) = 0;
	virtual	void			on_after_editor_tick	( ) = 0;

	virtual	void			run_renderer_commands	( ) = 0;
	virtual	void			draw_frame				( ) = 0;
	virtual	void			flush_debug_commands	( ) = 0;
	virtual	void			on_resize				( ) = 0;
	virtual	void			enable_game				( bool value ) = 0;
	virtual	void			enter_editor_mode		( ) = 0;
	virtual	void 			load_level				( pcstr project_name ) = 0;
	virtual	pcstr			get_resource_path		( ) const = 0;
	virtual	void 			unload_level			( ) = 0;

	virtual	render::world&	get_renderer_world		( ) = 0;
	virtual	sound::world&	get_sound_world			( ) = 0;

	virtual bool			app_is_active			( ) = 0;
	virtual void			on_app_activate			( ) = 0;
	virtual void			on_app_deactivate		( ) = 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( engine )
}; // class engine

} // namespace editor
} // namespace xray

#endif // #ifndef XRAY_EDITOR_ENGINE_H_INCLUDED