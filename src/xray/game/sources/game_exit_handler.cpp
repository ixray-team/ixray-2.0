////////////////////////////////////////////////////////////////////////////
//	Created		: 03.02.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "game_exit_handler.h"
#include "game.h"
#include <xray/input/world.h>
#include <xray/input/gamepad.h>
#include <xray/input/keyboard.h>
#include <xray/input/mouse.h>

using stalker2::exit_handler;

exit_handler::exit_handler	(  stalker2::game& game )
:m_game( game )
{
}

exit_handler::~exit_handler	( )
{
}

bool exit_handler::on_keyboard_action	( xray::input::world& input_world, xray::input::enum_keyboard key, xray::input::enum_keyboard_action action )
{
	if(action!=xray::input::kb_key_down)
		return false;

	if ( key == xray::input::key_f4 )
		if( input_world.get_keyboard()->is_key_down( xray::input::key_lalt ) || 
			input_world.get_keyboard()->is_key_down( xray::input::key_ralt )	) 
	{
		m_game.engine().exit	( 0 );
		return					true;
	}

	if ( key == xray::input::key_grave )
	{
		m_game.toggle_console	( );
		return					true;
	}
	
	return		false;
}

bool exit_handler::on_gamepad_action	( xray::input::world& input_world, xray::input::gamepad_button button, xray::input::enum_gamepad_action action )
{
	XRAY_UNREFERENCED_PARAMETERS		(action, button, &input_world);
	return false;
}

bool exit_handler::on_mouse_key_action	( xray::input::world& input_world, xray::input::mouse_button button, xray::input::enum_mouse_key_action action )
{
	XRAY_UNREFERENCED_PARAMETERS		(action, button, &input_world);
	return false;
}

bool exit_handler::on_mouse_move		( xray::input::world& input_world, int x, int y, int z )
{
	XRAY_UNREFERENCED_PARAMETERS		(&input_world, x, y, z);
	return false;
}
