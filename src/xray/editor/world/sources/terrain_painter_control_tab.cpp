////////////////////////////////////////////////////////////////////////////
//	Created		: 14.01.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_painter_control_tab.h"
#include "terrain_modifier_painter.h"
#include "tool_terrain.h"
#include "terrain_object.h"
#include "resource_selector.h"
#include "level_editor.h"
#include "texture_options.h"

using xray::editor::terrain_painter_control_tab;
using xray::editor::terrain_modifier_detail_painter;

void terrain_painter_control_tab::in_constructor()
{
	terrain_modifier_detail_painter^ control	= m_tool->m_terrain_modifier_detail_painter;
	control->subscribe_on_activated			(gcnew activate_control_event(this, &terrain_painter_control_tab::on_control_activated));
	control->subscribe_on_deactivated		(gcnew deactivate_control_event(this, &terrain_painter_control_tab::on_control_deactivated));
	control->subscribe_on_property_changed	(gcnew property_changed(this, &terrain_painter_control_tab::external_property_changed));
}

void terrain_painter_control_tab::on_control_activated(xray::editor::editor_control_base^)
{
	Show						();
	sync						(true);
}

void terrain_painter_control_tab::on_control_deactivated(xray::editor::editor_control_base^)
{
	Hide						();
}

void terrain_painter_control_tab::sync(bool b_sync_ui)
{
	if(m_in_sync)
		return;

	m_in_sync							= true;
	terrain_modifier_detail_painter^ control	= m_tool->m_terrain_modifier_detail_painter;
	ListViewItem^ selected				= (textures_list_view->SelectedItems->Count)?
										textures_list_view->SelectedItems[0] : nullptr;

	if(b_sync_ui)
	{
		textures_list_view->Items->Clear();

		if(m_tool->get_terrain_core())
		{
			for each(System::String^ t in m_tool->get_terrain_core()->m_textures)
				textures_list_view->Items->Add(t);

			if(control->active_texture)
			{
				ListViewItem^ found = textures_list_view->FindItemWithText(control->active_texture);
				if(found)
					found->Selected = true;
				else
					control->active_texture = nullptr;

			}

			textures_list_view->Items->Add("<New>");
		}
	}else
	{
		if(selected)
		{
			control->active_texture	= selected->Text;
		}else
			control->active_texture	= nullptr;
	}

	m_in_sync = false;
}

void terrain_painter_control_tab::sync(System::Object^, System::EventArgs^)
{
	sync					(false);
}

void terrain_painter_control_tab::external_property_changed(xray::editor::editor_control_base^)
{
	sync					(true);
}

bool terrain_painter_control_tab::select_texture(System::String^ active_item)
{

	resource_selector_ptr selector	(m_tool->get_level_editor()->get_editor_world(), resource_selector::resource_type::Texture);
	selector->selecting_entity			= resource_selector::entity::resource_object;
	selector->selected_name				= active_item;

	if(selector->ShowDialog(this) == DialogResult::OK)
	{
		texture_options_ptr opt_ptr			= static_cast_resource_ptr<texture_options_ptr>(selector->selected_resource);

		System::String^ selected			= gcnew System::String(opt_ptr->m_resource_name.c_str());
		terrain_modifier_detail_painter^ control= m_tool->m_terrain_modifier_detail_painter;
		terrain_core^ core						= m_tool->get_terrain_core();
		
		if(!core->m_textures.Contains(selected))
		{
			if(active_item=="<New>")
				core->add_texture				(selected);
			else
				core->change_texture			(active_item, selected);
		}
		control->active_texture					= selected;

		return true;
	}else
		return false;
}

void terrain_painter_control_tab::textures_list_view_MouseDown(System::Object^, System::Windows::Forms::MouseEventArgs^)
{
}

void terrain_painter_control_tab::textures_list_view_MouseClick(System::Object^, System::Windows::Forms::MouseEventArgs^)
{
	System::Drawing::Point	pt				= textures_list_view->PointToClient(MousePosition);
	ListViewItem^ list_item					= textures_list_view->GetItemAt(pt.X, pt.Y);

	if(!list_item)
	{
		if(textures_list_view->SelectedItems->Count)
			textures_list_view->SelectedItems[0]->Selected = false;

		sync								(false);
		return;
	}else
	{
		System::String^ active_item		= list_item->Text;

		if(active_item=="<New>")
		{
			select_texture(active_item);
			sync					(true); // control->form
		}else
		{
			list_item->Selected			= true;
			sync						(false);
		}
	}

}

void terrain_painter_control_tab::textures_list_view_DoubleClick(System::Object^, System::EventArgs^)
{		
	if(textures_list_view->SelectedItems->Count == 0)
		return;

	System::String^ active_item	= textures_list_view->SelectedItems[0]->Text;
	
	if(select_texture(active_item))
		sync		(true);

}
