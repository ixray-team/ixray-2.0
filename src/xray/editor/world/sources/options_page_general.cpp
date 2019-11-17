#include "pch.h"
#include "options_page_general.h"
#include "editor_world.h"

namespace xray {
namespace editor {


void options_page_general::activate_page()
{
	bool current_value					= m_editor_world.m_b_draw_grid;
	m_backup_values["draw_grid"]		= current_value;
	draw_grid_check_box1->Checked		= current_value;

	current_value						= m_editor_world.m_b_draw_collision;
	m_backup_values["draw_collision"]	= current_value;
	draw_collision_check_box->Checked	= current_value;

	math::float2 near_far				= m_editor_world.near_far_plane();
	
	m_backup_values["near_plane"]		= near_far.x;
	m_near_plane_box->Value				= System::Convert::ToDecimal(near_far.x);
	
	m_backup_values["far_plane"]		= near_far.y;
	m_far_plane_box->Value				= System::Convert::ToDecimal(near_far.y);
	
	float current_focus_dist			= m_editor_world.focus_distance();
	m_backup_values["focus_dist"]		= current_focus_dist;
	m_focus_distance_box->Value			= System::Convert::ToDecimal(current_focus_dist);
	Visible								= true;
}

bool options_page_general::changed( )
{
	bool current_value		= m_editor_world.m_b_draw_grid;
	bool backup_value		= System::Convert::ToBoolean(m_backup_values["draw_grid"]);
	bool changed			= (current_value!=backup_value);

	current_value			= m_editor_world.m_b_draw_collision;
	backup_value			= System::Convert::ToBoolean(m_backup_values["draw_collision"]);
	changed					|= (current_value!=backup_value);

	math::float2 near_far_backup;
	near_far_backup.x		= System::Convert::ToSingle(m_backup_values["near_plane"]);
	near_far_backup.y		= System::Convert::ToSingle(m_backup_values["far_plane"]);

	changed					|= (m_near_plane_box->Value != System::Convert::ToDecimal(near_far_backup.x) );
	changed					|= (m_far_plane_box->Value != System::Convert::ToDecimal(near_far_backup.y) );

	float focus_dist_backup	= System::Convert::ToSingle(m_backup_values["focus_dist"]);
	changed					|= (m_focus_distance_box->Value != System::Convert::ToDecimal(focus_dist_backup) );

	return					changed;
}

void options_page_general::deactivate_page()
{
	Visible = false;
}

bool options_page_general::accept_changes()
{
	return								true;
}

void options_page_general::cancel_changes()
{
	bool backup_value					= System::Convert::ToBoolean(m_backup_values["draw_grid"]);
	m_editor_world.m_b_draw_grid		= backup_value;

	math::float2 near_far_backup;
	near_far_backup.x		= System::Convert::ToSingle(m_backup_values["near_plane"]);
	near_far_backup.y		= System::Convert::ToSingle(m_backup_values["far_plane"]);

	m_editor_world.near_far_plane() = near_far_backup;

	float focus_dist_backup	= System::Convert::ToSingle(m_backup_values["focus_dist"]);
	m_editor_world.focus_distance() = focus_dist_backup;
}

void options_page_general::draw_grid_check_box1_CheckedChanged(System::Object^, System::EventArgs^)
{
	m_editor_world.m_b_draw_grid = draw_grid_check_box1->Checked;
}

void options_page_general::draw_collision_check_box_CheckedChanged(System::Object^, System::EventArgs^)
{
	m_editor_world.m_b_draw_collision = draw_collision_check_box->Checked;
}

void options_page_general::m_near_plane_box_ValueChanged(System::Object^, System::EventArgs^)
{
	math::float2 near_far;
	near_far.x		= System::Convert::ToSingle(m_near_plane_box->Value);
	near_far.y		= System::Convert::ToSingle(m_far_plane_box->Value);
	m_editor_world.near_far_plane() = near_far;
}

void options_page_general::m_focus_distance_box_ValueChanged(System::Object^, System::EventArgs^)
{
	float val		= System::Convert::ToSingle(m_focus_distance_box->Value);
	m_editor_world.focus_distance() = val;
}


} //	namespace editor
} //	namespace xray
