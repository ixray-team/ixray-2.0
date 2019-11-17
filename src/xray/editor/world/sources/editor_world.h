////////////////////////////////////////////////////////////////////////////
//	Created 	: 31.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef EDITOR_WORLD_H_INCLUDED
#define EDITOR_WORLD_H_INCLUDED

#include <xray/editor/world/world.h>
#include "flying_camera.h"
#include "picker.h"
#include "tool_window_storage.h"
#include "project_defines.h"
#include "editor_world_defines.h"
#include "editor_control_base.h"
#include "property_holder.h"

using System::Collections::ArrayList;

namespace xray {

namespace collision { struct space_partitioning_tree; }

namespace render {
namespace editor {
	struct renderer;
} // namespace editor
} // namespace render

namespace resources { class queries_result; }
namespace editor_base { 
	interface class editor_base; 
	ref class command_engine;
}

namespace editor {

ref class window_ide;
ref class input_action;
ref class input_engine;
ref class action_engine;
ref class gui_binder;
ref class transform_control_translation;
ref class transform_control_scaling;
ref class transform_control_rotation;
//ref class command_engine;

class flying_camera;

struct camera_properties
{
	float 								view_point_distance;
	float 								view_point_distance_min;
	float 								view_move_z_coeff;
	float 								view_move_speed_factor;
};

class editor_world :
	public xray::editor::world,
	private boost::noncopyable
{
private:
	typedef WeifenLuo::WinFormsUI::Docking::IDockContent IDockContent;

public:
								editor_world			( engine& engine );
								~editor_world			(  );

public:
	virtual	void				load					( );
	virtual	void				run						( );
	virtual	void				clear_resources			( );
	virtual	HWND				main_handle				( );
	virtual	HWND				view_handle				( );

			void				on_active_control_changed( editor_control_base^ c );
	virtual	void				tick					( );
			void				render					( u32 frame_id, xray::math::float4x4 view_matrix );
			void				render_grid				( );
			void				set_default_layout		( );

	inline	render::editor::renderer&		get_renderer			( );

	inline	math::float4x4		get_inverted_view_matrix( );
	inline	void				set_view_matrix			( math::float4x4 const& matrix );
			void				set_view_matrix			( math::float2 const& raw_angles, float const forward, float const right, float const up );

	inline	math::float4x4		get_projection_matrix	( );
	inline	void				set_projection_matrix	( math::float4x4 const& matrix );

	flying_camera &				get_flying_camera		( ) { return *m_flying_camera; };
	IDockContent^				find_dock_content		( System::String^ type_name );
			void				get_camera_props		( float3& p, float3& d);
			void				set_camera_props		( float3 const& p, float3 const& d);
			float&				focus_distance			( ) { return m_object_focus_distance; }

public:
			void				on_idle_start			( );
			void				on_idle					( );
			void				on_idle_end				( );
	inline	engine&				engine					( );
	inline	gcroot<window_ide^>	ide						( ) const;

public:
			void				load_bkg_sound			( pcstr name );
			void				on_bkg_sound_loaded		( resources::queries_result& data );
			void				bkg_sound_play			( );
			void				bkg_sound_stop			( );

			camera_properties&	get_camera_props		( );
			math::float3		screen_to_3d			( math::float2 const& screen, float z);
			math::float3		screen_to_3d_world		( math::float2 const& screen, float z);
			void				get_screen_ray			( math::int2 screen_xy, math::float3& origin, math::float3& direction );
			void				get_mouse_ray			( math::float3& origin, math::float3& direction );
			math::int2			get_mouse_position		( );
			math::int2			get_viewport			( );
			enum_editor_cursors get_view_cursor			( );
			enum_editor_cursors set_view_cursor			( enum_editor_cursors cursor );
			

			picker&				get_picker				( );
	

	inline	xray::editor_base::editor_base^	get_active_editor	( ) {return m_active_editor;}
			xray::editor_base::editor_base^	get_editor_by_name	( System::String^ editor_name );

public:
			void				show_sound_editor				( );
			void				show_dialog_editor				( );
			void				show_particle_editor			( );
			void				show_texture_editor				( );

	xray::editor_base::command_engine^	command_engine			( ) { return m_command_engine; }
			gcroot<action_engine^>	action_engine				( ) const { return m_action_engine; }
			gcroot<input_engine^>	input_engine				( ) const { return m_input_engine; }
			

public:
			bool				undo_stack_empty				( );
			bool				redo_stack_empty				( );
			void				on_window_closing				( );

			int					rollback						( int steps );

			void				pause							( bool bpause);
			bool				paused							( ) const { return m_paused; };

	virtual	void				editor_mode						( bool beditor_mode );
			bool				editor_mode						( ) const { return m_editor_mode; }
			bool				close_query						( );

public:
	collision::space_partitioning_tree*	get_collision_tree	( );

private:		
			bool				execute_input				( );

			void				create_collision_tree		( );
			void				register_actions			( );

private:
			void 				set_active_control_impl		( editor_control_base^ control );
			void				load_editors				( );
			void				load_settings				( );
			void				save_settings				( );

private:
	xray::editor::engine&				m_engine;
	gcroot<window_ide^>					m_window_ide;
	typedef System::Collections::Generic::List<xray::editor_base::editor_base^> editors_list;
	gcroot<editors_list^>				m_editors;
	gcroot<xray::editor_base::editor_base^>	m_active_editor;
public:	
	tool_window_storage					m_tool_windows;
	bool								m_b_draw_grid;
	bool								m_b_draw_collision;
	math::float2&						near_far_plane		() { return m_near_far_plane; }
private:

	math::int2							m_viewport_size;
	math::float2						m_near_far_plane;
	float								m_object_focus_distance;

	float4x4							m_inverted_view;
	float4x4							m_projection;
	
	
	gcroot<xray::editor::input_action^>	m_active_action;
	
	collision::space_partitioning_tree*	m_collision_tree;

	camera_properties					m_camera_props;

	bool								m_editor_mode;
	bool								m_paused;
	
	picker*								m_picker;
	flying_camera*						m_flying_camera;
	gcroot<xray::editor_base::command_engine^> m_command_engine;
	gcroot<editor::input_engine^>		m_input_engine;			
	gcroot<editor::action_engine^>		m_action_engine;	
	gcroot<gui_binder^>					m_gui_binder;
}; // class world
} // namespace editor
} // namespace xray

#include "editor_world_inline.h"

#endif // #ifndef EDITOR_WORLD_H_INCLUDED