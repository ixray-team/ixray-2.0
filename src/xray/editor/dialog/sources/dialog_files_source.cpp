//-------------------------------------------------------------------------------------------
//	Created		: 17.12.2009
//	Author		: Sergey Prishchepa
//	Copyright (C) GSC Game World - 2009
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "dialog_files_source.h"
#include <xray/editor/base/managed_delegate.h>
#include <xray/editor/base/images/images16x16.h>

using xray::dialog_editor::dialog_files_source;
using System::Windows::Forms::TreeNode;
using System::Reflection::BindingFlags;
using xray::dialog_editor::dialogs_node_sorter;
using xray::editor::controls::tree_node;
using xray::editor::controls::tree_node_type;

//-------------------------------------------------------------------------------------------
//- class dialogs_node_sorter ---------------------------------------------------------------
//-------------------------------------------------------------------------------------------
int dialogs_node_sorter::Compare(Object^ x, Object^ y)
{
	tree_node^ left = safe_cast<tree_node^>(x);
	tree_node^ right = safe_cast<tree_node^>(y);
	if(left->m_node_type!=tree_node_type::single_item)
	{
		if(right->m_node_type != tree_node_type::single_item)
			return (left->Text->CompareTo(right->Text));

		return (false);
	}

	if(right->m_node_type != tree_node_type::single_item)
		return (true);

	return (left->Text->CompareTo(right->Text));
}
//-------------------------------------------------------------------------------------------
//- class dialog_files_source ---------------------------------------------------------------
//-------------------------------------------------------------------------------------------

tree_view^ dialog_files_source::parent::get()
{
	return m_parent;
}

void dialog_files_source::parent::set(tree_view^ value)
{
	m_parent = value;
}

void dialog_files_source::refresh()
{
	fs_iterator_delegate* q = NEW(fs_iterator_delegate)(gcnew fs_iterator_delegate::Delegate(this, &dialog_files_source::on_fs_iterator_ready));
	resources::query_fs_iterator("resources/dialogs/configs", 
		boost::bind(&fs_iterator_delegate::callback, q, _1), g_allocator);
}

void dialog_files_source::on_fs_iterator_ready(xray::resources::fs_iterator fs_it)
{
	process_fs(fs_it.children_begin(), "dialogs/");
	m_parent->TreeViewNodeSorter = gcnew dialogs_node_sorter();
	m_parent->Sort();
	TreeNode^ node = m_parent->root;
	node->FirstNode->Expand();
}

void dialog_files_source::process_fs(xray::resources::fs_iterator it, String^ file_path)
{
	while(!it.is_end())
	{
		if(it.is_folder())
		{
			String^ folder_name = gcnew String(it.get_name());
			m_parent->add_group(file_path+folder_name, xray::editor_base::folder_closed, xray::editor_base::folder_open);
			process_fs(it.children_begin(), file_path+folder_name+"/");
		}
		else
		{
			if(System::IO::Path::GetExtension(gcnew String(it.get_name())) == ".dlg")
			{
				String^ file_name = System::IO::Path::GetFileNameWithoutExtension(gcnew String(it.get_name()));
				m_parent->add_item(file_path+file_name, xray::editor_base::node_resource);
			}
		}
		++it;
	}
}
