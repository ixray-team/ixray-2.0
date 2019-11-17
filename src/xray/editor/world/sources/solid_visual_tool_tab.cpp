#include "pch.h"
#include "solid_visual_tool_tab.h"
#include "tool_solid_visual.h"

using xray::editor::solid_visual_tool_tab;

System::Void  solid_visual_tool_tab::treeView_AfterSelect(System::Object^, System::Windows::Forms::TreeViewEventArgs^)
{
	TreeNode^ current_node	= treeView->SelectedNode;
	if(current_node==nullptr || current_node->Nodes->Count)
		m_tool->select_library_name(nullptr);
	else
		m_tool->select_library_name(current_node->FullPath);
}
