////////////////////////////////////////////////////////////////////////////
//	Created		: 22.01.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "tree_view.h"
#include "tree_node.h"

using namespace System::IO;
using namespace System::Windows::Forms;

using System::Reflection::BindingFlags;

namespace xray {
namespace editor {
namespace controls{

void		tree_view::select_node(TreeNode^ node)
{
	tree_node^ nod		= safe_cast<tree_node^>(node);
	if(!nod->m_is_selected)
	{
		nod->m_is_selected	= true;
		nod->m_fore_color	= node->ForeColor;
		nod->m_back_color	= node->BackColor;
		node->BackColor		= Drawing::SystemColors::Highlight;
		node->ForeColor		= Drawing::SystemColors::HighlightText;
		m_selected_nodes->Add(node);
		Invalidate	(node->Bounds);
	}
}

void		tree_view::select_sub_nodes(TreeNode^ node)
{
	for each(TreeNode^ sub_node in node->Nodes)
	{
		select_node(sub_node);
		select_sub_nodes(sub_node);
	}
}

void		tree_view::deselect_node(TreeNode^ node)
{
	tree_node^ nod		= safe_cast<tree_node^>(node);
	nod->m_is_selected	= false;
	node->BackColor		= nod->m_back_color;
	node->ForeColor		= nod->m_fore_color;
	m_selected_nodes->Remove(node);
	Invalidate	(node->Bounds);
}

void		tree_view::deselect_nodes(Collections::Generic::List<TreeNode^>^ nodes)
{
	for(int i=nodes->Count-1; i>=0; --i)
	{
		deselect_node(nodes[i]);
	}
}

void		tree_view::OnMouseDown				(MouseEventArgs^ e)
{
	if(e->Button == Windows::Forms::MouseButtons::Right)
	{
		if(selected_nodes->Contains(this->GetNodeAt(e->Location)))
			m_b_right_clicked = true;
	}
	TreeView::OnMouseDown(e);
}

void		tree_view::OnMouseUp				(MouseEventArgs^ e)
{
	m_b_right_clicked = false;
	TreeView::OnMouseUp(e);
}
void		tree_view::OnBeforeSelect			(TreeViewCancelEventArgs^ e)
{
	if(m_b_right_clicked || (!is_selectable_groups && safe_cast<tree_node^>(e->Node)->m_node_type == tree_node_type::group_item))
	{
		e->Cancel = true;
		TreeView::OnBeforeSelect(e);
		return;
	}

	if(is_multiselect)
	{
		if(ModifierKeys == Keys::Control && m_selected_nodes->Contains(e->Node))
		{
			if(m_just_selected_node == e->Node)
			{
				m_just_selected_node = nullptr;
				e->Cancel = true;
				return;
			}
			deselect_node(e->Node);
			SelectedNode = nullptr;
			e->Cancel = true;
			selected_items_changed(this, EventArgs::Empty);
		}
	}

	TreeView::OnBeforeSelect(e);
}

void		tree_view::OnAfterSelect			(TreeViewEventArgs^ e)
{
	if(!is_multiselect)
	{
		TreeView::OnAfterSelect(e);
		m_selected_nodes->Clear();
		m_selected_nodes->Add(e->Node);
		selected_items_changed(this, EventArgs::Empty);
		return;
	}

	m_just_selected_node = SelectedNode;
	SelectedNode = nullptr;
	if(ModifierKeys == Keys::Control && !m_selected_nodes->Contains(e->Node))
	{
		select_node(e->Node);
		m_last_selected_node = e->Node;
		TreeView::OnAfterSelect(e);
		selected_items_changed(this, EventArgs::Empty);
		m_just_selected_node = nullptr;
		return;
	}
	else if(!(ModifierKeys == Keys::Shift) ||(ModifierKeys == Keys::Control && m_last_selected_node == nullptr))
	{
		deselect_nodes(m_selected_nodes);
		select_node(e->Node);
		m_last_selected_node = e->Node;
		TreeView::OnAfterSelect(e);
		selected_items_changed(this, EventArgs::Empty);
		m_just_selected_node = nullptr;
		return;
	}

	if(m_last_selected_node != nullptr)
	{
		//Shift pressed
		TreeNode^ lower_node;
		TreeNode^ upper_node;
		TreeNode^ prev_lower_node;

		//Set upper - lower nodes
		if(m_last_selected_node->Bounds.Y < e->Node->Bounds.Y)
		{
			upper_node 			= m_last_selected_node;
			lower_node 			= e->Node;
		}
		else
		{
			upper_node 			= e->Node;
			lower_node 			= m_last_selected_node;
		}
		prev_lower_node			= lower_node;

		while(lower_node!=nullptr && lower_node != upper_node)
		{
			if(!(!is_selectable_groups && safe_cast<tree_node^>(lower_node)->m_node_type == tree_node_type::group_item))
				select_node			(lower_node);
			prev_lower_node		= lower_node;
			lower_node			= lower_node->PrevVisibleNode;
		}
		if(lower_node != nullptr)
			select_node(lower_node);
		
		m_last_selected_node = e->Node;
	}
	m_last_selected_node = e->Node;
	TreeView::OnAfterSelect(e);
	selected_items_changed(this, EventArgs::Empty);
	m_just_selected_node = nullptr;
}

void		tree_view::select_all				()
{
	if(nodes->Count==0)
		return;

	for(TreeNode^ node = nodes[0]; node != nullptr; node = node->NextVisibleNode)
	{
		if(!(!is_selectable_groups && safe_cast<tree_node^>(node)->m_node_type == tree_node_type::group_item))
			select_node			(node);
	}
}

}//namespace controls
}//namespace editor
}//namespace xray