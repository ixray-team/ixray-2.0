////////////////////////////////////////////////////////////////////////////
//	Created		: 22.01.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "level_editor.h"
#include "project.h"
#include "scene_graph_tab.h"
#include "object_inspector_tab.h"
#include "property_connection_editor.h"
#include "project_tab.h"
#include "tools_tab.h"
#include "tool_base.h"
#include "object_base.h"
#include "command_add_library_object.h"
#include "command_paste_objects.h"
#include "command_delete_object.h"
#include "command_select.h"
#include "action_engine.h"
#include "actions.h"
#include "input_actions.h"
#include "gui_binder.h"
#include <xray/editor/base/images/images16x16.h>

#include "project_browser_dialog.h"

#pragma managed(push)
#include "editor_world.h"
#include <xray/strings_stream.h>
#include <xray/fs_path.h>
#pragma managed(pop)

using xray::editor::level_editor;

void level_editor::register_actions()
{
	// Focuses view to the selected object(s)
	get_action_engine()->register_action( gcnew action_focus( "focus",	this ), "F" );

	get_action_engine()->register_action( gcnew action_select_editor_control("select none",			this, nullptr), "Escape");
	get_action_engine()->register_action( gcnew action_select_editor_control("select translate",	this, "translation" ),	"W");
	get_action_engine()->register_action( gcnew action_select_editor_control("select scale",		this, "scale" ),		"R");
	get_action_engine()->register_action( gcnew action_select_editor_control("select rotate",		this, "rotation" ),		"E");

	// Transform controls mouse actions
	get_action_engine()->register_action( gcnew	mouse_action_left_button_control	( "manipulator drag axis mode",		this, false )	, "LButton" );
	get_action_engine()->register_action( gcnew	mouse_action_left_button_control	( "manipulator drag plane mode",	this, true)		, "Control+LButton" );
	get_action_engine()->register_action( gcnew	mouse_action_middle_button_control	( "manipulator free drag axis mode",this, false )	, "MButton" );
	get_action_engine()->register_action( gcnew	mouse_action_middle_button_control	( "manipulator free drag plane mode",this, true )	, "Control+MButton" );

	get_action_engine()->register_action( gcnew mouse_action_control_properties	( "change control radius",	this, "radius", 0.05f ), "X" );
	get_action_engine()->register_action( gcnew mouse_action_control_properties	( "change control hardness",	this, "hardness", 0.005f ), "C" );
	get_action_engine()->register_action( gcnew mouse_action_control_properties	( "change control strength",	this, "strength", 0.05f ), "Z" );

	// Object select mouse action
	get_action_engine()->register_action( gcnew	mouse_action_select_object	( "select",			this, enum_selection_method_set )		, "LButton" );
	get_action_engine()->register_action( gcnew	mouse_action_select_object	( "select invert",	this, enum_selection_method_invert )	, "Shift+LButton" );
	get_action_engine()->register_action( gcnew	mouse_action_select_object	( "select subtract", this, enum_selection_method_subtract ), "Control+LButton" );
	get_action_engine()->register_action( gcnew	mouse_action_select_object	( "select add",		this, enum_selection_method_add )		, "Control+Shift+LButton" );

	action_delegate^ a					= gcnew action_delegate("select all", gcnew execute_delegate_managed(this, &level_editor::select_all) );
	a->set_enabled						(gcnew enabled_delegate_managed(this, &level_editor::selection_not_empty));
	get_action_engine()->register_action( a );

	a									= gcnew action_delegate("duplicate", gcnew execute_delegate_managed(this, &level_editor::duplicate_selected) );
	a->set_enabled						(gcnew enabled_delegate_managed(this, &level_editor::selection_not_empty));
	get_action_engine()->register_action( a );

	a				= gcnew action_delegate("delete", gcnew execute_delegate_managed(this, &level_editor::delete_selected));
	a->set_enabled(gcnew enabled_delegate_managed(this, &level_editor::selection_not_empty));
	get_action_engine()->register_action( a, "Delete" );

	a				= gcnew action_delegate("copy", gcnew execute_delegate_managed(this, &level_editor::copy_selected_to_clipboard));
	a->set_enabled(gcnew enabled_delegate_managed(this, &level_editor::selection_not_empty));
	get_action_engine()->register_action( a, "Control+C" );


	a				= gcnew action_delegate("cut", gcnew execute_delegate_managed(this, &level_editor::cut_selected_to_clipboard));
	a->set_enabled(gcnew enabled_delegate_managed(this, &level_editor::selection_not_empty));
	get_action_engine()->register_action( a, "Control+X" );

	a				= gcnew action_delegate("paste", gcnew execute_delegate_managed(this, &level_editor::paste_from_clipboard));
	a->set_enabled(gcnew enabled_delegate_managed(this, &level_editor::selection_not_empty));
	get_action_engine()->register_action( a, "Control+V" );

 	// Add library object to the scene.
 	get_action_engine()->register_action( gcnew	input_action_add_library_object( "add library object", this ), "A+LButton", "A, LButton" );

	// Snaps objects to others 
	get_action_engine()->register_action( gcnew key_action_snap( "snap objects", this ), "X+RButton" );

	// Places group of objects to the position pointed by the user.
	get_action_engine()->register_action( gcnew key_action_place( "place objects",this ), "X+LButton" );

	// Actions to work with project file 
	get_action_engine()->register_action( gcnew action_delegate("new project",
																gcnew  execute_delegate_managed(this, &level_editor::new_project_action) ), 
										"Control+N" );


	a				= gcnew action_delegate("save project", gcnew execute_delegate_managed(this, &level_editor::save_project_action));
	a->set_enabled(gcnew enabled_delegate_managed(this, &level_editor::save_needed));
	get_action_engine()->register_action( a, "Control+S" );


	get_action_engine()->register_action(  gcnew action_delegate("save project as",
																gcnew  execute_delegate_managed(this, &level_editor::save_as_project_action) ) );

	get_action_engine()->register_action(  gcnew action_delegate("open project",
																gcnew  execute_delegate_managed(this, &level_editor::open_project_action) ), 
										"Control+O" );

	a				= gcnew action_delegate("close project", gcnew execute_delegate_managed(this, &level_editor::close_project_action));
	a->set_enabled(gcnew enabled_delegate_managed(this, &level_editor::save_needed));
	get_action_engine()->register_action( a );

	// Drops object from the current position
	get_action_engine()->register_action( gcnew action_drop_objects	( "drop",		this, false ), "X+C" );

	// Drops object from the top
	get_action_engine()->register_action( gcnew action_drop_objects	( "drop top",	this, true ), "C" );

	a				= gcnew action_delegate("object inspector", gcnew execute_delegate_managed(this, &level_editor::switch_object_inspector));
	a->set_checked	(gcnew checked_delegate_managed(this, &level_editor::object_inspector_visible));
	
	get_action_engine()->register_action( a );

	a				= gcnew action_delegate("connection editor", gcnew execute_delegate_managed(this, &level_editor::switch_connection_editor));
	a->set_checked	(gcnew checked_delegate_managed(this, &level_editor::connection_editor_visible));
	get_action_engine()->register_action( a );

	a				= gcnew action_delegate("scene graph", gcnew execute_delegate_managed(this, &level_editor::switch_scene_graph));
	a->set_checked	(gcnew checked_delegate_managed(this, &level_editor::scene_graph_visible));
	get_action_engine()->register_action( a );

	a				= gcnew action_delegate("project explorer", gcnew execute_delegate_managed(this, &level_editor::show_project_explorer));
	a->set_checked	(gcnew checked_delegate_managed(this, &level_editor::project_explorer_visible));
	get_action_engine()->register_action( a );

	a				= gcnew action_delegate("tools frame", gcnew execute_delegate_managed(this, &level_editor::show_tools_frame));
	a->set_checked	(gcnew checked_delegate_managed(this, &level_editor::tools_frame_visible));
	get_action_engine()->register_action( a );

	ide()->add_menu_item("ProjectMenuItem", "Project", 1);
	ide()->add_menu_item(m_gui_binder, "new project",		"ProjectMenuItem", 0);
	ide()->add_menu_item(m_gui_binder, "open project",		"ProjectMenuItem", 1);
	ide()->add_menu_item(m_gui_binder, "save project",		"ProjectMenuItem", 2);
	ide()->add_menu_item(m_gui_binder, "save project as",	"ProjectMenuItem", 3);
	ide()->add_menu_item(m_gui_binder, "close project",		"ProjectMenuItem", 4);

	ide()->add_menu_item(m_gui_binder, "object inspector",	"ViewMenuItem", 10);
	ide()->add_menu_item(m_gui_binder, "connection editor",	"ViewMenuItem", 11);
	ide()->add_menu_item(m_gui_binder, "scene graph",		"ViewMenuItem", 12);
	ide()->add_menu_item(m_gui_binder, "project explorer",	"ViewMenuItem", 13);
	ide()->add_menu_item(m_gui_binder, "tools frame",		"ViewMenuItem", 14);

	ide()->add_menu_item(m_gui_binder, "duplicate",			"EditMenuItem", 10);
	ide()->add_menu_item(m_gui_binder, "cut",				"EditMenuItem", 11);
	ide()->add_menu_item(m_gui_binder, "copy",				"EditMenuItem", 12);
	ide()->add_menu_item(m_gui_binder, "paste",				"EditMenuItem", 13);
	ide()->add_menu_item(m_gui_binder, "delete",			"EditMenuItem", 14);
	ide()->add_menu_item(m_gui_binder, "select all",		"EditMenuItem", 15);

	ide()->register_image("select none",		xray::editor_base::tool_select );
	ide()->register_image("select translate",	xray::editor_base::tool_translate );
	ide()->register_image("select rotate",		xray::editor_base::tool_rotate );
	ide()->register_image("select scale",		xray::editor_base::tool_scale );
	
	ide()->add_button_item("ToolToolStrip", 1);
	ide()->add_button_item(m_gui_binder, "select none", "ToolToolStrip", 0);
	ide()->add_button_item(m_gui_binder, "select translate", "ToolToolStrip", 1);
	ide()->add_button_item(m_gui_binder, "select rotate", "ToolToolStrip", 2);
	ide()->add_button_item(m_gui_binder, "select scale", "ToolToolStrip", 3);
}

