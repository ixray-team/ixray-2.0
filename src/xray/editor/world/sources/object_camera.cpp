////////////////////////////////////////////////////////////////////////////
//	Created		: 24.04.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_camera.h"
#include "tool_utils.h"
#include <xray/render/base/game_renderer.h>
#include <xray/render/base/debug_renderer.h>

using xray::editor::object_camera;
using xray::editor::tool_utils;

object_camera::object_camera(tool_utils& t)
:super(t),
m_tool_utils(t),
m_fov(90.0f),
m_aspect_ratio(1.0f)
{}

object_camera::~object_camera()
{}

void object_camera::load(xray::configs::lua_config_value const& t)
{
	super::load					(t);
	m_fov						= t["fov"];
	m_aspect_ratio				= t["aspect_ratio"];
}

void object_camera::save(xray::configs::lua_config_value const& t)
{
	super::save					(t);
	t["fov"]					= m_fov;
	t["aspect_ratio"]			= m_aspect_ratio;
}

void object_camera::initialize_property_holder()
{
	super::initialize_property_holder();
	m_property_holder->add_float("fov", "general", "field of view", 90.0f,
		m_fov,
		property_holder::property_read_write, //?
		property_holder::do_not_notify_parent_on_change,
		property_holder::no_password_char,
		property_holder::do_not_refresh_grid_on_change
		);

	m_property_holder->add_float("aspect", "general", "aspect ratio", 1.0f,
		m_aspect_ratio,
		property_holder::property_read_write, //?
		property_holder::do_not_notify_parent_on_change,
		property_holder::no_password_char,
		property_holder::do_not_refresh_grid_on_change
		);

}

void object_camera::fill_ui(tree_node^ node, u32& count)
{
	super::fill_ui				(node, count);
	node->ImageKey				= "camera";
	node->SelectedImageKey		= "camera";
}

void object_camera::render(xray::render::base::world& render, float4x4 const& transformation)
{
	super::render	(render, transformation);

	render.game().draw_visual( m_tool_utils.get_preview_model(0), transformation);

//	if(m_selected)
//		render.debug().draw_sphere(transformation.c.xyz(), m_range, 0xffffff00);
}
