////////////////////////////////////////////////////////////////////////////
//	Created		: 12.02.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_ACTIONS_H_INCLUDED
#define INPUT_ACTIONS_H_INCLUDED

#include "action_continuous.h"
#include "project_defines.h"
#include "editor_world_defines.h"

namespace xray {

namespace collision { struct result; }

namespace editor {

ref class window_ide;
ref class action_engine;
ref class editor_control_base;
ref class level_editor;

class editor_world;
class collision_object_dynamic;
ref class command_snap_objects;
ref class command_place_objects;


public ref class mouse_action_base abstract : public action_continuous
{
protected:
					mouse_action_base		( System::String^ name, editor_world* world );
	virtual			~mouse_action_base		( );


public:
	virtual bool 	capture					( ) override { return false; }
	virtual void	release					( ) override {};

protected:
			void	init_capture			( );

	editor_world*			m_world;

	System::Drawing::Point	m_mouse_last_pos;
	System::Drawing::Size	m_viewport_size;
	math::float4x4*			m_inverted_view_captured;
	
};

//////////////////////////////////////////////////////////////////////////
public ref class mouse_action_view_rotate_base abstract : public mouse_action_base
{
	typedef mouse_action_base		super;
public:

	mouse_action_view_rotate_base	( System::String^ name, editor_world* world ): mouse_action_base( name, world ){};

protected:

	void set_rotated_view_matrix	 ( math::float2 const& raw_angles, math::float3 point );
}; // class mouse_action_view_rotate_base


//////////////////////////////////////////////////////////////////////////
public ref class mouse_action_view_move_xy : public mouse_action_base
{
	typedef mouse_action_base		super;
public:

	mouse_action_view_move_xy		( System::String^ name, editor_world* world );

	virtual bool		capture		() override;
	virtual void		execute		() override;
	virtual void		release		() override;

private:
	enum_editor_cursors m_old_cursor;

}; // class mouse_action_view_move_xy

//////////////////////////////////////////////////////////////////////////
public ref class mouse_action_view_move_z : public mouse_action_base
{
	typedef mouse_action_base		super;
public:

	mouse_action_view_move_z		( System::String^ name, editor_world* world );

	virtual bool		capture		() override;
	virtual void		execute		() override;
	virtual void		release		() override;

private:

	bool m_continuse_drugging;
	enum_editor_cursors m_old_cursor;

}; // class mouse_action_view_move_z


//////////////////////////////////////////////////////////////////////////
// maya rotate (alt+LMB)
public ref class mouse_action_view_rotate_around : public mouse_action_view_rotate_base
{
	typedef mouse_action_view_rotate_base	super;
public:

	mouse_action_view_rotate_around		( System::String^ name, editor_world* world );

	virtual bool		capture			( ) override;
	virtual void		execute			( ) override;
	virtual void		release			( ) override;

private:

	float  m_rotation_origin_x;
	float  m_rotation_origin_y;
	float  m_rotation_origin_z;

	enum_editor_cursors		m_old_cursor;

}; // class mouse_action_view_rotate_around


//////////////////////////////////////////////////////////////////////////
public ref class mouse_action_view_move_y_reverse : public mouse_action_base
{
	typedef mouse_action_base		super;
public:

	mouse_action_view_move_y_reverse		( System::String^ name, editor_world* world );

	virtual bool		capture				( ) override;
	virtual void		execute				( ) override;
	virtual void		release				( ) override;
}; // class mouse_action_view_move_y_reverse

//////////////////////////////////////////////////////////////////////////
public ref class mouse_action_view_move_xz_reverse : public mouse_action_base
{
	typedef mouse_action_base		super;
public:

	mouse_action_view_move_xz_reverse		( System::String^ name, editor_world* world );

	virtual bool		capture				( ) override;
	virtual void		execute				( ) override;
	virtual void		release				( ) override;

}; // class mouse_action_view_move_xz_reverse

//////////////////////////////////////////////////////////////////////////
// (LMB+RMB+Space)
public ref class mouse_action_view_rotate : public mouse_action_view_rotate_base
{
	typedef mouse_action_view_rotate_base	super;
public:

	mouse_action_view_rotate				( System::String^ name, editor_world* world );

	virtual bool		capture				( ) override;
	virtual void		execute				( ) override;
	virtual void		release				( ) override;

private:
	float			m_start_angle_x;
	float			m_shift_x;

}; // class mouse_action_view_rotate


//////////////////////////////////////////////////////////////////////////
public ref class mouse_action_left_button_control : public action_continuous
{
public:

	mouse_action_left_button_control		( System::String^ name, level_editor^ le, bool plane_mode );

	virtual bool		capture				( ) override;
	virtual void		execute				( ) override;
	virtual void		release				( ) override;

private:

	level_editor^			m_level_editor;
	editor_control_base^	m_active_handler;
	bool					m_plane_mode;

}; // class mouse_action_left_button_control


//////////////////////////////////////////////////////////////////////////
public ref class mouse_action_middle_button_control : public action_continuous
{
public:

	mouse_action_middle_button_control		( System::String^ name, level_editor^ le, bool plane_mode );

	virtual bool		capture				( ) override;
	virtual void		execute				( ) override;
	virtual void		release				( ) override;

private:

	level_editor^			m_level_editor;
	editor_control_base^	m_active_handler;
	bool					m_plane_mode;

}; // class mouse_action_middle_button_control


//////////////////////////////////////////////////////////////////////////
public ref class mouse_action_select_object : public action_continuous
{
public:

	mouse_action_select_object		( System::String^ name, level_editor^ le, enum_selection_method method );

	virtual bool		capture		( ) override;
	virtual void		execute		( ) override;
	virtual void		release		( ) override;

private:
	level_editor^					m_level_editor;
	collision_object_dynamic const*	m_collision_object;
	int								m_start_x;
	int								m_start_y;
	enum_selection_method			m_select_method;

}; // class mouse_action_left_button_select


//////////////////////////////////////////////////////////////////////////
public ref class input_action_add_library_object : public action_continuous
{
public:

	input_action_add_library_object		( System::String^ name, level_editor^ le);

	virtual bool		enabled		() override;
	virtual bool		capture		() override;
	virtual void		execute		() override;
	virtual void		release		() override {};

private:
	level_editor^			m_level_editor;
}; // class input_action_add_library_object


//////////////////////////////////////////////////////////////////////////
public ref class key_action_snap : public action_continuous
{
public:

	key_action_snap		( System::String^ name, level_editor^ le);

	virtual bool		capture		() override;
	virtual void		execute		() override;
	virtual void		release		() override;

private:

	level_editor^					m_level_editor;
	command_snap_objects^			m_snap_command;
}; // class key_action_snap

//////////////////////////////////////////////////////////////////////////
public ref class key_action_place : public action_continuous
{
public:

	key_action_place		( System::String^ name, level_editor^ le);

	virtual bool		capture		() override;
	virtual void		execute		() override;
	virtual void		release		() override;

private:

	level_editor^					m_level_editor;
	command_place_objects^			m_snap_command;
}; // class key_action_place


public ref class mouse_action_control_properties : public action_continuous
{
	typedef action_continuous super;

public:
					mouse_action_control_properties	( System::String^ action_name, level_editor^ le, System::String^ prop_name, float scale );
	virtual			~mouse_action_control_properties( );


public:
	virtual bool 	enabled					( ) override;
	virtual bool 	capture					( ) override;
	virtual void	release					( ) override;
	virtual void	execute					( ) override;

protected:
	System::String^			m_prop_name;
	float					m_scale;
	level_editor^			m_level_editor;
	System::Drawing::Point	m_mouse_capture_pos;
};

} // namespace editor
} // namespace xray


#endif // #ifndef INPUT_ACTIONS_H_INCLUDED