void level_editor::add_library_object( float3 position )
{
	int activeTab_index		= m_tools_tab->selected_tab_index();
	tool_base^ tool			= safe_cast<tool_base^>(m_object_tools->default[activeTab_index]);
	if( activeTab_index >= 0 && tool->get_selected_library_name() )
		get_command_engine()->run( gcnew command_add_library_object ( this, tool, tool->get_selected_library_name(), create_translation(position), true ) );
}

void level_editor::copy_selected_to_clipboard()
{
	object_list^ objects = get_project()->selection_list();

	if( objects->Count )
	{
		configs::lua_config_ptr cfg = configs::create_lua_config( );

		get_project()->save_to_config( cfg->get_root(), objects, false );

		xray::strings::stream stream(g_allocator);
		cfg->save			( stream );
		stream.append		( "%c", 0 );

		xray::os::copy_to_clipboard	( (pcstr)stream.get_buffer() );
	}
}

void level_editor::paste_from_clipboard()
{
	pstr string				= xray::os::paste_from_clipboard( *g_allocator );
	// clipboard doesn't contain text information
	if ( !string ) {
		// we could use function xray::os::is_clipboard_empty() to check
		// whether there non-text information and show some message in that case
		return;
	}

	configs::lua_config_ptr config = xray::configs::create_lua_config_from_string( string );
	FREE					( string );
	
	// clipboard does contain text information
	if ( !config ) {
		// but there are no valid lua script in clipboard
		// we could show message about this
		return;
	}

	if ( config->empty() ) {
		// we could show message about this
		return;
	}

	get_command_engine()->run	( gcnew command_paste_objects(this, config->get_root()) );
}

