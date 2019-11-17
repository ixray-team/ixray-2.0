////////////////////////////////////////////////////////////////////////////
//	Created		: 26.03.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "main_menu.h"
#include "game.h"
#include <xray/input/keyboard.h>
#include <xray/input/mouse.h>
#include <xray/input/gamepad.h>

namespace stalker2{

bool main_menu::on_keyboard_action( input::world& input_world, input::enum_keyboard key, input::enum_keyboard_action action )
{
	XRAY_UNREFERENCED_PARAMETER( input_world );
	if(action==xray::input::kb_key_down)
	{
		if(key == xray::input::key_escape)
		{
			m_game.main_menu_close_query();
			return					true;
		}

		if(key == xray::input::key_q)
		{
			m_game.exit			( "quit" );
			return				true;
		}
	}
	return dialog_input_handler()->on_keyboard_action( input_world, key, action );
}

bool main_menu::on_gamepad_action( input::world& input_world, input::gamepad_button button, input::enum_gamepad_action action )
{
	return dialog_input_handler()->on_gamepad_action( input_world, button, action );
}

bool main_menu::on_mouse_key_action( input::world& input_world, input::mouse_button button, input::enum_mouse_key_action action )
{
	return dialog_input_handler()->on_mouse_key_action( input_world, button, action );
}

bool main_menu::on_mouse_move( input::world& input_world, int x, int y, int z )
{
	return dialog_input_handler()->on_mouse_move( input_world, x, y, z );
}

} // namespace stalker2

