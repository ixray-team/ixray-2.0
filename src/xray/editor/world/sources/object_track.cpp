////////////////////////////////////////////////////////////////////////////
//	Created		: 27.04.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_track.h"
#include "tool_utils.h"
#include <xray/render/base/game_renderer.h>
#include <xray/render/base/debug_renderer.h>

using xray::editor::object_track;
using xray::editor::tool_utils;

object_track::object_track(tool_utils& t)
:super(t),
m_tool_utils(t)
{}

object_track::~object_track()
{}

void object_track::load(xray::configs::lua_config_value const& t)
{
	super::load			(t);
}

void object_track::save(xray::configs::lua_config_value const& t)
{
	super::save			(t);
}

void object_track::fill_ui(tree_node^ node, u32& count)
{
	super::fill_ui		(node, count);
}

void object_track::render(xray::render::base::world& world, float4x4 const& transformation)
{
	super::render		(world, transformation);
}

void object_track::initialize_property_holder()
{
	super::initialize_property_holder();
}