void level_editor::snap_selected_down_to_terrain	( float3 const& position )
{
	object_list^ objects = m_project->selection_list();
	
	math::aabb bbox;
	for each (object_base^ obj in objects)
		bbox.modify( obj->aabb() );
	
	float3 trans_vector = position - bbox.center();
	trans_vector.y = 0;
}

void level_editor::new_project( )
{
	m_project->create_new		();
	get_command_engine()->set_saved_flag();
}

bool level_editor::load(System::String^ name)
{
	if( !save_confirmation		( ) )
		return false;

	close_project		();

	set_active_control	( nullptr );

	m_project->load		(name);

	get_command_engine()->set_saved_flag();
	change_recent_list	(name, true);
	return				true;
}

bool level_editor::save_project(System::String^ name)
{
	m_project->save			( name );
	change_recent_list		( name, true );
	
	get_command_engine()->set_saved_flag();
	
	return true;
}

void level_editor::close_project()
{
	m_project->clear				();
	get_command_engine()->clear_stack();
}

void level_editor::delete_selected()
{
	get_command_engine()->run( gcnew command_delete_selected_objects( this ));
}

void level_editor::cut_selected_to_clipboard()
{
	copy_selected_to_clipboard	( );
	delete_selected				( );
}

void level_editor::duplicate_selected				( )
{
	object_list^ objects = m_project->selection_list();

	if( 0==objects->Count )
		return;

	configs::lua_config_ptr cfg		= configs::create_lua_config( );
	m_project->save_to_config		(cfg->get_root(), objects, false );
	get_command_engine()->run		( gcnew command_paste_objects(this, cfg->get_root()));
}

void level_editor::select_all()
{
	object_list^ objects =	m_project->get_all_objects();
	if(objects->Count>0)
		get_command_engine()->run( gcnew command_select( this, objects, enum_selection_method_set ) );
}

bool level_editor::save_needed( )
{
	return ( !m_project->empty( ) &&  !get_command_engine()->saved_flag( ) );
}

