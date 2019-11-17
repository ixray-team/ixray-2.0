////////////////////////////////////////////////////////////////////////////
//	Created 	: 28.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RENDER_WORLD_H_INCLUDED
#define XRAY_RENDER_WORLD_H_INCLUDED

namespace xray {
namespace render {

namespace engine	{	struct renderer; struct command;	}
namespace game		{	struct renderer;					}
namespace ui		{	struct renderer;					}
namespace editor	{	struct renderer;					}

struct XRAY_NOVTABLE world {
	virtual	void				clear_resources	( ) = 0;

	virtual	engine::renderer&	engine			( ) = 0;
	virtual	game::renderer&		game			( ) = 0;
	virtual	ui::renderer&		ui				( ) = 0;
	virtual	editor::renderer&	editor			( ) = 0;

	virtual	void				push_command	( engine::command* command ) = 0;
	virtual	void				destroy_command	( engine::command* command ) = 0;
	virtual	void			set_editor_render_mode	( bool beditor, bool bgame ) = 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( world )
}; // class world

} // namespace render
} // namespace xray

#endif // #ifndef XRAY_RENDER_WORLD_H_INCLUDED