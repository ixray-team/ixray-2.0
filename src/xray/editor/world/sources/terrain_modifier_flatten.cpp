////////////////////////////////////////////////////////////////////////////
//	Created		: 26.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_modifier_flatten.h"
#include "collision_object_dynamic.h"
#include "terrain_object.h"
#include "level_editor.h"
#include "tool_terrain.h"
#include "terrain_modifier_command.h"

using xray::math::float4x4;

namespace xray {
namespace editor {

terrain_modifier_flatten::terrain_modifier_flatten(xray::editor::level_editor^ le, tool_terrain^ tool)
:super(le, tool)
{
	m_control_id			= "terrain_modifier_flatten";
	m_min_strength			= -150.0f;
	m_max_strength			= 150.0f;
}

terrain_modifier_flatten::~terrain_modifier_flatten()
{}

void terrain_modifier_flatten::execute_input()
{
	System::Drawing::Point screen_xy;
	m_level_editor->ide()->get_mouse_position(screen_xy);
	if(screen_xy == m_last_screen_xy)
		return;

	m_last_screen_xy					= screen_xy;
	
	m_level_editor->get_command_engine()->preview();
}

void terrain_modifier_flatten::do_work()
{
	// do work
	refresh_active_working_set			(shape_type);

	float4x4 inv_transform;

	for each (terrain_node_key key in m_active_id_list.Keys)
	{
		height_diff_dict^ curr_processed_values_list	= m_processed_values_list[key];
		height_diff_dict^ curr_stored_values_list		= m_stored_values_list[key];
		vert_id_list^ curr_processed_id_list			= m_processed_id_list[key];
		terrain_node^ curr_terrain						= m_tool_terrain->get_terrain_core()->m_nodes[key];

		inv_transform.try_invert			(curr_terrain->get_transform());
		float3 center_position_local		= inv_transform.transform_position(*m_center_position_global);

		for each (u16 vert_idx in m_active_id_list[key])
		{
			terrain_vertex% v				= curr_terrain->m_vertices[vert_idx];

			float const distance			= curr_terrain->distance_xz(vert_idx, center_position_local);

			ASSERT							(distance<radius);

			float original_height			= curr_stored_values_list[vert_idx];
			float needed_diff				= strength - original_height;
			float vertex_influence			= calc_influence_factor(distance) * needed_diff;
			float new_height				= original_height + vertex_influence;

			if(	!curr_processed_values_list->ContainsKey(vert_idx) ||
				( math::abs(curr_processed_values_list[vert_idx]-original_height) < math::abs(vertex_influence)) )
			{
				curr_processed_id_list->Add				(vert_idx);
				curr_processed_values_list[vert_idx]	= new_height;
			}

			v.height		= curr_processed_values_list[vert_idx];
			curr_terrain->m_vertices[vert_idx]			 = v;
		}// verts
		
		m_tool_terrain->get_terrain_core()->sync_visual_vertices(curr_terrain, m_active_id_list[key]);
	} // keys

	// create update-command from vertex set m_active_list
}

} // namespace editor
} // namespace xray
