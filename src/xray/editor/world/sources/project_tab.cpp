////////////////////////////////////////////////////////////////////////////
//	Created		: 20.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "project_tab.h"
#include "project.h"
#include "object_base.h"
#include "project_items.h"
#include "tool_base.h"
#include "level_editor.h"

using xray::editor::project_tab;

void project_tab::in_constructor()
{
}

Void  project_tab::treeView_MouseDown(System::Object^, System::Windows::Forms::MouseEventArgs^)
{
	System::Windows::Forms::TreeNode^ node = treeView->GetNodeAt(PointToClient(MousePosition));
	if(node && node->Tag)
	{
		treeitem_wrapper^ wrp	= safe_cast<treeitem_wrapper^>(node->Tag);
		wrp->on_mouse_down		();
	}else
		m_level_editor->get_project()->select_object((object_base^)nullptr, enum_selection_method_set);
}

System::Void project_tab::global_view_menu_Opening(System::Object^, System::ComponentModel::CancelEventArgs^ ce )
{
	Windows::Forms::TreeNode^ node	= treeView->GetNodeAt(PointToClient( MousePosition) );

	removeToolStripMenuItem->Enabled		= (node!=nullptr);
	newFilterToolStripMenuItem->Enabled		= (node!=nullptr) && (safe_cast<treeitem_wrapper^>(node->Tag)->type==0);

}

System::Void project_tab::newFilterToolStripMenuItem_Click(System::Object^, System::EventArgs^)
{
	m_level_editor->get_project()->add_new_group		();
}

System::Void project_tab::removeToolStripMenuItem_Click(System::Object^, System::EventArgs^)
{
	m_level_editor->delete_selected		();
}

System::Void project_tab::treeView_BeforeLabelEdit(System::Object^, System::Windows::Forms::NodeLabelEditEventArgs^  e)
{
	treeitem_wrapper^ w		= safe_cast<treeitem_wrapper^>(e->Node->Tag);
	if(w->type!=0)
		e->CancelEdit = true;
}

System::Void project_tab::treeView_AfterLabelEdit(System::Object^, System::Windows::Forms::NodeLabelEditEventArgs^  e)
{
	if(e->Label==nullptr || e->Label->Length==0)
	{
		e->CancelEdit = true;
		return;
	}

	project_group_treeitem_wrapper^ w		= safe_cast<project_group_treeitem_wrapper^>(e->Node->Tag);
	w->m_project_group->name				= e->Label;
}
