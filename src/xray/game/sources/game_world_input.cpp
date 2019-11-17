////////////////////////////////////////////////////////////////////////////
//	Created		: 26.03.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "game_world.h"
#include "game.h"
#include <xray/input/world.h>
#include <xray/input/keyboard.h>

namespace stalker2{

bool game_world::on_keyboard_action( input::world& input_world, input::enum_keyboard key, input::enum_keyboard_action action )
{
	XRAY_UNREFERENCED_PARAMETER	( input_world );

	if(key == xray::input::key_escape && action==xray::input::kb_key_down)
	{
		m_game.game_world_close_query	( );
		return					true;
	}

	return false;
}

} // namespace stalker2