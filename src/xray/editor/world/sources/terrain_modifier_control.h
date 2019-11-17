////////////////////////////////////////////////////////////////////////////
//	Created		: 14.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_MODIFIER_CONTROL_H_INCLUDED
#define TERRAIN_MODIFIER_CONTROL_H_INCLUDED

#include "editor_control_base.h"
#include "terrain_quad.h"

namespace xray {
namespace editor {

class collision_object_dynamic;
ref class level_editor;
ref class tool_terrain;
ref class terrain_control_cursor;
ref class terrain_modifier_command;

public ref class terrain_modifier_control_base abstract: public editor_control_base
{
	typedef editor_control_base super;
public:
								terrain_modifier_control_base	(level_editor^ le, tool_terrain^ tool);
	virtual						~terrain_modifier_control_base	();
	virtual		void			initialize				() override;
	virtual		void			destroy					() override;
	virtual		void			show					(bool show) override;
	virtual		void			activate				(bool b_activate) override;
	virtual		void			update					() override;
	virtual		void			start_input				() override;
	virtual		void			end_input				() override;
	virtual		void			draw					(math::float4x4  const& view_matrix) override;
	virtual		void			set_draw_geomtry		(bool draw) override;
	virtual		void			resize					(float size) override;
	virtual		void			set_mode_modfiers		(bool plane_mode, bool free_mode) override;
	virtual		void			collide					(collision_object const* , float3 const& ) override {};
	virtual		u32				acceptable_collision_type() override;
	virtual		void			do_work					()	= 0;
	virtual		void			change_property			(System::String^ prop_name, float const value) override;
	virtual		void			load_settings			(RegistryKey^ key)	override;
	virtual		void			save_settings			(RegistryKey^ key)	override;
	virtual		void			pin						(bool bpin) override;
				float3			get_picked_point		();
public:
	property modifier_shape		shape_type;
	property float				radius;
	property float				hardness;
	property float				distortion;
	property float				strength;

	level_editor^				m_level_editor;
	tool_terrain^				m_tool_terrain;
	float						strength_value				();

protected:
	terrain_modifier_command^	m_apply_command;
	
	key_vert_id_dict			m_active_id_list;
	key_vert_id_dict			m_processed_id_list;

	System::Drawing::Point		m_last_screen_xy;
	math::int2					get_mouse_position			();

	float3*						m_center_position_global;

	bool						m_visible;
	terrain_control_cursor^		m_cursor;
	
	float						m_min_strength;
	float						m_max_strength;

	virtual		void			on_working_set_changed		() {};
	virtual		void			create_command				()	= 0;
				float			calc_influence_factor		(float const& distance);
				void			refresh_active_working_set	(modifier_shape type);
}; // class terrain_modifier_control_base


public ref class terrain_modifier_height abstract: public terrain_modifier_control_base
{
	typedef terrain_modifier_control_base	super;
public:
								terrain_modifier_height		(level_editor^ le, tool_terrain^ tool);
	virtual						~terrain_modifier_height	();
	virtual		void			end_input					() override;

protected:
	virtual		void			on_working_set_changed		() override;
	virtual		void			create_command				() override;
	key_height_diff_dict		m_processed_values_list;
	key_height_diff_dict		m_stored_values_list;
}; // class terrain_modifier_height

} // namespace editor
} // namespace xray

#endif // #ifndef TERRAIN_MODIFIER_CONTROL_H_INCLUDED