////////////////////////////////////////////////////////////////////////////
//	Created		: 23.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_modifier_smooth.h"
#include "collision_object_dynamic.h"
#include "terrain_object.h"
#include "tool_terrain.h"
#include "level_editor.h"
#include "terrain_modifier_command.h"

using xray::math::float4x4;

namespace xray {
namespace editor {

enum class get_height_result{ok, missing};

ref struct terrain_helper
{
	terrain_helper(terrain_node^ t)
	:terrain(t)
	{
	}

	get_height_result get_height(u16 origin_vert_idx, int const& offset_x_idx, int const& offset_z_idx, float& result)
	{
		int dim			= terrain->m_dimension;

		int _x, _z;
		terrain->vertex_xz(origin_vert_idx, _x, _z);
		int x_new		= _x + offset_x_idx;
		int z_new		= _z + offset_z_idx;
	
		if((x_new<0) || (x_new>dim) || (z_new<0) || (z_new>dim))
		{
			terrain_node_key thiskey = terrain->m_tmp_key;

			if(x_new<0)
				thiskey.x	-=1;
			else
			if(x_new>dim)
				thiskey.x	+=1;

			if(z_new<0)
				thiskey.z	+=1;
			else
			if(z_new>dim)
				thiskey.z	-=1;

			if(terrain->get_terrain_core()->m_nodes.ContainsKey(thiskey))
			{
				terrain_node^ t = terrain->get_terrain_core()->m_nodes[thiskey];
				int t_dim		= t->m_dimension;
				ASSERT			(t_dim==dim);
				int dx			= x_new;
				int dz			= z_new;

				if(x_new<0)
					dx = (t_dim - math::abs(x_new));
				else
				if(x_new>dim)
					dx = (x_new - dim);

				if(z_new<0)
					dz = (t_dim - math::abs(z_new));
				else
				if(z_new>dim)
					dz = (z_new-dim);

				u32 idx			= terrain->vertex_id(dx, dz);
				result			= t->m_vertices[idx].height;
				return			get_height_result::ok;
			}else
				return			get_height_result::missing;
		}


		u16 resulting_idx = terrain->vertex_id(x_new, z_new);

		result = terrain->m_vertices[resulting_idx].height;
		
		return get_height_result::ok;
	}

	terrain_node^ terrain;
};

terrain_modifier_smooth::terrain_modifier_smooth(xray::editor::level_editor^ le, tool_terrain^ tool)
:super(le, tool)
{
	m_control_id			= "terrain_modifier_smooth";
	m_timer					= NEW(xray::timing::timer);
	m_min_strength			= 0.01f;
	m_max_strength			= 0.9f;
}

terrain_modifier_smooth::~terrain_modifier_smooth()
{
	DELETE					(m_timer);
}

void terrain_modifier_smooth::start_input()
{
	System::Drawing::Point screen_xy;
	m_level_editor->ide()->get_mouse_position(screen_xy);
	if(screen_xy != m_last_screen_xy)
		refresh_active_working_set	(shape_type);

	m_last_screen_xy				= screen_xy;

	m_timer->start				();
	m_last_screen_xy.X			= -10000;// force first update
	m_last_screen_xy.Y			= -10000;
	m_matrix_dim				= 7;
	build_gaussian_matrix		();

	super::start_input			();
}


void terrain_modifier_smooth::execute_input()
{
	System::Drawing::Point screen_xy;
	m_level_editor->ide()->get_mouse_position(screen_xy);
	if(screen_xy != m_last_screen_xy)
		refresh_active_working_set	(shape_type);

	m_last_screen_xy				= screen_xy;

	m_level_editor->get_command_engine()->preview();
}

void terrain_modifier_smooth::do_work()
{
	float const time			= m_timer->get_elapsed_sec();

	if(time<0.01f)
		return;

	m_timer->start				();

	float4x4					inv_transform;


	for each (terrain_node_key key in m_active_id_list.Keys)
	{
		height_diff_dict^ curr_processed_values_list	= m_processed_values_list[key];
		vert_id_list^ curr_processed_id_list			= m_processed_id_list[key];
		terrain_node^ curr_terrain						= m_tool_terrain->get_terrain_core()->m_nodes[key];
		terrain_helper									helper(curr_terrain);

		inv_transform.try_invert			(curr_terrain->get_transform());
		float3 center_position_local		= inv_transform.transform_position(*m_center_position_global);

		for each (u16 vert_idx in m_active_id_list[key])
		{
			float	influenced_verts_count	= 0.0;
			float	current_vert_sum		= 0.0;

			for(int ix=0; ix<m_matrix_dim; ++ix)
			{
				int		_x					= ix - (m_matrix_dim>>1);
				for (int iy=0; iy<m_matrix_dim; ++iy)
				{
					int _y					= iy - (m_matrix_dim>>1);
					float _h				= math::float_min;
					float infl				= m_gaussian_matrix[ix, iy];

					if(get_height_result::ok == helper.get_height(vert_idx, _x, _y, _h) )
					{
						ASSERT(math::valid(_h));
						influenced_verts_count	+= infl;
						current_vert_sum		+= _h * infl;
					}
					// else out of terrain bound
				}
			}// in matrix
			ASSERT(influenced_verts_count>0.0);

			terrain_vertex% v				= curr_terrain->m_vertices[vert_idx];
			float const distance			= curr_terrain->distance_xz(vert_idx, center_position_local);

			ASSERT							(distance<radius+math::epsilon_3);
			float influence					= calc_influence_factor(distance);

			float vert_height_new			= current_vert_sum / influenced_verts_count;
			float vert_height_old			= v.height;
			float vert_height				= vert_height_old + influence * (vert_height_new - vert_height_old); // lerp

			if(math::is_zero(vert_height))
				vert_height = 0.0f;

			curr_processed_values_list[vert_idx]	= (float)vert_height;
			
			if(!curr_processed_id_list->Contains(vert_idx))
				curr_processed_id_list->Add		(vert_idx);

		} // verts in node

	}// keys

	for each (terrain_node_key key in m_active_id_list.Keys)
	{
		height_diff_dict^ curr_processed_values_list	= m_processed_values_list[key];
		terrain_node^ curr_terrain						= m_tool_terrain->get_terrain_core()->m_nodes[key];
		
		for each (u16 vert_idx in m_active_id_list[key])
		{
			terrain_vertex% v	= curr_terrain->m_vertices[vert_idx];
			v.height			= curr_processed_values_list[vert_idx];
			curr_terrain->m_vertices[vert_idx] = v;
		}

		m_tool_terrain->get_terrain_core()->sync_visual_vertices(curr_terrain, m_active_id_list[key] );
	}

}

void terrain_modifier_smooth::build_gaussian_matrix()
{
	m_gaussian_matrix			= gcnew cli::array<float, 2>(m_matrix_dim, m_matrix_dim);

	float g0					= 1/(2 * math::pi * strength*strength);

	for (int ix=0; ix<m_matrix_dim; ++ix)
	{
		int _x						= ix - (m_matrix_dim>>1);
		for (int iy=0; iy<m_matrix_dim; ++iy)
		{
			int _y					= iy - (m_matrix_dim>>1);
			float g1				= exp(-(_x*_x + _y*_y)/ (2*strength*strength));
			float g					= g0*g1;

			m_gaussian_matrix[ix, iy]= g;
		}
	}
}

} // namespace editor
} // namespace xray
