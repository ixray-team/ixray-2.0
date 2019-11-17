#include "pch.h"
#include "compound_tool_tab.h"
#include "tool_compound.h"

using xray::editor::compound_tool_tab;

System::Void compound_tool_tab::on_edit_library_object(System::Object^  sender, System::EventArgs^  e)
{
	XRAY_UNREFERENCED_PARAMETERS			( sender, e );
	System::String^ node_name				= treeView->SelectedNode->FullPath;
	m_tool->on_edit_library_object			(node_name);
}

System::Void compound_tool_tab::on_new_library_object(System::Object^  sender, System::EventArgs^  e)
{
	XRAY_UNREFERENCED_PARAMETERS			( sender, e );
	m_tool->on_make_new_library_object();
}

System::Void compound_tool_tab::on_remove(System::Object^  sender, System::EventArgs^  e)
{
	XRAY_UNREFERENCED_PARAMETERS			( sender, e );
	System::String^ node_name				= treeView->SelectedNode->FullPath;
	m_tool->on_remove_library_object		(node_name);
}

System::Void compound_tool_tab::on_context_menu_opening(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e)
{
	XRAY_UNREFERENCED_PARAMETERS			( sender, e );
}

System::Void compound_tool_tab::on_after_select(System::Object^, System::Windows::Forms::TreeViewEventArgs^  e)
{
	if(e->Node==nullptr || e->Node->Nodes->Count)
		m_tool->select_library_name(nullptr);
	else
		m_tool->select_library_name(e->Node->FullPath);
}
