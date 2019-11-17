////////////////////////////////////////////////////////////////////////////
//	Created		: 30.01.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#ifndef UI_SCROLL_PAD_H_INCLUDED
#define UI_SCROLL_PAD_H_INCLUDED
#include "ui_window.h"

class ui_scroll_pad : public ui_window
{
	typedef ui_window	super;
public:
	virtual void	draw					(xray::render::world& w);
	virtual void	tick					();
};
#endif //UI_SCROLL_PAD_H_INCLUDED