////////////////////////////////////////////////////////////////////////////
//	Created		: 03.02.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ui_world.h"
#include "ui_dialog.h"
#include "ui_image.h"
#include "ui_text.h"
#include "ui_text_edit.h"
#include "ui_scroll_view.h"

window* ui_world::create_window()
{
	return NEW(ui_window)();
}

dialog* ui_world::create_dialog()
{
	return NEW(ui_dialog)(*this);
}

text* ui_world::create_text()
{
	return NEW(ui_text)(*this);
}

text_edit* ui_world::create_text_edit()
{
	return NEW(ui_text_edit)(*this, te_standart);
}

image* ui_world::create_image()
{
	return NEW(ui_image)();
}

scroll_view* ui_world::create_scroll_view()
{
	return NEW(ui_scroll_view)();
}

void ui_world::destroy_window(window* w)
{
	DELETE(w);
}
