////////////////////////////////////////////////////////////////////////////
//	Created		: 30.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "tool_base.h"
#include "object_base.h"
#include "level_editor.h"
#include "editor_world.h"
#include "property_container.h"
#include "property_holder.h"
#include "object_inspector_tab.h"
#include "lua_config_value_editor.h"

namespace xray {
namespace editor {

lib_item::lib_item(xray::configs::lua_config_value lv, level_editor^ le)
:m_level_editor(le)
{ 
	m_config				= NEW(configs::lua_config_value)(lv);
	m_property_restrictor	= gcnew property_restrictor();
	m_property_restrictor->in_create();
	m_lib_item_name			= "";
	
}

lib_item::~lib_item()
{
	this->!lib_item();
}

lib_item::!lib_item()
{
	m_property_restrictor->in_destroy	();
	delete m_property_restrictor;
	DELETE (m_config);
}

System::String^ lib_item::get_raw_lib_name()
{
	return gcnew System::String( (*m_config)["lib_name"]);
}

tool_base::tool_base(level_editor^ le, System::String^ name)
:m_level_editor(le),
m_tool_name(name),
m_b_ready(false),
m_selected_library(nullptr)
{
	m_library_cfg		= NEW(configs::lua_config_ptr)();
	m_tool_library		= gcnew library_items;
	m_object_list		= gcnew object_list;
}

tool_base::~tool_base()
{
	R_ASSERT				(m_object_list->Count==0);

	for each (lib_item^ itm in m_tool_library)
		delete				itm;

	DELETE (m_library_cfg);
}

void tool_base::select_library_name(System::String^ name)
{
	m_selected_library = name;
}

lib_item^ tool_base::get_library_item(System::String^ library_name, bool b_assert)
{
	lib_item^ result		= nullptr;
	for each (lib_item^ li in m_tool_library)
	{
		if(library_name==li->m_lib_item_name)
		{
			result	= li;
			break;
		}
	}
	R_ASSERT		(!b_assert || result, "library item [%s] doesnt exist in library", library_name);
	return			result;
}

void tool_base::on_library_files_loaded(xray::resources::queries_result& data)
{
	R_ASSERT			(!data.is_failed());
	(*m_library_cfg)	= static_cast_checked<configs::lua_config*>(data[0].get_unmanaged_resource().c_ptr());

	configs::lua_config::const_iterator	i		= (*m_library_cfg)->begin( );
	configs::lua_config::const_iterator	const e = (*m_library_cfg)->end( );
	for ( ; i != e; ++i ) 
	{
		configs::lua_config_value t	= *i;
		System::String^ name		= gcnew System::String(t["name"]);
		lib_item^ itm				= new_lib_item(name);
		itm->m_property_restrictor->load(t);
		m_tool_library->Add			(itm);
	}

	on_library_loaded				();
	m_b_ready						= true;
	m_level_editor->tool_loaded		(this);
}

void tool_base::on_commit_library_item(lib_item^ lib_item)
{
	int idx = m_tool_library->IndexOf	(lib_item);
	
	if(idx==-1) //edit or new 
		m_tool_library->Add				(lib_item);

	rename_cfg(lib_item->m_config, lib_item->m_lib_item_name);
	on_library_loaded				();
}

void tool_base::on_remove_library_object(System::String^ library_name)
{
	lib_item^ li					= get_library_item(library_name, true);
	// check existing links
	li->m_config->remove_from_parent();
	m_tool_library->Remove			(li);
	on_library_loaded				();
	delete li;
}

lib_item^ tool_base::new_lib_item(System::String^ library_name)
{
	unmanaged_string s(library_name);
	xray::configs::lua_config_value v = (**m_library_cfg)[s.c_str()];

	lib_item^ result				= gcnew lib_item(v, get_level_editor());
	result->m_lib_item_name			= library_name;
	return							result;
}

void tool_base::check_correct_lib_name(System::String^% name, int index)
{
	System::String^ index_str		= System::UInt32(index).ToString();
	lib_item^ item					= get_library_item(name, false);

	if( item != nullptr )
	{
		int idx			= name->LastIndexOf('(');
		int idx_en		= (idx!=-1)?name->IndexOf(')', idx):-1;

		if( idx!=-1 && idx_en!=-1)
		{
			System::String^  num = name->Substring(idx+1, idx_en-idx-1);
			if(num==index_str)
				++index;

			name = name->Substring(0, idx);
		}
		name += "(";
		name += index_str;
		name += ")";

		check_correct_lib_name(name, index );
	}
}

editor_ide^ tool_base::ide()
{
	return get_level_editor()->ide();
}

} // namespace editor
} // namespace xray
