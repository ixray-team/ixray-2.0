////////////////////////////////////////////////////////////////////////////
//	Created		: 14.05.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "actions.h"
#include "editor_world.h"
#include "project.h"
#include "object_base.h"
#include "input_engine.h"
#include "command_drop_objects.h"
#include "editor_control_base.h"
#include "level_editor.h"
#include "terrain_modifier_control.h"
#include "collision_object.h"


using namespace System;
using namespace System::Windows::Forms;

namespace xray {	
namespace editor {


action_delegate::action_delegate ( String^ name, execute_delegate_managed^ execute_del ):
action_single			( name ),
enabled_delegate_man	( nullptr ),
execute_delegate_unman	( NULL ),
enabled_delegate_unman	( NULL ),
checked_delegate_man	( nullptr )
{
	ASSERT( execute_del != nullptr );
	execute_delegate_man = execute_del;
}

action_delegate::action_delegate ( String^ name, execute_delegate_unmanaged* execute_del ):
action_single			( name ),
execute_delegate_man	( nullptr ),
enabled_delegate_man	( nullptr ),
enabled_delegate_unman	( NULL ),
checked_delegate_man	( nullptr )
{
	ASSERT( execute_del != nullptr );
	execute_delegate_unman = execute_del;

}

void action_delegate::set_enabled(enabled_delegate_managed^ d )
{
	enabled_delegate_man = d;
}

void action_delegate::set_enabled(enabled_delegate_unmanaged* d )
{
	enabled_delegate_unman = d;
}
	
void action_delegate::set_checked(checked_delegate_managed^ d )
{
	checked_delegate_man	= d;
}

action_delegate::~action_delegate ()
{
	DELETE (execute_delegate_unman);
	DELETE (enabled_delegate_unman);
}

editor_world_action::editor_world_action(String^ name, editor_world* world)
:action_single		( name ),
m_editor_world		( world ) 
{
}

void action_undo::execute	() 
{
	m_editor_world->rollback( -1 );
}

bool action_undo::enabled	()
{
	return !m_editor_world->undo_stack_empty( );
}

void action_redo::execute	() 
{
	m_editor_world->rollback( 1 );
}

bool action_redo::enabled	()
{
	return !m_editor_world->redo_stack_empty( );
}

void action_show_sound_editor::execute	() 
{
	m_editor_world->show_sound_editor();
}

void action_show_dialog_editor::execute	() 
{
	m_editor_world->show_dialog_editor();
}

void action_show_particle_editor::execute	() 
{
	m_editor_world->show_particle_editor();
}

void action_show_texture_editor::execute	() 
{
	m_editor_world->show_texture_editor();
}

void action_pause::execute	() 
{
	bool current	= m_editor_world->paused();
	m_editor_world->pause(!current);
}

checked_state action_pause::checked_state()
{
	return m_editor_world->paused() ? 
		checked_state::checked : 
		checked_state::unchecked;
}

void action_editor_mode::execute	() 
{
	bool current = m_editor_world->editor_mode();
	m_editor_world->editor_mode(!current);
}

xray::editor::checked_state action_editor_mode::checked_state()
{
	return m_editor_world->editor_mode() ? 
		checked_state::checked : 
		checked_state::unchecked;
}

action_focus::action_focus( String^ name, level_editor^ le):
action_single			( name ),
m_level_editor			( le ) ,
m_halo_factor			( 1.f )
{
}

bool action_focus::enabled()
{
	return true;
}

void action_focus::execute() 
{
	if(m_level_editor->selection_not_empty())
		do_focus_selection( );
	else
		do_focus_pick_collision( );
}

void action_focus::do_focus_pick_collision( )
{
//	float distance		= 20.f;
	float distance		= m_level_editor->focus_distance();
	collision_object const* picked_collision = NULL;
	float3					picked_position;

	bool res = m_level_editor->get_picker().ray_query(collision_type_dynamic, &picked_collision, true, &picked_position);
	if(res)
	{
		float4x4 view_inv	= m_level_editor->get_inverted_view_matrix();
		float4x4 view		= math::invert4x3( view_inv );


		float3 new_pos		= view_inv.k.xyz()*(-distance)+ picked_position;
		float4x4 translate	= create_translation( new_pos - view_inv.c.xyz() );

		m_level_editor->get_editor_world().get_camera_props().view_point_distance	= distance;
		m_level_editor->get_editor_world().get_flying_camera().set_destination( math::invert4x3( view_inv*translate ) );
	}
}

void action_focus::do_focus_selection( )
{
	object_list^ list = m_level_editor->get_project()->selection_list();

//	float distance			= 20.f;
	float distance		= m_level_editor->focus_distance();
	math::float3 view_point ( 0, 0, 0 );

	math::float4x4 view_inv = m_level_editor->get_inverted_view_matrix();
	math::float4x4 view		= math::invert4x3( view_inv );

	if( list->Count >0  )
	{
		math::aabb view_aabb = create_min_max( float3( math::float_max, math::float_max, math::float_max ), float3( math::float_min, math::float_min, math::float_min ) );

		for each(object_base^ o in list)
		{
			view_aabb.modify( o->aabb( ).modify( view ) );
		}

		math::float4x4 projection = m_level_editor->get_editor_world().get_projection_matrix();

		math::float3 extens		= view_aabb.extents();
		math::float3 center		= view_aabb.center();
		float width				= extens.x*projection.e00;// /( center.z-extens.z );
		float height			= extens.y*projection.e11;// /( center.z-extens.z );

		float max_dimension		= math::max( width, height );

		distance				= max_dimension/m_halo_factor+extens.z;
		view_point				= view_inv.transform_position(center);
	}

	math::float3 new_pos		= view_inv.k.xyz()*(-distance)+ view_point;
	math::float4x4 translate	= create_translation( new_pos - view_inv.c.xyz() );

	m_level_editor->get_editor_world().get_camera_props().view_point_distance = distance;
	m_level_editor->get_editor_world().get_flying_camera().set_destination( math::invert4x3( view_inv*translate ) );
}

action_select_editor_control::action_select_editor_control ( String^ name, level_editor^ le, System::String^ control_id ):
action_single			( name ),
m_custom_editor			( le ),
m_control_id			( control_id )
{
}

void action_select_editor_control::execute	() 
{
	m_custom_editor->set_active_control(m_control_id);
}

checked_state action_select_editor_control::checked_state()
{
	if(m_control_id==nullptr && m_custom_editor->get_active_control()==nullptr)
		return checked_state::checked;
	else
	if(m_custom_editor->get_active_control() && m_custom_editor->get_active_control()->name == m_control_id) 
		return checked_state::checked;
	else
		return checked_state::unchecked;
}

action_drop_objects::action_drop_objects ( String^ name, level_editor^ le, bool from_top ):
action_single			( name ),
m_level_editor			( le ),
m_from_top				(from_top)
{
}

void action_drop_objects::execute	() 
{
	m_level_editor->get_command_engine()->run( gcnew command_drop_objects( m_level_editor, m_from_top ));
}

bool action_drop_objects::enabled	()
{
	return m_level_editor->selection_not_empty();
}

} // namespace xray
} // namespace editor
