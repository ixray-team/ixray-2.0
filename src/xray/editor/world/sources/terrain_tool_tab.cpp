#include "pch.h"
#include "terrain_tool_tab.h"
#include "tool_terrain.h"
#include "library_object_tab.h"
#include "level_editor.h"
#include "project.h"
#include "gui_binder.h"
#include "terrain_object.h"
#include "terrain_modifier_bump.h"
#include "terrain_modifier_raise_lower.h"
#include "terrain_modifier_flatten.h"
#include "terrain_modifier_smooth.h"
#include "terrain_modifier_painter.h"
#include "terrain_painter_control_tab.h"
#include "terrain_core_form.h"

using System::Windows::Forms::ToolStripButton;

namespace xray {
namespace editor {

void terrain_tool_tab::in_constructor()
{
	m_tool->m_terrain_modifier_bump->subscribe_on_activated				(gcnew activate_control_event(this, &terrain_tool_tab::on_control_activated));
	m_tool->m_terrain_modifier_bump->subscribe_on_deactivated			(gcnew deactivate_control_event(this, &terrain_tool_tab::on_control_deactivated));
	m_tool->m_terrain_modifier_bump->subscribe_on_property_changed		(gcnew property_changed(this, &terrain_tool_tab::on_control_property_changed));

	m_tool->m_terrain_modifier_raise_lower->subscribe_on_activated		(gcnew activate_control_event(this, &terrain_tool_tab::on_control_activated));
	m_tool->m_terrain_modifier_raise_lower->subscribe_on_deactivated	(gcnew deactivate_control_event(this, &terrain_tool_tab::on_control_deactivated));
	m_tool->m_terrain_modifier_raise_lower->subscribe_on_property_changed(gcnew property_changed(this, &terrain_tool_tab::on_control_property_changed));

	m_tool->m_terrain_modifier_flatten->subscribe_on_activated			(gcnew activate_control_event(this, &terrain_tool_tab::on_control_activated));
	m_tool->m_terrain_modifier_flatten->subscribe_on_deactivated		(gcnew deactivate_control_event(this, &terrain_tool_tab::on_control_deactivated));
	m_tool->m_terrain_modifier_flatten->subscribe_on_property_changed	(gcnew property_changed(this, &terrain_tool_tab::on_control_property_changed));

	m_tool->m_terrain_modifier_smooth->subscribe_on_activated			(gcnew activate_control_event(this, &terrain_tool_tab::on_control_activated));
	m_tool->m_terrain_modifier_smooth->subscribe_on_deactivated			(gcnew deactivate_control_event(this, &terrain_tool_tab::on_control_deactivated));
	m_tool->m_terrain_modifier_smooth->subscribe_on_property_changed	(gcnew property_changed(this, &terrain_tool_tab::on_control_property_changed));

	m_tool->m_terrain_modifier_detail_painter->subscribe_on_activated	(gcnew activate_control_event(this, &terrain_tool_tab::on_control_activated));
	m_tool->m_terrain_modifier_detail_painter->subscribe_on_deactivated	(gcnew deactivate_control_event(this, &terrain_tool_tab::on_control_deactivated));
	m_tool->m_terrain_modifier_detail_painter->subscribe_on_property_changed(gcnew property_changed(this, &terrain_tool_tab::on_control_property_changed));

	m_tool->m_terrain_modifier_diffuse_painter->subscribe_on_activated	(gcnew activate_control_event(this, &terrain_tool_tab::on_control_activated));
	m_tool->m_terrain_modifier_diffuse_painter->subscribe_on_deactivated	(gcnew deactivate_control_event(this, &terrain_tool_tab::on_control_deactivated));
	m_tool->m_terrain_modifier_diffuse_painter->subscribe_on_property_changed(gcnew property_changed(this, &terrain_tool_tab::on_control_property_changed));

	m_terrain_core_form								= nullptr;
	System::Windows::Forms::UserControl^ form		= nullptr;

	form									= gcnew xray::editor::terrain_painter_control_tab(m_tool);
	control_parameters_panel->Controls->Add	(form);
	form->Hide								();

	ToolStrip^ strip			= top_toolStrip;
	gui_binder^ binder			= m_tool->get_level_editor()->get_gui_binder();
	editor_ide^ ide				= m_tool->ide();

	ToolStripButton^ button		= nullptr;
	System::String^ action_name	= nullptr;

	action_name					= "select terrain bump modifier";
	button						= gcnew ToolStripButton;
	strip->Items->Add			(button);
	binder->bind				(button, action_name);
	button->Image				= ide->get_image(action_name);
	button->DisplayStyle		= System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	button->ImageScaling		= System::Windows::Forms::ToolStripItemImageScaling::SizeToFit;

	action_name					= "select terrain raise-lower modifier";
	button						= gcnew ToolStripButton;
	strip->Items->Add			(button);
	binder->bind				(button, action_name);
	button->Image				= ide->get_image(action_name);
	button->DisplayStyle		= System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	button->ImageScaling		= System::Windows::Forms::ToolStripItemImageScaling::SizeToFit;

	action_name					= "select terrain flatten modifier";
	button						= gcnew ToolStripButton;
	strip->Items->Add			(button);
	binder->bind				(button, action_name);
	button->Image				= ide->get_image(action_name);
	button->DisplayStyle		= System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	button->ImageScaling		= System::Windows::Forms::ToolStripItemImageScaling::SizeToFit;

	action_name					= "select terrain smooth modifier";
	button						= gcnew ToolStripButton;
	strip->Items->Add			(button);
	binder->bind				(button, action_name);
	button->Image				= ide->get_image(action_name);
	button->DisplayStyle		= System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	button->ImageScaling		= System::Windows::Forms::ToolStripItemImageScaling::SizeToFit;

	action_name					= "select terrain detail painter modifier";
	button						= gcnew ToolStripButton;
	strip->Items->Add			(button);
	binder->bind				(button, action_name);
	button->Image				= ide->get_image(action_name);
	button->DisplayStyle		= System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	button->ImageScaling		= System::Windows::Forms::ToolStripItemImageScaling::SizeToFit;

	action_name					= "select terrain diffuse painter modifier";
	button						= gcnew ToolStripButton;
	strip->Items->Add			(button);
	binder->bind				(button, action_name);
	button->Image				= ide->get_image(action_name);
	button->DisplayStyle		= System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	button->ImageScaling		= System::Windows::Forms::ToolStripItemImageScaling::SizeToFit;
	m_sound = NEW (xray::sound::sound_ptr)();

	sound_cb->Items->Add("test_sounds/guitar_10");
	sound_cb->Items->Add("test_sounds/ai1_enemy_lost_0");
	sound_cb->SelectedIndex = 0;
}

void terrain_tool_tab::in_destructor()
{
	*m_sound = 0;
	DELETE(m_sound);
}

void terrain_tool_tab::on_control_activated(xray::editor::editor_control_base^ c)
{
	active_control_property_grid->SelectedObject = c;
}

void terrain_tool_tab::on_control_deactivated(xray::editor::editor_control_base^)
{
	active_control_property_grid->SelectedObject = nullptr;
}

void terrain_tool_tab::on_control_property_changed(xray::editor::editor_control_base^ /*c*/)
{
	active_control_property_grid->Refresh();
}

void terrain_tool_tab::button1_Click_1(System::Object^, System::EventArgs^)
{
	if(!m_terrain_core_form)
		m_terrain_core_form = gcnew terrain_core_form(m_tool);
	
	m_terrain_core_form->Show();
}

void terrain_tool_tab::btn_play_Click(System::Object^, System::EventArgs^)
{
	LOG_INFO("btn_play_Click");
	if((*m_sound).c_ptr())
	{
		(*m_sound)->play		( );
	}else
	{
		System::String^ sound_name = sound_cb->SelectedItem->ToString();
		load_sound		( unmanaged_string(sound_name).c_str() );
	}
}

void terrain_tool_tab::btn_stop_Click(System::Object^, System::EventArgs^)
{
	if((*m_sound).c_ptr())
	{
		LOG_INFO("btn_stop_Click");
		(*m_sound)->stop		( );
		(*m_sound) = 0;
	}
}

void terrain_tool_tab::load_sound( pcstr name )
{
	LOG_INFO("load_sound %s", name);
	query_result_delegate* q = NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &terrain_tool_tab::on_sound_loaded));

	resources::query_resource
	(
		name,
		resources::sound_class,
		boost::bind(&query_result_delegate::callback, q, _1),
		g_allocator
	);
}

void terrain_tool_tab::on_sound_loaded( resources::queries_result& data )
{
	LOG_INFO("on_sound_loaded");
	ASSERT				(data.is_successful());
	*m_sound	= static_cast_resource_ptr<xray::sound::sound_ptr>(data[0].get_unmanaged_resource());
	(*m_sound)->play		( );
}

} // namespace editor
} // namespace xray
