////////////////////////////////////////////////////////////////////////////
//	Created		: 18.01.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef LEVEL_EDITOR_H_INCLUDED
#define LEVEL_EDITOR_H_INCLUDED

#include "project_defines.h"
#include "editor_ide.h"
#include <xray/editor/world/property_holder.h>

namespace xray {

namespace collision { struct space_partitioning_tree; }
namespace render {
	namespace editor { struct renderer; }
	struct world;
} // namespace render

namespace editor {

class editor_world;
ref class project;
ref class tools_tab;
ref class project_tab;
ref class library_object_tab;
ref class property_connection_editor;
ref class editor_control_base;
ref class object_inspector_tab;
ref class scene_graph_tab;
ref class transform_control_translation;
ref class transform_control_rotation;
ref class transform_control_scaling;
ref class gui_binder;
ref class window_ide;
ref class action_engine;
class picker;

public ref class level_editor : public xray::editor_base::editor_base
{
	typedef xray::editor_base::editor_base super;
	typedef WeifenLuo::WinFormsUI::Docking::IDockContent IDockContent;
public:
							level_editor					( editor_world& world );
	virtual					~level_editor					( );
	virtual void			initialize						( );
	virtual void			destroy							( );
	virtual void			clear_resources					( );
	virtual void			on_render						( );
	virtual void			tick							( );
	virtual bool			close_query						( );
	virtual IDockContent^	find_dock_content				( System::String^ type_name );
	virtual bool			load							( System::String^ name );

	virtual void			load_settings					( );
	virtual void			save_settings					( );

	virtual	void			set_default_layout				( );
	virtual System::String^	name							( ) {return m_name;};
	virtual	void			show							( ) {};

	virtual xray::editor_base::command_engine^	get_command_engine( ) ;
	
	xray::editor::action_engine^		get_action_engine	( );

	void					add_library_object				( float3 position );
	System::String^			project_name					( );
			tool_base^		get_tool						( System::String^ tool_name );
			project^		get_project						( );
			gui_binder^		get_gui_binder					( ) {return m_gui_binder;}
			void			save_internal					( );
			void			tool_loaded						( tool_base^ tool);
			void			new_project						( );
			bool			save_project					( System::String^ name );
			void			close_project					( );
			bool			save_needed						( );

			void			select_all						( );
			void			delete_selected					( );
			void			copy_selected_to_clipboard		( );
			void			cut_selected_to_clipboard		( );
			void			paste_from_clipboard			( );
			void			duplicate_selected				( );
			u32				selected_items_count			( );
			bool			selection_not_empty				( );
			float			focus_distance					( );

			collision::space_partitioning_tree* get_collision_tree			( );
			project_tab^						get_project_explorer		( );
			tools_tab^							get_tools_explorer			( );
			property_connection_editor^			get_connection_editor		( );
			library_object_tab^					get_library_object_tab		( );
			object_inspector_tab^				get_object_inspector_tab	( );

			void			switch_scene_graph				( );
			void			switch_object_inspector			( );
			void			switch_connection_editor		( );
			void			show_project_explorer			( );
			void			show_tools_frame				( );

			void			on_active_control_changed		(editor_control_base^ c);

			void			get_recent_list					(System::Collections::ArrayList^ recent_list);
			void			change_recent_list				(System::String^ path, bool b_add);

			// editor_world wrappers
			editor_ide^		ide					();
			picker&			get_picker					();
			void				get_mouse_ray				(math::float3& origin, math::float3& direction);
			void				get_screen_ray				(math::int2 screen_xy, math::float3& origin, math::float3& direction);
			math::float4x4		get_inverted_view_matrix	();
			math::int2			get_viewport				();
			void				get_camera_props			( float3& p, float3& d);
			void				set_camera_props			( float3 const& p, float3 const& d);

inline	render::editor::renderer&	get_editor_renderer		();
			editor_world&		get_editor_world			( ) {return m_editor_world;}

			editor_control_base^ get_active_control			();
			void 				set_active_control			(System::String^ control_id);
			void				register_editor_control		(editor_control_base^ c);
			void				unregister_editor_control	(editor_control_base^ c);

private:
			editor_world&						m_editor_world;
			System::String^						m_name;
			project^							m_project;
			System::Collections::ArrayList^		m_object_tools;
			tools_tab^							m_tools_tab;
			project_tab^						m_project_tab;
			library_object_tab^					m_library_object_tab;
			property_connection_editor^			m_connection_editor;
			scene_graph_tab^					m_scene_graph_tab;
			object_inspector_tab^				m_object_inspector_tab;

			editor_control_base^				m_active_control;
			System::Collections::ArrayList		m_editor_controls;

			transform_control_translation^		m_transform_control_translation;
			transform_control_scaling^			m_transform_control_scaling;
			transform_control_rotation^			m_transform_control_rotation;
			gui_binder^							m_gui_binder;

			void			snap_selected_down_to_terrain	( float3 const& position );

			void			new_project_action				( );
			void			open_project_action				( );
			void			save_project_action				( );
			void			save_as_project_action			( );
			void			close_project_action			( );

			bool			save_confirmation				( );
			bool			is_clipboard_empty				( );
			bool			object_inspector_visible		( );
			bool			project_explorer_visible		( );
			bool			tools_frame_visible				( );
			bool			connection_editor_visible		( );
			bool			scene_graph_visible				( );
private:
			void			register_actions				( );
}; // class level_editor

}// namespace editor
} // namespace xray

#endif // #ifndef LEVEL_EDITOR_H_INCLUDED