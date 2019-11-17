////////////////////////////////////////////////////////////////////////////
//	Created		: 12.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "tool_solid_visual.h"
#include "object_solid_visual.h"
#include "solid_visual_tool_tab.h"
#include <xray/editor/base/managed_delegate.h>
#include "level_editor.h"

using xray::editor::tool_solid_visual;
using xray::editor::object_base;
using xray::editor::solid_visual_tool_tab;

tool_solid_visual::tool_solid_visual(xray::editor::level_editor^ le)
:super(le, "solid_visual")
{
	m_tool_tab 		= gcnew solid_visual_tool_tab(this);
	load_library	();
}

tool_solid_visual::~tool_solid_visual()							
{
}

xray::editor::tool_tab^ tool_solid_visual::ui()
{
	return m_tool_tab;
}

void tool_solid_visual::load_library()
{
	fs_iterator_delegate* q = NEW(fs_iterator_delegate)(gcnew fs_iterator_delegate::Delegate(this, &tool_solid_visual::on_library_fs_iterator_ready));
	resources::query_fs_iterator(	"resources/library/static", 
		boost::bind(&fs_iterator_delegate::callback, q, _1), g_allocator);
}

object_base^ tool_solid_visual::create_object(System::String^ name)
{
	return create_raw_object(name);
}

object_base^ tool_solid_visual::load_object(xray::configs::lua_config_value const& t)
{
	System::String^ library_name	= gcnew System::String(t["lib_name"]);
	object_base^ result				= create_object(library_name);
	result->load					(t);
	return result;
}

object_base^ tool_solid_visual::create_raw_object(System::String^ id)
{
	object_solid_visual^ result			=  gcnew object_solid_visual(this);
	m_object_list->Add					(result);
	initialize_property_holder			(result); //tmp
	result->set_visual_name_			(id);
	int del_ind							= id->LastIndexOf("/");
	if( del_ind > 0 )
		result->set_name					(id->Substring(del_ind+1), true);
	else
		result->set_name					(id, true);

	result->set_lib_name				(id);
	return result;
}

void tool_solid_visual::destroy_object(object_base^ o)
{
	R_ASSERT(m_object_list->Contains(o));
	m_object_list->Remove(o);
	delete o;
}

void tool_solid_visual::process_recursive_names(xray::resources::fs_iterator it, System::Windows::Forms::TreeNodeCollection^ nodes, int& count)
{
	while( !it.is_end() )
	{
		if(it.is_folder())
		{
			controls::tree_node^ folder_node	= gcnew controls::tree_node(gcnew System::String(it.get_name()));
			folder_node->Name					= folder_node->Text;
			folder_node->ImageKey				= "group";
			folder_node->SelectedImageKey		= "group";
			nodes->Add(folder_node);
			process_recursive_names				(it.children_begin(), folder_node->nodes, count);
		}else
		{
			if(strstr(it.get_name(), ".ogf"))
			{
				System::String^ s				= gcnew System::String(it.get_name());
				s								= s->Remove(s->Length-4);
				controls::tree_node^ file_node	= gcnew controls::tree_node(s);
				file_node->Name					= s;
				file_node->ImageKey				= "mesh";
				file_node->SelectedImageKey		= "mesh";
				nodes->Add(file_node);
				++count;
			}
		}
		++it;
	}
}


void tool_solid_visual::on_library_fs_iterator_ready(xray::resources::fs_iterator fs_it)
{
	m_tool_tab->treeView->nodes->Clear();

	int count				= 0;
	process_recursive_names	(fs_it.children_begin(), m_tool_tab->treeView->nodes, count);
	
	m_tool_tab->toolStripStatusLabel1->Text = System::String::Format("Total:{0} items", count);
	m_b_ready					= true;
}

void tool_solid_visual::get_active_tool_name(System::String^% name)
{
	name = m_tool_tab->treeView->SelectedNode->Name;
}
