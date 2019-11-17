////////////////////////////////////////////////////////////////////////////
//	Created 	: 24.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object.h"
#include "game_world.h"

namespace stalker2{

game_object::game_object	( game& game )
:m_game( game )
{}

game_object::~game_object	( )
{}

void game_object::load(xray::configs::lua_config_value const& t)
{
	float3 scale			= t["scale"];
	float3 rotation			= t["rotation"];
	float3 position			= t["position"];

	m_transform				= create_scale(scale) * create_rotation_INCORRECT(rotation);
	m_transform.c.xyz()		= position;
}
} // namespace stalker2