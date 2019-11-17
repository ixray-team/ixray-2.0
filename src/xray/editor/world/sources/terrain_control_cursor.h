////////////////////////////////////////////////////////////////////////////
//	Created		: 08.02.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_CONTROL_CURSOR_H_INCLUDED
#define TERRAIN_CONTROL_CURSOR_H_INCLUDED

#include <xray/render/base/world.h>
#include <xray/render/base/visual.h>

namespace xray {
namespace editor {

ref class terrain_core;
ref class terrain_node;
ref class terrain_modifier_control_base;

//tmp ???
struct vert_struct
{
	float3	position;
	float3	normal;
	float2	uv;
};

public ref class terrain_control_cursor
{
public:
					terrain_control_cursor	(terrain_modifier_control_base^ o);
					~terrain_control_cursor	();

		void		update					();
		void		set_inner_raduis		(float radius);
		void		set_outer_raduis		(float radius);
		void		show					(bool show);
		void		pin						(bool bpin){m_pinned=bpin;}
		float3*		m_last_picked_position;
protected:
		void		update					(terrain_core^ terrain_core, float3 const& picked_position_global);
		void		generate_geometry		();
		void		on_visual_ready			(xray::resources::queries_result& data);
		void		update_ring				(terrain_core^ terrain, 
											float3 const& picked_position_global,  
											float radius, 
											float width, 
											u16 segments_count, 
											vector<vert_struct>& vertices);

	terrain_modifier_control_base^			m_owner;
	
	render::visual_ptr*				m_visual_inner_ring;
	render::visual_ptr*				m_visual_outer_ring;
	render::visual_ptr*				m_visual_center_point;
	render::visual_ptr*				m_visual_strength;

	vector<vert_struct>*					m_inner_vertices;
	vector<vert_struct>*					m_outer_vertices;
	vector<vert_struct>*					m_center_vertices;
	vector<vert_struct>*					m_strength_vertices;

	bool									m_shown;
	u32										m_visual_inner_ring_id;
	u32										m_visual_outer_ring_id;
	u32										m_visual_center_point_id;
	u32										m_visual_strength_id;
	
	float									m_inner_radius;
	float									m_inner_width;
	u16										m_inner_segments;
	float									m_outer_radius;
	float									m_outer_width;
	u16										m_outer_segments;
	float									m_center_radius;
	float									m_center_width;
	u16										m_center_segments;
	System::Drawing::Point					m_last_mouse_pos;
	bool									m_pinned;
};

} // namespace editor
} // namespace xray

#endif // #ifndef TERRAIN_CONTROL_CURSOR_H_INCLUDED