void level_editor::save_internal( )
{
	m_project->save_internal( );
}

xray::editor::object_inspector_tab^  level_editor::get_object_inspector_tab()
{
	return m_object_inspector_tab;
}

xray::editor::library_object_tab^ level_editor::get_library_object_tab()
{
	return m_library_object_tab;
}

xray::editor::property_connection_editor^ level_editor::get_connection_editor()
{
	return m_connection_editor;
}

xray::editor::tools_tab^ level_editor::get_tools_explorer()
{
	return m_tools_tab;
}

xray::editor::project_tab^ level_editor::get_project_explorer()
{
	return m_project_tab;
}

xray::collision::space_partitioning_tree* level_editor::get_collision_tree()
{
	return get_editor_world().get_collision_tree();
}


xray::editor::project^ level_editor::get_project()
{
	return m_project;
}

u32 level_editor::selected_items_count()
{
	return get_project()->selection_list()->Count;
}

Void level_editor::new_project_action()
{
	if( !save_confirmation		( ) )
		return;

	if(!m_project->empty())
		m_project->clear();

	new_project			();
}

bool level_editor::close_query()
{
	return save_confirmation();
}

Void level_editor::open_project_action()
{
	project_browser_dialog	dlg( ide() );

	fs::path_string resource_path	= get_editor_world().engine().get_resource_path();
	System::String^ resource_path_m	= gcnew System::String( resource_path.c_str() );

	if(dlg.SelectProjectToLoad( resource_path_m, m_project->project_name() ) == ::DialogResult::OK)
	{
		ASSERT							( dlg.selected_project );
		load							( dlg.selected_project );
	}
}

Void level_editor::save_project_action ()
{
	if( m_project->name_assigned( ) )
		save_project( m_project->project_name() );
	else
		save_as_project_action( );
}

Void level_editor::save_as_project_action()
{
	project_browser_dialog	dlg( ide() );

	fs::path_string resource_path	= get_editor_world().engine().get_resource_path();
	System::String^ resource_path_m	= gcnew System::String( resource_path.c_str() );

	if(dlg.SelectProjectToSave( resource_path_m, m_project->project_name() ) == ::DialogResult::OK)
	{
		ASSERT							( dlg.selected_project );
		save_project					( dlg.selected_project );
	}
}

Void level_editor::close_project_action()
{
	if( !save_confirmation		( ) )
		return;

	close_project		( );
}

bool level_editor::save_confirmation()
{
	if( save_needed() )
	{
		::DialogResult result = MessageBox::Show( ide()->wnd(), "Do you want to save changes in '" 
														+ IO::Path::GetFileNameWithoutExtension ( m_project->project_name() )
														+ "' project ?", "XRay Editor"
														, MessageBoxButtons::YesNoCancel
														, MessageBoxIcon::Exclamation );

		if ( result == ::DialogResult::Yes )
			save_project_action();
		else 
			if ( result == ::DialogResult::Cancel )
				return false;

	}
	return true;
}

bool level_editor::selection_not_empty()
{
	return selected_items_count()!=0;
}

bool level_editor::is_clipboard_empty()
{
	return xray::os::is_clipboard_empty();
}

//---------------
void level_editor::switch_scene_graph()
{
	if(!scene_graph_visible())
	{
		m_scene_graph_tab->initialize	();
		ide()->show_tab					(m_scene_graph_tab);
	}else
		m_scene_graph_tab->Hide();
}
bool level_editor::scene_graph_visible()
{
	return m_scene_graph_tab->Visible;
}

//---------------
void level_editor::switch_object_inspector()
{
	if(!object_inspector_visible())
		get_project()->show_object_inspector();
	else
		get_object_inspector_tab()->Hide();
}
bool level_editor::object_inspector_visible()
{
	return get_object_inspector_tab()->Visible;
}

//---------------
void level_editor::show_project_explorer()
{
	if(!project_explorer_visible())
		ide()->show_tab				(m_project_tab);
	else
		m_project_tab->Hide			();
}
bool level_editor::project_explorer_visible()
{
	return get_project_explorer()->Visible;
}

//------------------
void level_editor::switch_connection_editor()
{
	if(!connection_editor_visible())
		get_project()->show_connection_editor();
	else
		get_connection_editor()->Hide();
}

bool level_editor::connection_editor_visible()
{
	return get_connection_editor()->Visible;
}

//------------------
void level_editor::show_tools_frame()
{
	if(!m_tools_tab->Visible)
		ide()->show_tab				(m_tools_tab);
	else
		m_tools_tab->Hide			();
}
bool level_editor::tools_frame_visible()
{
	return get_tools_explorer()->Visible;
}
