////////////////////////////////////////////////////////////////////////////
//	Created		: 09.04.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "tool_window_storage.h"
//#include "control_properties_tab.h"
#include "sound_editor.h"
#include "particle_editor.h"
#include "resource_editor.h"
#include "texture_document_factory.h"

#include <xray/render/base/world.h>

using xray::editor::tool_window_storage;


void tool_window_storage::initialize(xray::editor::editor_world* editor_world)
{
//	m_control_properties_tab	= gcnew control_properties_tab();
	m_sound_editor				= gcnew sound_editor(*editor_world);
	m_particle_editor			= gcnew particle_editor(*editor_world);
//	m_resource_editor			= gcnew resource_editor(*editor_world, resource_editor_base::resource_type::Texture);
}

void tool_window_storage::destroy()
{
}

void tool_window_storage::on_render(xray::render::world& /*render_world*/)
{
}

WeifenLuo::WinFormsUI::Docking::IDockContent^ tool_window_storage::get_by_typename(System::String^ /*name*/)
{
	//if(name=="xray.editor.control_properties_tab")
	//	return m_control_properties_tab;
	//else 
		return nullptr;
}
