////////////////////////////////////////////////////////////////////////////
//	Created		: 03.02.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#ifndef XRAY_UI_WORLD_H_INCLUDED
#define XRAY_UI_WORLD_H_INCLUDED

#include <xray/ui/ui.h>

namespace xray {
namespace ui {

struct XRAY_NOVTABLE world {
	virtual	void				tick				( )					= 0;
	virtual	void				clear_resources		( )					= 0;
	virtual ui::window*			create_window		( )					= 0;
	virtual ui::dialog*			create_dialog		( )					= 0;
	virtual ui::text*			create_text			( )					= 0;
	virtual ui::text_edit*		create_text_edit	( )					= 0;
	virtual ui::image*			create_image		( )					= 0;
	virtual ui::scroll_view*	create_scroll_view	( )					= 0;
	virtual void				destroy_window		( ui::window* w )	= 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( world )
}; // class world

} // namespace ui
} // namespace xray

#endif // #ifndef XRAY_UI_WORLD_H_INCLUDED