////////////////////////////////////////////////////////////////////////////
//	Created		: 09.02.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "texture_editor_source.h"
#include <xray/editor/base/managed_delegate.h>
#include "resource_editor.h"
#include <xray/editor/base/images/images16x16.h>

using namespace System;
using namespace System::IO;

namespace xray {
namespace editor {

	texture_editor_source::texture_editor_source()
{
	m_filter_type = (ETType)-1;
}

	texture_editor_source::texture_editor_source(Object^ filter)
{
	m_filter_type = texture_options::get_type_from_name(unmanaged_string(safe_cast<String^>(filter)).c_str());
}

void	texture_editor_source::on_fs_iterator_ready		(xray::resources::fs_iterator fs_it)
{
	process_fs(fs_it.children_begin(), "");
	m_parent->on_items_loaded();
}

void	texture_editor_source::process_fs				(xray::resources::fs_iterator it,  String^ file_path)
{
	while( !it.is_end() )
	{
		if(it.is_folder())
		{
			String^ folder_name = gcnew String(it.get_name());
			TreeNode^ node = m_parent->add_group(file_path+folder_name, xray::editor_base::folder_closed, xray::editor_base::folder_open);
			process_fs	(it.children_begin(), file_path+folder_name+"/");
			if(node->Nodes->Count == 0)
				m_parent->Nodes->Remove(node);
		}
		else
		{
			if( safe_cast<String^>("tga")->Contains(Path::GetExtension(gcnew String(it.get_name()))->Trim('.')))
			{
				String^ file_name = Path::GetFileNameWithoutExtension(gcnew String(it.get_name()));
				if(m_filter_type != (ETType)(-1))
				{
					if(texture_options::get_type_from_name(unmanaged_string(file_name).c_str()) == m_filter_type)
						m_parent->add_item(file_path+file_name, xray::editor_base::node_resource);
				}
				else
					m_parent->add_item(file_path+file_name, xray::editor_base::node_resource);
			}
		}
		++it;
	}
}

void	texture_editor_source::refresh					()
{
	fs_iterator_delegate* q = NEW(fs_iterator_delegate)(gcnew fs_iterator_delegate::Delegate(this, &texture_editor_source::on_fs_iterator_ready));
	resources::query_fs_iterator(	unmanaged_string("resources/textures_new/sources").c_str(), 
		boost::bind(&fs_iterator_delegate::callback, q, _1), g_allocator);
}


}//namespace editor
}//namespace xray