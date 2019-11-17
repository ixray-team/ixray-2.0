////////////////////////////////////////////////////////////////////////////
//	Created		: 18.01.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "level_editor.h"
#include "project.h"
#include "object_base.h"
#include "editor_world.h"
#include "action_engine.h"
#include "tools_tab.h"
#include "library_object_tab.h"
#include "project_tab.h"
#include "scene_graph_tab.h"
#include "object_inspector_tab.h"
#include "property_connection_editor.h"

#include "tool_solid_visual.h"
#include "tool_light.h"
#include "tool_compound.h"
#include "tool_terrain.h"
#include "window_ide.h"

// commands
#include "command_set_object_transform.h"
#include "transform_control_translation.h"
#include "transform_control_scaling.h"
#include "transform_control_rotation.h"

#include "gui_binder.h"

RegistryKey^ get_sub_key(RegistryKey^ root, System::String^ name)
{
	RegistryKey^ result = root->OpenSubKey(name, true);
	if(!result)
		result			= root->CreateSubKey(name);

	return result;
}

namespace xray {
namespace editor {

level_editor::level_editor(editor_world& world)
:m_editor_world(world),
m_object_tools( gcnew System::Collections::ArrayList), 
m_active_control(nullptr)
{
	m_name = "level_editor";
}

level_editor::~level_editor()
{}

xray::editor_base::command_engine^ level_editor::get_command_engine()
{
	return get_editor_world().command_engine();
}

xray::editor::action_engine^ level_editor::get_action_engine()
{
	return get_editor_world().action_engine();
}

void level_editor::initialize()
{
	m_gui_binder				= gcnew gui_binder(dynamic_cast<xray::editor::action_engine^>(get_action_engine())); 

	m_library_object_tab		= gcnew library_object_tab(this);
	m_tools_tab					= gcnew tools_tab;
	m_project_tab				= gcnew project_tab(this);
	m_object_inspector_tab		= gcnew object_inspector_tab;

	m_project					= gcnew project(this);

	tool_solid_visual^ tool_sv	= gcnew tool_solid_visual(this);
	m_tools_tab->add_tab		("solid visual", tool_sv->ui()->frame());
	m_object_tools->Add			(tool_sv);

	tool_light^ p_tool_light	= gcnew tool_light(this);
	m_tools_tab->add_tab		("lights", p_tool_light->ui()->frame());
	m_object_tools->Add			(p_tool_light);

	tool_compound^ p_tool_compound= gcnew tool_compound(this);
	m_tools_tab->add_tab		("compound", p_tool_compound->ui()->frame());
	m_object_tools->Add			(p_tool_compound);

	tool_terrain^ p_tool_terrain= gcnew tool_terrain(this);
	m_tools_tab->add_tab		("terrain", p_tool_terrain->ui()->frame());
	m_object_tools->Add			(p_tool_terrain);

// Initializing standard set of transformation controls
	m_transform_control_translation		= gcnew transform_control_translation(this);
	m_transform_control_scaling			= gcnew transform_control_scaling(this);
	m_transform_control_rotation		= gcnew transform_control_rotation(this);

	register_editor_control				(m_transform_control_translation);
	register_editor_control				(m_transform_control_scaling);
	register_editor_control				(m_transform_control_rotation);

	register_actions					();

	m_project->clear					();
	m_connection_editor					= gcnew property_connection_editor(this);
	m_scene_graph_tab					= gcnew scene_graph_tab(this);

	m_transform_control_translation->initialize();
	m_transform_control_scaling->initialize();
	m_transform_control_rotation->initialize();
}

void level_editor::clear_resources	( )
{
	close_project					();
	unregister_editor_control		(m_transform_control_translation);
	unregister_editor_control		(m_transform_control_scaling);
	unregister_editor_control		(m_transform_control_rotation);

	for each (tool_base^ tool in m_object_tools)
		tool->save_library	();
}

void level_editor::destroy()
{
	delete m_transform_control_translation;
	delete m_transform_control_scaling;
	delete m_transform_control_rotation;

	for each (tool_base^ tool in m_object_tools)
		delete				tool;

	m_object_tools->Clear	( );
	delete					m_library_object_tab;
	delete					m_scene_graph_tab;
	delete					m_connection_editor;
	delete					m_project;
}

tool_base^ level_editor::get_tool(System::String^ tool_name)
{
	for(int i=0; i<m_object_tools->Count; ++i)
	{
		tool_base^ tool = safe_cast<tool_base^>(m_object_tools->default[i]);
		if(tool->name()==tool_name)
			return tool;
	}

	UNREACHABLE_CODE(return nullptr);
}

void level_editor::on_render()
{
	m_project->render				();
	m_library_object_tab->on_render	();

	if(get_active_control())
		get_active_control()->draw	(get_inverted_view_matrix());
}

void level_editor::tick()
{
	m_gui_binder->update_items	();
	for each (tool_base^ t in m_object_tools)
		t->tick();

	if( get_active_control() )
		get_active_control()->update	( );
}

void level_editor::tool_loaded(tool_base^ )
{
	for each(tool_base^ t in m_object_tools)
		if(!t->ready())
			return;

	new_project();
}

WeifenLuo::WinFormsUI::Docking::IDockContent^ level_editor::find_dock_content(System::String^ type_name)
{
	IDockContent^ result = nullptr;

	if(type_name=="xray.editor.project_tab")
	{
		result = m_project_tab;
	}else
	if(type_name=="xray.editor.tools_tab")
	{
		result = m_tools_tab;
	}else
	if(type_name=="xray.editor.object_inspector_tab")
	{
		result = m_object_inspector_tab;
	}else

	if(result==nullptr)
	{
		unmanaged_string s(type_name);
		LOG_INFO("frame not found %s", s.c_str());
	
	}
	return result;
}

void level_editor::load_settings()
{
	RegistryKey^ product_key = ide()->base_registry_key();
	RegistryKey^ editor_key = get_sub_key(product_key, m_name);

	for each(tool_base^ tool in m_object_tools)
		tool->load_settings	(editor_key);

	m_transform_control_translation->load_settings	(editor_key);
	m_transform_control_scaling->load_settings		(editor_key);
	m_transform_control_rotation->load_settings		(editor_key);

	editor_key->Close		();
	product_key->Close		();
}

void level_editor::save_settings()
{
	RegistryKey^ product_key = ide()->base_registry_key(); 
	RegistryKey^ editor_key = get_sub_key(product_key, m_name);

	for each(tool_base^ tool in m_object_tools)
		tool->save_settings	(editor_key);

	m_transform_control_translation->save_settings	(editor_key);
	m_transform_control_scaling->save_settings		(editor_key);
	m_transform_control_rotation->save_settings		(editor_key);

	editor_key->Close		();
	product_key->Close		();
}


System::String^ level_editor::project_name( )
{
	return m_project->project_name( );
}

void level_editor::set_default_layout	( )
{
	ide()->show_tab			( m_project_tab, WeifenLuo::WinFormsUI::Docking::DockState::DockLeft );
	ide()->show_tab			( m_tools_tab, WeifenLuo::WinFormsUI::Docking::DockState::DockRight );
	m_object_inspector_tab->Show( m_project_tab->Pane, WeifenLuo::WinFormsUI::Docking::DockAlignment::Bottom, .25f );
}

float level_editor::focus_distance( )
{
	return m_editor_world.focus_distance();
}


picker& level_editor::get_picker()
{
	return get_editor_world().get_picker();
}

void level_editor::get_mouse_ray(float3& origin, float3& direction)
{
	return get_editor_world().get_mouse_ray(origin, direction);
}

void level_editor::get_screen_ray(xray::math::int2 screen_xy, float3& origin, float3& direction)
{
	return get_editor_world().get_screen_ray(screen_xy, origin, direction);
}

xray::math::int2 level_editor::get_viewport()
{
	return get_editor_world().get_viewport();
}

float4x4 level_editor::get_inverted_view_matrix()
{
	return get_editor_world().get_inverted_view_matrix();
}

void level_editor::on_active_control_changed(editor_control_base^ c)
{
	get_editor_world().on_active_control_changed(c);
}

xray::render::editor::renderer& level_editor::get_editor_renderer()
{
	return get_editor_world().get_renderer();
}

editor_ide^ level_editor::ide()
{
	return get_editor_world().ide();
}

void level_editor::get_camera_props(float3& p, float3& d)
{
	get_editor_world().get_camera_props(p, d);
}

void level_editor::set_camera_props(float3 const& p, float3 const& d)
{
	get_editor_world().set_camera_props(p, d);
}

void level_editor::register_editor_control(editor_control_base^ c)
{
	ASSERT(!m_editor_controls.Contains(c));
	m_editor_controls.Add(c);
}

void level_editor::unregister_editor_control(editor_control_base^ c)
{
	ASSERT(m_editor_controls.Contains(c));
	m_editor_controls.Remove(c);
}

editor_control_base^ level_editor::get_active_control()
{
	return m_active_control;
}

void level_editor::set_active_control(System::String^ control_id)
{
	editor_control_base^ result		= nullptr;
	if(control_id!=nullptr)
	{
		if(m_active_control && m_active_control->id()==control_id)
			return;

		for each (editor_control_base^ e in m_editor_controls)
		{
			if(e->id() == control_id)
			{
				result = e;
				break;
			}
		}

		ASSERT( (result!=nullptr), "The control is not registered !" );
	}

	if( m_active_control )
	{
		m_active_control->activate	(false);
		m_active_control->show		(false);
	}

	m_active_control = result;

	if(m_active_control)
		m_active_control->activate	(true);

	on_active_control_changed(m_active_control);
}

#define _recent_list_prefix		"Line"
#define _recent_key_name		"recent files"

Void level_editor::get_recent_list(ArrayList^ recent_list)
{
	recent_list->Clear			();

	RegistryKey^ product_key	= ide()->base_registry_key();
	ASSERT						(product_key);

	RegistryKey^ editor_key		= get_sub_key(product_key, m_name);
	RegistryKey^ recent_key		= get_sub_key(editor_key, _recent_key_name);

	array<String^>^	value_names	= recent_key->GetValueNames();

	for each(System::String^ name in value_names)
	{
		if( !name->StartsWith(_recent_list_prefix) )
			continue; 

		String^ tmp_value	=	dynamic_cast<String^>(recent_key->GetValue(name));
		if( tmp_value != nullptr )
			recent_list->Add	(tmp_value);
	}

	recent_key->Close		();
	editor_key->Close		();
	product_key->Close		();
}

Void level_editor::change_recent_list(System::String^ path, bool b_add)
{
	ArrayList					recent_list;
	get_recent_list				(%recent_list);

	if(recent_list.Contains(path))
		recent_list.Remove		(path);

	if(b_add)
		recent_list.Insert			(0, path);

	// save

	RegistryKey^ product_key	= ide()->base_registry_key();
	ASSERT						(product_key);

	RegistryKey^ editor_key		= get_sub_key(product_key, m_name);

	editor_key->DeleteSubKey	(_recent_key_name, false);

	RegistryKey^ recent_key		= editor_key->CreateSubKey(_recent_key_name);
	ASSERT						(recent_key);

	int i=0;
	for each(System::String^ path in recent_list)
	{
		ASSERT					(path);
		recent_key->SetValue	(_recent_list_prefix + (i++), path);
	}

	recent_key->Close			();
	editor_key->Close			();
	product_key->Close			();
}

} // namespace editor
} // namespace xray
