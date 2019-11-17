#include "pch.h"
#include "light_tool_tab.h"
#include "tool_light.h"
#include "library_object_tab.h"
//#include "editor_world.h"

using xray::editor::tool_light;
using xray::editor::light_tool_tab;

System::Void light_tool_tab::on_new_library_object(System::Object^, System::EventArgs^)
{
	m_tool->on_make_new_library_object		();
}

System::Void light_tool_tab::on_edit_library_object(System::Object^, System::EventArgs^)
{
	System::String^ node_name				= treeView->SelectedNode->FullPath;
	m_tool->on_edit_library_object			(node_name);
}

System::Void light_tool_tab::on_remove(System::Object^, System::EventArgs^)
{
	System::String^ node_name				= treeView->SelectedNode->FullPath;
	m_tool->on_remove_library_object		(node_name);
}

System::Void light_tool_tab::on_after_select(System::Object^, System::Windows::Forms::TreeViewEventArgs^  e)
{
	if(e->Node==nullptr || e->Node->Nodes->Count)
		m_tool->select_library_name(nullptr);
	else
		m_tool->select_library_name(e->Node->FullPath);
}

System::Void light_tool_tab::on_context_menu_opening(System::Object^, System::ComponentModel::CancelEventArgs^)
{
	ASSERT(0); // uncomment next
	//bool b_on_item					= (treeView->SelectedNode!=nullptr) && (treeView->SelectedNode->Nodes->Count==0);
	//bool b_lib_wnd_visible			= m_tool->world().m_tool_windows.m_library_object_tab->Visible;
	//new_item_menu->Visible			= !b_lib_wnd_visible;
	//properties_item_menu->Visible	= b_on_item && !b_lib_wnd_visible;
	//remove_item_menu->Visible		= b_on_item && !b_lib_wnd_visible;
}
