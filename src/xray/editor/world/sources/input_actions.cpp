////////////////////////////////////////////////////////////////////////////
//	Created		: 12.02.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "input_actions.h"
#include "editor_world.h"
#include "window_view.h"
#include "project.h"
#include "transform_control_translation.h"
#include "terrain_modifier_control.h"
#include "collision_object_dynamic.h"
#include "collision_object_control.h"
#include "flying_camera.h"
#include "object_base.h"
#include "level_editor.h"
#include "actions.h"
#include "action_engine.h"

// commands
#include "command_select.h"
#include "command_delete_object.h"
#include "command_snap_objects.h"
#include "command_place_objects.h"

#include "window_ide.h"

#pragma unmanaged
#include <xray/collision/object.h>
#include <xray/collision/space_partitioning_tree.h>
#include <xray/render/base/world.h>
#include <xray/editor/world/engine.h>
#pragma managed

using xray::collision::result;

namespace xray {
namespace editor {

mouse_action_base::mouse_action_base			( System::String^ name, editor_world* world ):
action_continuous		( name )
{
	m_world			= world;
	ASSERT			( m_world );

	m_inverted_view_captured	= NEW( float4x4 )( );
}

mouse_action_base::~mouse_action_base			( )
{
	DELETE			( m_inverted_view_captured );
}

void mouse_action_base::init_capture()
{
	m_world->ide()->get_mouse_position	(m_mouse_last_pos);
	m_world->ide()->get_view_size		(m_viewport_size);
	(*m_inverted_view_captured)			= m_world->get_inverted_view_matrix();
}

mouse_action_view_move_xy::mouse_action_view_move_xy( System::String^ name, editor_world* world  )
:mouse_action_base			( name, world )
{
}

void mouse_action_view_move_xy::execute				() 
{ 
	// Get Mouse current position
	Point mouse_position;
	m_world->ide()->get_mouse_position(mouse_position);

	// Factors for moving in the direction axises   
	float right = 0.f;
	float up = 0.f;

	// Mouse move start point 
	float2 drag_start	(	(float)m_mouse_last_pos.X ,	(float)m_mouse_last_pos.Y	);

	// Mouse move current point
	float2 drag			(	(float)mouse_position.X,	(float)mouse_position.Y	);

	// Convert screen coordinates to 3d space coordinates by a given z coordinate. 
	float3 drag_start_3d	= m_world->screen_to_3d	( drag_start, m_world->get_camera_props( ).view_point_distance );
	float3 drag_3d			= m_world->screen_to_3d	( drag, m_world->get_camera_props( ).view_point_distance );

	// Factor for moving by x axis is equal to delta of the 3d points. That is 
	// the camera will be moved right an/or left instead of moving the world around it.
	right	=	-	(drag_3d.x - drag_start_3d.x);
	// The same with y 
	up		=		(drag_3d.y - drag_start_3d.y);

	// Save the current position to calculate next delta relative to the current position.
	m_mouse_last_pos.X = mouse_position.X;
	m_mouse_last_pos.Y = mouse_position.Y;

	if( right == 0 && up == 0)
		return;

	// Set view matrix without any rotation; just moving it right/left or up/down.
	m_world->set_view_matrix			( float2( 0.f, 0.f ), 0, right, up );

	return; 
}

bool mouse_action_view_move_xy::capture			() 
{
	// Initialize starting parameters 
	super::init_capture		( );

	m_old_cursor = m_world->set_view_cursor	( enum_editor_cursors::enum_editor_cursors_cross );

	return true;
}

void mouse_action_view_move_xy::release( )
{
	m_world->set_view_cursor	( m_old_cursor );
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
mouse_action_view_move_z::mouse_action_view_move_z( System::String^ name, editor_world* world  )
:mouse_action_base			( name, world ),
m_continuse_drugging		( false )
{
	
}

void mouse_action_view_move_z::execute				() 
{ 
	// Get Mouse current position
	Point mouse_position;
	m_world->ide()->get_mouse_position(mouse_position);

	// Factor to move camera cross its z axis
	float						forward = 0.f;

	// Calculating move factor depend on mouse move delta: 
	// down and/or right moving forward, up and/or right moving back.
	forward = (float)( mouse_position.X - m_mouse_last_pos.X )  + ( mouse_position.Y - m_mouse_last_pos.Y );

	// Coefficient to reduce move speed depend in distance of view point. By decreasing distance of 
	// move speed also will be decreased.
	float speed_reduce_coeff; 

	// if the view point is too near to the camera than just setting fixed coefficient.
	if( !m_continuse_drugging )
		speed_reduce_coeff = (m_world->get_camera_props().view_point_distance + m_world->get_camera_props().view_move_z_coeff)/(m_world->get_camera_props().view_point_distance+0.001f);
	else
		speed_reduce_coeff = 2*m_world->get_camera_props().view_move_speed_factor;

	// Calculate move factor depend on speed coeeficient.
	forward /= (m_world->get_camera_props().view_move_speed_factor +  speed_reduce_coeff);

	// Save the current position a starting for next mouse move.
	m_mouse_last_pos.X = mouse_position.X;
	m_mouse_last_pos.Y = mouse_position.Y;

	// Reduce view point distance in de move factor to keep the point into its place 
	// during z move.
	m_world->get_camera_props().view_point_distance -= forward;

	// Bring the view point with camera if it is in the behind of the camera.
	if( m_world->get_camera_props().view_point_distance < 0 )
		m_world->get_camera_props().view_point_distance = 0;

	if( forward == 0 )
		return;

	// Set new view matrix my moving it forward or back according to factor.
	m_world->set_view_matrix				( math::float2( 0.f, 0.f ), forward, 0, 0 );
	return;
}

bool mouse_action_view_move_z::capture			() 
{
	// Initialize starting parameters 
	super::init_capture();

	if( m_world->get_camera_props().view_point_distance < m_world->get_camera_props().view_point_distance_min )
		m_continuse_drugging = true;
	else
		m_continuse_drugging = false;

	m_world->set_view_cursor	( enum_editor_cursors::enum_editor_cursors_cross );
	return true;
}

void mouse_action_view_move_z::release		()
{
	m_world->set_view_cursor	( m_old_cursor );
}


//////////////////////////////////////////////////////////////////////////
mouse_action_view_rotate_around::mouse_action_view_rotate_around( System::String^ name, editor_world* world  )
:mouse_action_view_rotate_base( name, world ),
m_rotation_origin_x			( 0 ),
m_rotation_origin_y			( 0 ),
m_rotation_origin_z			( 0 )
{

}

void mouse_action_view_rotate_around::execute				() 
{ 
	// Get Mouse current position
	Point mouse_position;
	m_world->ide()->get_mouse_position(mouse_position);

	// Compute rotation factors from mouse move.
	float const	angle_x			= .5f * math::deg2rad( float( mouse_position.Y - m_mouse_last_pos.Y ) );
	float const	angle_y			= .5f * math::deg2rad( float( mouse_position.X - m_mouse_last_pos.X )*.75f );

	// draw rotation origin
	float3 cp(m_rotation_origin_x, m_rotation_origin_y, m_rotation_origin_z);
	float3 sh_x(2.0f, 0.0f, 0.0f);
	float3 sh_y(0.0f, 2.0f, 0.0f);
	float3 sh_z(0.0f, 0.0f, 2.0f);

	m_world->get_renderer().draw_line( cp-sh_x, cp, 0xffffff00 );
	m_world->get_renderer().draw_line( cp-sh_y, cp, 0xffffff00 );
	m_world->get_renderer().draw_line( cp-sh_z, cp, 0xffffff00 );
	
	m_world->get_renderer().draw_line( cp, cp+sh_x, 0xffff0000 );
	m_world->get_renderer().draw_line( cp, cp+sh_y, 0xff00ff00 );
	m_world->get_renderer().draw_line( cp, cp+sh_z, 0xff0000ff );

	if( angle_x == 0 && angle_y == 0)
		return;

	// set rotated view by angle_x angle_y around rotation origin point.
	set_rotated_view_matrix		( float2( angle_x, angle_y ), float3( m_rotation_origin_x, m_rotation_origin_y, m_rotation_origin_z) );
	return;
}

bool mouse_action_view_rotate_around::capture			() 
{
	// Initialize starting parameters 
	super::init_capture();

	// Compute the rotation origin point 
	float4x4 inv_view_matrix	= *m_inverted_view_captured;
	float3 rotation_origin		= (inv_view_matrix.c +  inv_view_matrix.k * m_world->get_camera_props().view_point_distance ).xyz();

	// Save the point
	m_rotation_origin_x 	= rotation_origin.x;
	m_rotation_origin_y 	= rotation_origin.y;
	m_rotation_origin_z 	= rotation_origin.z;

	m_old_cursor = m_world->set_view_cursor	( enum_editor_cursors::enum_editor_cursors_cross );

	return true;
}

void mouse_action_view_rotate_around::release		()
{
	m_world->set_view_cursor	( m_old_cursor );
}

void mouse_action_view_rotate_base::set_rotated_view_matrix	 ( float2 const& raw_angles, float3 point )
{
	// Rotation matrix for the starting position
	float4x4	rotation				= math::create_rotation_INCORRECT( m_inverted_view_captured->get_angles_xyz_INCORRECT( ) );

	// Rotation matrix to rotate around X axis of the camera.
	float4x4	rotation_X_camrea		= math::create_rotation( m_inverted_view_captured->i.xyz(), -raw_angles.x );

	// Rotation matrix to rotate around Y axis of the world.
	float4x4	rotation_Y_world		= math::create_rotation_y( -raw_angles.y*(m_inverted_view_captured->j.xyz().y/abs(m_inverted_view_captured->j.xyz().y)) );

	// Rotation matrix to rotate around Y axis of the camera.
	float4x4 point_tranlate			= math::create_translation( point*-1 );

	// Get camera starting position
	float3 camera_pos	= m_inverted_view_captured->c.xyz( );

	// Move it to the origin: to rotate it around the origin.
	camera_pos = point_tranlate.transform_position( camera_pos);

	// Rotate the camera around the origin (view point).
	float3 position		= (rotation_X_camrea* rotation_Y_world).transform_position( camera_pos );// + trans;

	// Return the point into its coordinate system.
	position = invert4x3(point_tranlate).transform_position( position);

	// Create translation from camera position
	float4x4 const translation	= math::create_translation( position );

	// Combine all transformation and create a view matrix.
	m_world->set_view_matrix(	math::invert4x3 ( rotation * rotation_X_camrea * rotation_Y_world * translation ));
}


//////////////////////////////////////////////////////////////////////////
mouse_action_view_move_y_reverse::mouse_action_view_move_y_reverse( System::String^ name, editor_world* world  )
:mouse_action_base			( name, world )
{
}

void mouse_action_view_move_y_reverse::execute				() 
{ 
	// Get Mouse current position
	Point mouse_position	= m_world->ide()->get_mouse_pos_infinte();

	// Factors for moving in the direction axises   
	float up = 0.f;

	// Mouse move start point 
	float2 drag_start	(	(float)m_mouse_last_pos.X ,	(float)m_mouse_last_pos.Y	);

	// Mouse move current point
	float2 drag			(	(float)mouse_position.X,	(float)mouse_position.Y	);

	// Calculate Delta 
	up		=		-(drag.y - drag_start.y)*0.05f;

	// Save the current position to calculate next delta relative to the current position.
	m_mouse_last_pos.Y = mouse_position.Y;

	if( up == 0 )
		return;

	// Set view matrix without any rotation; just moving it up/down by world y axis.
	float4x4 m				= m_world->get_inverted_view_matrix() * math::create_translation( float3( 0, up, 0));
	m_world->set_view_matrix( math::invert4x3( m ) );


	return; 
}

bool mouse_action_view_move_y_reverse::capture			() 
{
	// Initialize starting parameters 
	m_world->ide()->view().set_infinite_mode( true );
	super::init_capture();
	m_mouse_last_pos = m_world->ide()->get_mouse_pos_infinte();

	return true;
}

void mouse_action_view_move_y_reverse::release			() 
{
	m_world->ide()->view().set_infinite_mode( false );
}

//////////////////////////////////////////////////////////////////////////
mouse_action_view_move_xz_reverse::mouse_action_view_move_xz_reverse		( System::String^ name, editor_world* world  )
:mouse_action_base			( name, world )
{
}

void mouse_action_view_move_xz_reverse::execute				() 
{ 
	// Get Mouse current position
	Point mouse_position	= m_world->ide()->get_mouse_pos_infinte();

	// Factors for moving in the direction axises   
	float right = 0.f;
	float forward = 0.f;

	// Mouse move start point 
	float2 drag_start	(	(float)m_mouse_last_pos.X ,	(float)m_mouse_last_pos.Y	);

	// Mouse move current point
	float2 drag			(	(float)mouse_position.X,	(float)mouse_position.Y	);

	// Calculate deltas
	right				= (drag.x - drag_start.x)*0.05f;
	forward				= -(drag.y - drag_start.y)*0.05f;

	// Save the current position to calculate next delta relative to the current position.
	m_mouse_last_pos.X	= mouse_position.X;
	m_mouse_last_pos.Y	= mouse_position.Y;

	if( right == 0 && forward == 0)
		return;

	// Set view matrix without any rotation; just moving it right/left or forward/back by world axes.
	float3 right_vector = float3( 0, 1, 0 ) ^ m_world->get_inverted_view_matrix().k.xyz();

	float mag = right_vector.magnitude();
	if( mag< math::epsilon_5 )
	{
		right_vector = math::float3( 0, -1, 0 ) ^ m_world->get_inverted_view_matrix().j.xyz();
		right_vector.normalize();
	}
	else
		right_vector /= mag;

	float3 forward_vector	= float3( 0, 1, 0 ) ^ right_vector;
	forward_vector.normalize();

	forward_vector *= -forward;
	right_vector *= right;
	
	float4x4 m = m_world->get_inverted_view_matrix() * math::create_translation( forward_vector+right_vector );
	m_world->set_view_matrix			( math::invert4x3( m ) );

	return; 
}

bool mouse_action_view_move_xz_reverse::capture			() 
{
	// Initialize starting parameters 
	super::init_capture();

	m_world->ide()->view().set_infinite_mode( true );
	m_mouse_last_pos =	m_world->ide()->get_mouse_pos_infinte();

	return true;
}

void mouse_action_view_move_xz_reverse::release			() 
{
	m_world->ide()->view().set_infinite_mode( false );
}


//////////////////////////////////////////////////////////////////////////
mouse_action_view_rotate::mouse_action_view_rotate		( System::String^ name, editor_world* world  )
:mouse_action_view_rotate_base			( name, world ),
m_start_angle_x							( 0 ),
m_shift_x								( 0 )
{
}

void mouse_action_view_rotate::execute				() 
{ 
	// Get Mouse current position
	Point mouse_position	= m_world->ide()->get_mouse_pos_infinte();

	// Compute rotation factors from mouse move.
	float angle_x			= .5f*math::deg2rad( float( mouse_position.Y - m_mouse_last_pos.Y + m_shift_x ) );
	float angle_y			= .5f*math::deg2rad( float( mouse_position.X - m_mouse_last_pos.X )*.75f );

	if( angle_x == 0 && angle_y == 0)
		return;


 	if( angle_x > math::pi_d2 - m_start_angle_x )
	{
		angle_x = math::pi_d2 - m_start_angle_x;
		m_shift_x = math::rad2deg(angle_x/.5f)-(mouse_position.Y - m_mouse_last_pos.Y);
	}

	if( angle_x < -math::pi_d2 - m_start_angle_x )
	{
		angle_x = -math::pi_d2 - m_start_angle_x;
		m_shift_x = math::rad2deg(angle_x/.5f)-(mouse_position.Y - m_mouse_last_pos.Y);
	}


	// set rotated view by angle_x angle_y around rotation origin point.
	set_rotated_view_matrix		( float2( angle_x, angle_y ), m_inverted_view_captured->c.xyz() );
}

bool mouse_action_view_rotate::capture			() 
{
	// Initialize starting parameters 
	m_world->ide()->view().set_infinite_mode( true );
	super::init_capture							( );
	m_mouse_last_pos						= m_world->ide()->get_mouse_pos_infinte();
	m_start_angle_x							= m_inverted_view_captured->get_angles_xyz_INCORRECT().x;
	m_shift_x								= 0;
	
	return true;
}
void mouse_action_view_rotate::release		( )
{
	m_world->ide()->view().set_infinite_mode( false );
}

//////////////////////////////////////////////////////////////////////////
// mouse_action_left_button_control
mouse_action_left_button_control::mouse_action_left_button_control( System::String^ name, level_editor^ le, bool plane_mode )
:action_continuous	( name ),
m_level_editor		( le ),
m_active_handler	( nullptr ),
m_plane_mode		( plane_mode )
{
}

void mouse_action_left_button_control::execute		(  )
{
	if( m_active_handler )
		m_active_handler->execute_input();
}

bool mouse_action_left_button_control::capture	( )
{
	collision_object const* picked_collision = NULL;

	editor_control_base^	active_control = m_level_editor->get_active_control();
	float3					picked_position;
	
	if( active_control && m_level_editor->get_picker( ).ray_query( active_control->acceptable_collision_type(), &picked_collision, true, &picked_position ) )
	{
		ASSERT( picked_collision->get_type() & active_control->acceptable_collision_type() );

		m_active_handler					= active_control;
		
		m_active_handler->set_mode_modfiers	( m_plane_mode, false );

		m_active_handler->collide			( picked_collision, picked_position );

		m_active_handler->start_input		( );
		return								true;
	}

	return									false;
}

void mouse_action_left_button_control::release			() 
{
	if( m_active_handler )
	{
		m_active_handler->end_input	( );
		m_active_handler			= nullptr;
	}
}


//////////////////////////////////////////////////////////////////////////
// mouse_action_middle_button_control

mouse_action_middle_button_control::mouse_action_middle_button_control( System::String^ name, level_editor^ le, bool plane_mode  )
:action_continuous	( name ),
m_level_editor		( le ),
m_active_handler	( nullptr ),
m_plane_mode		( plane_mode )
{
}

void mouse_action_middle_button_control::execute(  )
{
	if( m_active_handler )
		m_active_handler->execute_input();
}

bool mouse_action_middle_button_control::capture( )
{
	editor_control_base^ active_control	= m_level_editor->get_active_control();

	if( active_control == nullptr )
		return false;

	collision_object const* picked_collision					= NULL;

	float3 picked_position;
	if( m_level_editor->get_picker( ).ray_query( active_control->acceptable_collision_type(), &picked_collision, true, &picked_position ) )
	{
		ASSERT( picked_collision->get_type() & active_control->acceptable_collision_type() );

		m_active_handler					= active_control;

		m_active_handler->set_mode_modfiers	( m_plane_mode, true );
		m_active_handler->collide			( picked_collision, picked_position );
		m_active_handler->start_input		( );
		return true;
	}
	return false;
}

void mouse_action_middle_button_control::release( ) 
{
	if( m_active_handler )
	{
		m_level_editor->get_active_control()->end_input( );
		m_active_handler					= nullptr;
	}
}	

//////////////////////////////////////////////////////////////////////////
// mouse_action_select_object
mouse_action_select_object::mouse_action_select_object( System::String^ name, level_editor^ le, enum_selection_method method )
:action_continuous	( name ),
m_level_editor		( le ),
m_collision_object	( NULL ),
m_start_x			( 0 ),
m_start_y			( 0 ),
m_select_method		( method )
{
}

bool mouse_action_select_object::capture	()
{
	m_collision_object		= NULL;

	collision_object const* object		= NULL;
	if( m_level_editor->get_picker( ).ray_query( collision_type_dynamic, &object, true ) )
		m_collision_object = static_cast_checked<collision_object_dynamic const*>(object);

	math::int2 start_xy;
	m_level_editor->ide()->get_mouse_position(start_xy);

	m_start_x = start_xy.x;
	m_start_y = start_xy.y;

	return true;
}


void mouse_action_select_object::execute		(  )
{
	math::int2 screen_xy;
	m_level_editor->ide()->get_mouse_position(screen_xy);

	screen_xy += (m_level_editor->get_viewport()/2);

	math::int2 start_xy( m_start_x, m_start_y );
	start_xy += (m_level_editor->get_viewport()/2);

	m_level_editor->get_editor_renderer().draw_screen_rectangle(	
							math::float2( (float)start_xy.x, (float)start_xy.y ), 
							math::float2( (float)screen_xy.x, (float)screen_xy.y ), 
							math::color_xrgb( 128, 128, 128 ), 
							0xCCCCCCCC );
}

void mouse_action_select_object::release			() 
{
	
	enum_selection_method select_method = m_select_method;

	math::int2 start_xy;
	m_level_editor->ide()->get_mouse_position(start_xy);

	if( math::abs( start_xy.x - m_start_x ) < 2 &&  math::abs( start_xy.y - m_start_y ) < 2 )
	{
		if( m_collision_object )
			m_level_editor->get_command_engine()->run( gcnew command_select(m_level_editor, m_collision_object->get_owner(), select_method) );
		else
		if( select_method == enum_selection_method_set )
			m_level_editor->get_command_engine()->run( gcnew command_select(m_level_editor));
			
		return;
	}
	
	math::int2 min, max;
	min.x = math::min( start_xy.x, m_start_x);
	min.y = math::min( start_xy.y, m_start_y);

	max.x = math::max( start_xy.x, m_start_x);
	max.y = math::max( start_xy.y, m_start_y);

	vector< collision_object const*> coll_objects;

	if( min.x == max.x )
		max.x+=1;

	if( min.y == max.y )
		max.y+=1;

	if( m_level_editor->get_picker().frustum_query( collision_type_dynamic, min, max, coll_objects ))
	{
		object_list^ objs = gcnew object_list;
		vector< collision_object const*>::iterator it		= coll_objects.begin();
		vector< collision_object const*>::const_iterator en	= coll_objects.end();
		for( ;it != en; ++it )
		{
			ASSERT( (*it)->get_type() & collision_type_dynamic );
			collision_object_dynamic const* collision_dynamic = static_cast_checked< collision_object_dynamic const* >(*it);
			objs->Add( collision_dynamic->get_owner() );
		}
		if(objs->Count)
		m_level_editor->get_command_engine()->run( gcnew command_select(m_level_editor, objs, select_method) );
	}
	else
		m_level_editor->get_command_engine()->run( gcnew command_select(m_level_editor));
}


//////////////////////////////////////////////////////////////////////////
// input_action_add_library_object
input_action_add_library_object::input_action_add_library_object( System::String^ name, level_editor^ le ):
action_continuous( name ),
m_level_editor(le)
{
}

bool input_action_add_library_object::enabled()
{
	return !m_level_editor->get_project()->empty() && m_level_editor->get_active_control()==nullptr;
}

bool input_action_add_library_object::capture	()
{
	// If the ray doesn't touch the ground then just place the object on the 0 plane.
	float3	origin;
	float3	direction;
	float3	picked_position;

	m_level_editor->get_mouse_ray	( origin, direction );

	collision_object const* picked_collision = NULL;


	bool bpick = m_level_editor->get_picker( ).ray_query(	collision_type_dynamic, 
															origin, 
															direction, 
															&picked_collision, 
															true, 
															&picked_position );

	if( !bpick )
	bpick = transform_control_base::plane_ray_intersect(float3( 0, 0, 0 ),	
															float3( 0, 1, 0 ),
															origin,						
															direction,
															picked_position);
	if( bpick )
	{
		m_level_editor->add_library_object		( picked_position );
	}else
	{
		// Here need to rise an error message ....
	}

	return true;
}

void input_action_add_library_object::execute		(  )
{
}


//////////////////////////////////////////////////////////////////////////
// key_action_snap
key_action_snap::key_action_snap( System::String^ name, level_editor^ le)
:action_continuous	( name ),
m_level_editor		( le ),
m_snap_command		( nullptr )
{
}

bool key_action_snap::capture	()
{
	m_snap_command = gcnew command_snap_objects(m_level_editor);
	m_level_editor->get_command_engine()->run_preview( m_snap_command );

	return true;
}

void key_action_snap::execute		(  )
{
	ASSERT( m_snap_command );
	m_level_editor->get_command_engine()->preview();

}

void key_action_snap::release			() 
{

	ASSERT( m_snap_command );
	m_level_editor->get_command_engine()->end_preview();

	m_snap_command = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// key_action_place
key_action_place::key_action_place	( System::String^ name, level_editor^ le)
:action_continuous	( name ),
m_level_editor		( le ),
m_snap_command		( nullptr )
{
}

bool key_action_place::capture	()
{
	m_snap_command = gcnew command_place_objects(m_level_editor);
	m_level_editor->get_command_engine()->run_preview( m_snap_command );

	return true;
}

void key_action_place::execute		(  )
{
	ASSERT( m_snap_command );
	m_level_editor->get_command_engine()->preview();

}

void key_action_place::release			( ) 
{

	ASSERT( m_snap_command );
	m_level_editor->get_command_engine()->end_preview();

	m_snap_command = nullptr;
}

mouse_action_control_properties::mouse_action_control_properties(System::String^ action_name, level_editor^ le, System::String^ prop_name, float scale)
:super			( action_name ),
m_prop_name		( prop_name ),
m_level_editor	( le ),
m_scale			( scale )
{}


mouse_action_control_properties::~mouse_action_control_properties( )
{}

bool mouse_action_control_properties::enabled( )
{
	if(m_level_editor->get_active_control() == nullptr)
		return false;

	terrain_modifier_control_base^ tc = dynamic_cast<terrain_modifier_control_base^>(m_level_editor->get_active_control());
	return (tc!= nullptr);
}

bool mouse_action_control_properties::capture( )
{
	while ( ShowCursor( FALSE ) >= 0 );

	m_level_editor->ide()->get_mouse_position(m_mouse_capture_pos);
	m_level_editor->get_active_control()->pin(true);
	return true;
}

void mouse_action_control_properties::release( )
{
	while ( ShowCursor( TRUE ) <= 0 );

	super::release		( );
	m_level_editor->get_active_control()->pin(false);
}

void mouse_action_control_properties::execute( )
{
	super::execute		( );

	System::Drawing::Point	mouse_position;
	m_level_editor->ide()->get_mouse_position(mouse_position);

	if(mouse_position != m_mouse_capture_pos)
	{
		float dx		= (float)(mouse_position.X-m_mouse_capture_pos.X);
		float dy		= -(float)(mouse_position.Y-m_mouse_capture_pos.Y);
		float forward	= (math::abs(dx) > math::abs(dy)) ? dx : dy;
		forward			*= m_scale;

		m_level_editor->get_active_control()->change_property(m_prop_name, forward);
		m_level_editor->ide()->set_mouse_position(m_mouse_capture_pos);
	}
}


} // namespace editor
} // namespace xray
