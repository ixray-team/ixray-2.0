////////////////////////////////////////////////////////////////////////////
//	Created		: 12.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_light.h"
#include "game_world.h"
#include "game.h"
#include <xray/render/base/game_renderer.h>
#include <xray/render/base/world.h>

namespace stalker2{

object_light::object_light( game& game )
:super			( game ),
m_color			( 0xffffffff ),
m_brightnes		( 1.f ),
m_range			( 1.f ),
m_b_cast_shadow	( false )
{}

object_light::~object_light( )
{
	m_game.render_world().game().remove_light		( (u32)this );
}

void object_light::load(xray::configs::lua_config_value const& t)
{
	super::load		(t);
	m_color			= (float4)t["color"];
	m_brightnes		= t["brightness"];
	m_range			= t["range"];
	m_b_cast_shadow	= t["cast_shadow"];

	xray::render::light_props props;
	props.shadow_cast	= m_b_cast_shadow;
	props.range			= m_range;
	props.color			= m_color.get_d3dcolor();
	props.transform		= m_transform;
	props.type			= xray::render::light_type_point;

	m_game.render_world().game().add_light		( (u32)this, props );
}


} // namespace stalker2