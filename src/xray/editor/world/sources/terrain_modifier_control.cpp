////////////////////////////////////////////////////////////////////////////
//	Created		: 14.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_modifier_control.h"
#include "project.h"
#include "collision_object_dynamic.h"
#include "terrain_object.h"
#include "terrain_modifier_command.h"
#include "terrain_control_cursor.h"
#include "level_editor.h"
#include "tool_terrain.h"
#include "picker.h"

namespace xray {
namespace editor {

using xray::math::float4x4;

terrain_modifier_control_base::terrain_modifier_control_base(xray::editor::level_editor^ le, tool_terrain^ tool)
:super			(le->ide()),
m_level_editor	(le),
m_last_screen_xy(-10000, -10000),
m_center_position_global(NEW (float3)(0,0,0)),
m_apply_command(nullptr),
m_visible(false),
m_cursor(nullptr),
m_tool_terrain(tool)
{
	shape_type		= modifier_shape::circle;
	radius			= 5.0f; //1..50
	hardness		= 0.5f; //0..1
	strength		= 0.5f;
}

terrain_modifier_control_base::~terrain_modifier_control_base()
{
	DELETE			(m_center_position_global);
	delete			(m_cursor);
}

u32 terrain_modifier_control_base::acceptable_collision_type()
{
	return collision_type_terrain;
}

void terrain_modifier_control_base::initialize()
{}

void terrain_modifier_control_base::destroy()
{}

void terrain_modifier_control_base::show(bool show)
{
	m_visible = show;
}

void terrain_modifier_control_base::activate(bool b_activate)
{
	super::activate(b_activate);

	if(m_cursor==nullptr)
		m_cursor		= gcnew terrain_control_cursor(this);
	
	m_cursor->show		(b_activate);
}

void terrain_modifier_control_base::update()
{
	object_base^ o = m_level_editor->get_project()->aim_object();
	if(o && !m_visible)
		show(true);
	else
	if(!o && m_visible)
		show(false);

	ASSERT(m_cursor);
	m_cursor->update();

	if ( !m_visible )
		return;
}

void terrain_modifier_control_base::pin(bool bpin)
{
	m_cursor->pin(bpin);
}

void terrain_modifier_control_base::start_input()
{
	super::start_input				();

	ASSERT							(m_apply_command == nullptr);
	create_command					();
	m_level_editor->get_command_engine()->run_preview( m_apply_command );
}

void terrain_modifier_control_base::end_input()
{
	super::end_input				();

	m_level_editor->get_command_engine()->end_preview();
	m_apply_command					= nullptr;

	for each (terrain_node_key key in m_processed_id_list.Keys)
	{
		terrain_node^ curr_terrain			= m_tool_terrain->get_terrain_core()->m_nodes[key];
		curr_terrain->initialize_collision	();
	}

	m_processed_id_list.Clear		();
	m_active_id_list.Clear			();
}

void terrain_modifier_control_base::draw(float4x4  const& /*view_matrix*/)
{}

void terrain_modifier_control_base::set_draw_geomtry(bool /*draw*/)
{}

void terrain_modifier_control_base::resize(float /*size*/)
{}

void terrain_modifier_control_base::set_mode_modfiers(bool /*plane_mode*/, bool /*free_mode*/)
{}

float3 terrain_modifier_control_base::get_picked_point()
{
	return *m_cursor->m_last_picked_position;
}

// 0..1
float terrain_modifier_control_base::calc_influence_factor(float const & distance)
{
	float hardness_bound	= radius * hardness;
	float res				= 0.0f;

	if(distance<hardness_bound)
	{
		res			= 1.0f;
	}else
	{
		float t		= 1.0f - (radius-distance) / (radius-hardness_bound);
		res			= (1+2*t)*(1-t)*(1-t);
		ASSERT			(res>=0.0f && res<=1.0f);
	}
	return			res;
}


bool temp_foo ( xray::collision::ray_triangle_result const& )
{	return true;}

void terrain_modifier_control_base::refresh_active_working_set(modifier_shape t)
{
	m_active_id_list.Clear				();

	collision_object const* picked_collision = NULL;
	
	if(m_level_editor->get_picker( ).ray_query( acceptable_collision_type(), &picked_collision, true, m_center_position_global ) )
	{
		ASSERT( picked_collision->get_type() & acceptable_collision_type() );
	
		m_tool_terrain->get_terrain_core()->select_vertices(*m_center_position_global, radius, t, %m_active_id_list);

		for each (terrain_node_key key in m_active_id_list.Keys)
			if(!m_processed_id_list.ContainsKey(key))
				m_processed_id_list.Add			(key, gcnew vert_id_list);

	}

	on_working_set_changed				();
}

xray::math::int2 terrain_modifier_control_base::get_mouse_position()
{
	xray::math::int2					result;
	m_level_editor->ide()->get_mouse_position(result);
	return result;
}

void terrain_modifier_control_base::load_settings(RegistryKey^ key)
{
	super::load_settings		(key);
	RegistryKey^ self_key		= get_sub_key(key, name);

	shape_type					= (modifier_shape)(System::Enum::Parse(modifier_shape::typeid, self_key->GetValue("shape_type", modifier_shape::circle)->ToString()));
	radius						= convert_to_single(self_key->GetValue("radius", "3.0"));
	hardness					= convert_to_single(self_key->GetValue("hardness", "0.5"));
	distortion					= convert_to_single(self_key->GetValue("distortion", "0.5"));
	strength					= convert_to_single(self_key->GetValue("strength",	"0.5"));

	self_key->Close				();
}

void terrain_modifier_control_base::save_settings(RegistryKey^ key)
{
	super::save_settings		(key);
	RegistryKey^ self_key		= get_sub_key(key, name);

	self_key->SetValue			("shape_type", shape_type);
	self_key->SetValue			("radius", convert_to_string(radius) );
	self_key->SetValue			("hardness", convert_to_string(hardness) );
	self_key->SetValue			("distortion", convert_to_string(distortion) );
	self_key->SetValue			("strength", convert_to_string(strength) );

	self_key->Close				();
}

void terrain_modifier_control_base::change_property(System::String^ prop_name, float const value)
{
	if(prop_name=="radius")
	{
		radius			+= value;
		radius			= math::clamp_r(radius, 0.5f, 50.0f);
	}else
	if(prop_name=="hardness")
	{
		hardness		+= value;
		hardness		= math::clamp_r(hardness, 0.0f, 1.0f);
	}else
	if(prop_name=="strength")
	{
		strength		+= value;
		strength		= math::clamp_r(strength, m_min_strength, m_max_strength);
	}

	super::change_property(prop_name, value);
}

float terrain_modifier_control_base::strength_value()
{
	return ((strength-m_min_strength) / (m_max_strength-m_min_strength)) * 5.0f;
}

terrain_modifier_height::terrain_modifier_height(xray::editor::level_editor^ le, tool_terrain^ tool)
:super(le, tool)
{}

terrain_modifier_height::~terrain_modifier_height()
{}

void terrain_modifier_height::end_input()
{
	for each (terrain_node_key key in m_processed_id_list.Keys)
	{
		height_diff_dict^ curr_processed_values_list	= m_processed_values_list[key];
		height_diff_dict^ curr_stored_values_list		= m_stored_values_list[key];
		terrain_node^ curr_terrain						= m_tool_terrain->get_terrain_core()->m_nodes[key];

		for each (u16 vert_idx in m_processed_id_list[key])
		{
			float old_h							= curr_stored_values_list[vert_idx];
			float new_h							= curr_terrain->m_vertices[vert_idx].height;
			curr_processed_values_list[vert_idx] = new_h - old_h;
		}

		(safe_cast<terrain_modifier_height_command^>(m_apply_command))->set_data(key, m_processed_values_list[key]);
	}

	super::end_input				();
	m_processed_values_list.Clear	();
	m_stored_values_list.Clear		();
}

void terrain_modifier_height::on_working_set_changed()
{
	for each (terrain_node_key key in m_active_id_list.Keys)
	{
		if(!m_stored_values_list.ContainsKey(key))
		{
			m_stored_values_list.Add	(key, gcnew height_diff_dict);
			m_processed_values_list.Add	(key, gcnew height_diff_dict);
		}

		height_diff_dict^ curr_stored_values_list		= m_stored_values_list[key];
		terrain_node^ curr_terrain						= m_tool_terrain->get_terrain_core()->m_nodes[key];

		for each (u16 vert_idx in m_active_id_list[key])
		{
			if(!curr_stored_values_list->ContainsKey(vert_idx))
				curr_stored_values_list[vert_idx] = curr_terrain->m_vertices[vert_idx].height;
		}
	}
}

void terrain_modifier_height::create_command()
{
	m_apply_command			= gcnew terrain_modifier_height_command(this, m_tool_terrain->get_terrain_core());
}

} // namespace editor
} // namespace xray
