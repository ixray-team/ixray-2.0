////////////////////////////////////////////////////////////////////////////
//	Created		: 17.03.2009
//	Author		: 
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_light.h"
#include "tool_light.h"
#include "lua_config_value_editor.h"
#include <xray/render/base/editor_renderer.h>

namespace xray{
namespace editor{

object_light::object_light(tool_light^ t)
:super(t), 
m_tool_light(t)
{
	m_color			= NEW(math::color)(1.0f,1.0f,1.0f,1.0f);
	set_lib_name	("base");
	image_key		= "point_light";
}

object_light::~object_light()
{
	DELETE			(m_color);
}

void object_light::load_defaults()
{
	super::load_defaults();
	m_brightnes			= 1.0f;
	m_range				= 1.0f;
	m_b_cast_shadow		= true;
}

void object_light::load(xray::configs::lua_config_value const& t)
{
	super::load		(t);
	*m_color		= (float4)t["color"];
	m_brightnes		= t["brightness"];
	m_range			= t["range"];
	m_b_cast_shadow	= t["cast_shadow"];
	initialize_collision		();
}


void object_light::save(xray::configs::lua_config_value t)
{
	super::save		(t);
	t["color"]		= *m_color;
	t["brightness"]	= m_brightnes;
	t["range"]		= m_range;
	t["cast_shadow"]= m_b_cast_shadow;
}

void object_light::render()
{
	super::render	();

	if(m_selected)
	{
		float tmp = m_range;
		get_editor_renderer().draw_sphere(m_transform->c.xyz(), tmp, 0xffffff00);
	}
}

void object_light::initialize_collision()
{
	ASSERT( !m_collision.initialized() );
	float3 extents				(0.3f,0.3f,0.3f);
	m_collision.create_cube		(this, extents);
	m_collision.insert			();
	m_collision.set_matrix		(m_transform);
}

void object_light::set_visible(bool bvisible)
{
	ASSERT(bvisible!=get_visible());

	if(bvisible)
	{
		xray::render::light_props props;
		fill_props( props );
		get_editor_renderer().add_light		(m_id, props);
	}
	else
		get_editor_renderer().remove_light	(m_id);

	super::set_visible(bvisible);
}

void object_light::set_transform			(float4x4 const& transform)
{
	super::set_transform			(transform);

	if( get_visible())
	{
		xray::render::light_props props;
		fill_props( props );
		get_editor_renderer().update_light		(m_id, props);
	}
}

void object_light::fill_props				( xray::render::light_props& props )
{
	props.shadow_cast	= m_b_cast_shadow;
	props.range			= m_range;
	props.color			= m_color->get_d3dcolor();
	props.transform		= *m_transform;
	props.type			= xray::render::light_type_point;
}


object_dynamic_light::object_dynamic_light(tool_light^ t)
:super(t)
{
	m_texture = "";
}

object_dynamic_light::~object_dynamic_light()
{
}

void object_dynamic_light::load_defaults() 
{
	super::load_defaults();
	m_b_volumetric	= false;
	m_cone			= 45.0f;
	m_light_type	= xray::render::light_type_spot;
}

void object_dynamic_light::load(xray::configs::lua_config_value const& t)
{
	super::load		(t);
	m_light_type	= (xray::render::light_type)((u32)t["light_type"]);
	m_cone			= t["cone"];
	m_b_volumetric	= t["volumetric"];
	texture_name	= gcnew System::String(t["texture"]);
}

void object_dynamic_light::save(xray::configs::lua_config_value t)
{
	super::save		(t);
	t["light_type"]	= (u32)m_light_type;
	t["cone"]		= m_cone;
	t["volumetric"]	= m_b_volumetric;
	set_c_string(t["texture"], texture_name);
}

object_static_light::object_static_light(tool_light^ t)
:super(t)
{}

object_static_light::~object_static_light()
{}

void object_static_light::load_defaults()
{
	super::load_defaults();
	m_b_lightmap = true;
}

void object_static_light::load(xray::configs::lua_config_value const& t)
{
	super::load		(t);
	m_b_lightmap	= t["lightmap"];
}

void object_static_light::save(xray::configs::lua_config_value t)
{
	super::save		(t);
	t["lightmap"]	= m_b_lightmap;
}

void object_light::set_range_impl				(float range)
{
	m_range = range;

	if( get_visible())
	{
		xray::render::light_props props;
		fill_props( props );
		get_editor_renderer().update_light		(m_id, props);
	}
}

void object_light::set_color_impl				(xray::math::color const& color)
{
	*m_color = color;

	if( get_visible())
	{
		xray::render::light_props props;
		fill_props( props );
		get_editor_renderer().update_light		(m_id, props);
	}
}

void object_light::set_cast_shadow_impl			(bool cast_shadow)
{
	m_b_cast_shadow = cast_shadow;

	if( get_visible())
	{
		xray::render::light_props props;
		fill_props( props );
		get_editor_renderer().update_light		(m_id, props);
	}
}

} // namespace editor
} // namespace xray
