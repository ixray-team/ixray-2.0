////////////////////////////////////////////////////////////////////////////
//	Created		: 30.01.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ui_world.h"
#include "ui_text_edit.h"
#include <xray/render/world.h>

//static ui::dialog* d			= NULL;

ui_world::ui_world(engine& engine, xray::input::world& input, xray::render::world& render)
:	m_engine			(engine),
	m_input				(input),
	m_render			(render),
	m_base_screen_size	(1024.0f, 768.0f)
{
//	m_dialog_holder		= NEW (ui_dialog_holder)( *this );
	m_timer.start	();
}

ui_world::~ui_world()
{
//	DELETE (m_dialog_holder);
}

void ui_world::tick()
{
//	m_dialog_holder->update			();
//	m_dialog_holder->draw			(m_render);
}

void ui_world::clear_resources		( )
{
}

void ui_world::on_device_reset()
{
	m_font_manager.on_device_reset();
}
