////////////////////////////////////////////////////////////////////////////
//	Created		: 20.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "project_items.h"
#include "project.h"
#include "tool_base.h"
#include "object_base.h"
#include "lua_config_value_editor.h"

namespace xray{
namespace editor{


project_group::project_group(project^ p, project_group^ parent)
:m_project(p),m_parent(parent)
{
	m_name = gcnew System::String("New");
	m_groups = gcnew groups;
	m_items = gcnew items;
}

void project_group::load(xray::configs::lua_config_value const& c, enum_id_action id_actions )
{
	configs::lua_config_value const& subitems	= c["subitems"];

	if( m_parent != nullptr )
		m_name		= gcnew System::String(c["name"]);

	configs::lua_config_value::const_iterator	i = subitems.begin( );
	configs::lua_config_value::const_iterator	e = subitems.end( );

	for ( ; i != e; ++i )
	{
		configs::lua_config_value t		= *i;

		if( strings::compare( t["type"], "folder") == 0 )
		{
			System::String^	name		= gcnew System::String(t["name"]);

			project_group^ group =  get_group( name );

			if( group == nullptr )
			{	
				group					= gcnew project_group(m_project, this);
				m_groups->Add			(group);
			}
			
			group->load					(t, id_actions);
		} 
		else if( strings::compare( t["type"], "object") == 0 )
		{
			add_item(t,false,id_actions);		
		}
	}
}

void project_group::paste(xray::configs::lua_config_value const& c, 
						  System::Collections::IDictionary^% path_to_id, 
						  System::String^% curr_path)
{
	configs::lua_config_value const& subitems	= c["subitems"];
	m_name										= gcnew System::String (c["name"]);

	configs::lua_config_value::const_iterator	i = subitems.begin();
	configs::lua_config_value::const_iterator	e = subitems.end();

	for ( ; i != e; ++i )
	{
		configs::lua_config_value t		= *i;

		System::String^	name					= gcnew System::String(t["name"]);

		if( 0==strings::compare(t["type"], "folder") )
		{
			project_group^ group	= get_group(name);

			if( group == nullptr )
			{	
				group				= gcnew project_group(m_project, this);
				m_groups->Add		(group);

				t["newly_added"]	= true;
			}

			System::String^ s		= System::String::Concat(curr_path, name, "/");
			group->paste(t, path_to_id, s);

		}else 
		if( 0==strings::compare(t["type"], "object") )
		{
			project_item^ new_item	= add_item(t, true, enum_id_action(enum_id_action_save|enum_id_action_load));		
			
			System::String^ s		= System::String::Concat(curr_path, name);
			path_to_id[new_item->m_object_base] = s;
		}
	}
}

void project_group::get_all_items(object_list^% objects)
{
	for each (project_item^ pi in m_items)
		objects->Add( pi->m_object_base );

	for each (project_group^ pg in m_groups)
		pg->get_all_items( objects );
}

void project_group::save(xray::configs::lua_config_value c)
{
	set_c_string				(c["name"], m_name);
	c["type"]					= "folder";

	u32 index = 0;
	for each (project_group^ pg in m_groups)
	{
		pg->save				( c["subitems"][index] );
		++index;
	}

	for each (project_item^ pi in m_items)
	{
		pi->save				( c["subitems"][index], false );
		++index;
	}
}

void project_group::save(xray::configs::lua_config_value c, System::String^ path, object_base^ object, bool save_ids)
{
	ASSERT(object != nullptr);

	set_c_string				(c["name"], m_name);
	c["type"]					= "folder";

	if(!path->Empty)
	{
		int ss					= path->IndexOf('/');
		System::String^			name;
		System::String^			new_path;
		if(ss==-1)
		{
			name = path;
		}else
		{
			name		= path->Substring(0, ss);
			new_path	= path->Substring(ss+1);
		}

		u32 index = 0;
		for each (project_group^ pg in m_groups)
		{
			if( name == pg->name )
			{
				pg->save( c["subitems"][index], new_path, object, save_ids ); 
				return;
			}
			++index;
		}
	}else
	{
		u32 index = 0;
		if( c["subitems"].get_type() == configs::t_table_indexed )
			index = c["subitems"].size();

		object->m_owner_project_item->save( c["subitems"][index], save_ids ); 

		index			= c["subitems"].size();
		index			= index;
	}
}

void project_group::clear()
{
	for each (project_group^ pg in m_groups)
	{
		pg->clear();
		delete pg;
	}
	m_groups->Clear();

	for each (project_item^ pi in m_items)
	{
		pi->clear		(true);
		delete pi;
	}
	m_items->Clear		();
	m_tree_node->Remove	();
}

void project_group::remove(project_group^ pg)
{
	R_ASSERT		(m_groups->Contains(pg));
	m_groups->Remove(pg);
}

void project_group::remove(project_item^ pi)
{
	R_ASSERT						(m_items->Contains(pi));
	pi->m_object_base->set_visible	(false);
	m_items->Remove					(pi);
}

void project_group::remove(xray::configs::lua_config_value const& cfg)
{
	configs::lua_config_value const& subitems	= cfg["subitems"];

	configs::lua_config_value::const_iterator	i = subitems.begin( );
	configs::lua_config_value::const_iterator	e = subitems.end( );

	for ( ; i != e; ++i )
	{
		configs::lua_config_value const& t		= *i;

		if( strings::compare( t["type"], "folder") == 0 )
		{
			System::String^	name	= gcnew System::String(t["name"]);
			project_group^ group	= get_group( name );

			if( group != nullptr )
			{
				group->remove		(t);

				if( t.value_exists("newly_added") )
					remove(group);
			}
		}else 
		if( strings::compare( t["type"], "object") == 0 )
		{
			project_item^ item		= nullptr;
			if( t.value_exists("id") )
				item = get_item( u32(t["id"]) );
			else
				item = get_item( gcnew System::String(t["name"]) );

			if( item != nullptr )
				m_project->remove_item( item, true );
		}
	}
}

bool project_group::is_empty()
{
	return m_groups->Count==0 && m_items->Count==0;
}

void project_group::fill_tree_view(tree_node_collection^ nodes, u32& count)
{
	m_tree_node							= gcnew controls::tree_node(gcnew System::String(name));
	m_tree_node->Name					= m_tree_node->Text;
	m_tree_node->ImageKey				= "group";
	m_tree_node->SelectedImageKey		= "group";
	nodes->Add							(m_tree_node);
	m_tree_node->Tag					= gcnew project_group_treeitem_wrapper(this);


	for each (project_group^ pg in m_groups)
		pg->fill_tree_view				(m_tree_node->Nodes, count);

	for each (project_item^ pi in m_items)
		pi->fill_tree_view				(m_tree_node->Nodes, count);
}

void project_group::render()
{
	for each (project_group^ pg in m_groups)
		pg->render			();

	for each (project_item^ pi in m_items)
		pi->render			();
}

project_item^ project_group::add_item(object_base^ object)
{
	project_item^ new_item		= gcnew project_item(m_project, object, this);

	System::String^ name		= object->get_name();

	if(	check_correct_existance(name, 1) )
		object->set_name		(name, false);

	m_items->Add				(new_item);
	new_item->m_object_base->set_visible(true);

	u32 count					= 0;
	new_item->fill_tree_view	(m_tree_node->Nodes, count);
	
	if(!m_tree_node->IsExpanded)
		m_tree_node->Expand();

	return new_item;
}

project_item^ project_group::add_item(xray::configs::lua_config_value c, bool fillui, enum_id_action id_action)
{
	ASSERT( strings::compare( c["type"], "object") == 0 );

	project_item^ item			= gcnew project_item(m_project, this);
	item->load					(c, id_action);

	System::String^ name		= item->m_object_base->get_name();
	if(	check_correct_existance(name, 1) )
		item->m_object_base->set_name(name, false);

	m_items->Add				(item);
	item->m_object_base->set_visible(true);

	if(fillui)
	{
		u32 count				= 0;
		item->fill_tree_view	(m_tree_node->Nodes, count);
	}

	m_project->on_object_load(item->m_object_base, c);
	return item;
}

bool project_group::check_correct_existance(System::String^% name, u32 index)
{
	System::String^ index_str		= System::UInt32(index).ToString();
	project_item^ item				= get_item(name);

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

		check_correct_existance( name, index );
		return true;
	}
	return false;
}

project_group^ project_group::get_group(System::String^ name)
{
	for each (project_group^ pg in m_groups)
		if(pg->name==name)
			return pg;

	return nullptr;
}

project_item^ project_group::get_item(u32 id)
{
	for each (project_item^ pi in m_items)
		if(pi->m_object_base->id()==id)
			return pi;

	return nullptr;
}

project_item^ project_group::get_item(System::String^ name)
{
	for each (project_item^ pi in m_items)
		if(pi->name()==name)
			return pi;

	return nullptr;
}

System::String^ project_group::get_path()
{
	System::String^ result	= nullptr;
	if(m_parent != nullptr)
	{
		System::String^ parent_path		= m_parent->get_path();
		result							= System::String::Concat(parent_path, name, "/");
	}else
		result = "";

	return result;
}

void project_group::set_last_selected()
{
	m_project->m_last_selected_group = this;
}

void project_group::set_name_internal(System::String^ s)
{
	m_name = s;
	if(m_tree_node)
		m_tree_node->Text = m_name;
}

project_item::project_item(project^ p, project_group^ pg)
:m_project(p), m_object_base(nullptr), m_parent(pg)
{}

project_item::project_item(project^ p, object_base^ object, project_group^ pg)
:m_project(p), m_object_base(object), m_parent(pg)
{
	object->m_owner_project_item = this;
}

void project_item::load(xray::configs::lua_config_value& t, enum_id_action id_action )
{
	ASSERT(m_object_base==nullptr, "The item allready has an object");

	System::String^ tool_name		= gcnew System::String(t["tool_name"]);
	m_object_base					= m_project->get_tool(tool_name)->load_object(t);
	m_object_base->m_owner_project_item	= this;
	
	if( (id_action&enum_id_action_load) != 0 && t.value_exists("id") )
		m_object_base->assign_id		(t["id"]);
	else
	{
		m_object_base->assign_id		(0);

		// we use non-const reference to config because of this
		// logic is not clear here
#pragma message( XRAY_TODO("Dima to Armen: please refactor this and return const reference") )
		if( (id_action&enum_id_action_save) != 0 )
			t["id"] = m_object_base->id();
	}
}

void project_item::save(xray::configs::lua_config_value cfg, bool save_ids)
{
	m_object_base->save(cfg);

	if(save_ids)
		cfg["id"] = m_object_base->id();
}

void project_item::clear(bool b_destroy_object)
{
	System::String^ s = m_tree_node->Text;
	if(b_destroy_object)
		m_object_base->owner()->destroy_object(m_object_base);

	m_object_base		= nullptr;
	System::String^ s2 = m_tree_node->Text;
	m_tree_node->Remove	();
}

System::String^ project_item::name()
{
	R_ASSERT			(m_object_base);
	return m_object_base->get_name();
}

void project_item::fill_tree_view(tree_node_collection^ nodes, u32& count)
{
	XRAY_UNREFERENCED_PARAMETER			( count );
	m_tree_node							= nodes->Add(gcnew System::String(name()));
	m_tree_node->ImageKey				= m_object_base->image_key;
	m_tree_node->SelectedImageKey		= m_object_base->image_key;
	m_tree_node->Tag					= gcnew project_item_treeitem_wrapper(this);
}

void project_item::refresh_ui()
{
	m_tree_node->Text = gcnew System::String(name());
}

void project_item::free_tree_view(tree_node_collection^ nodes)
{
	nodes->Remove( m_tree_node);
}

void project_item::render()
{
	m_object_base->render			();
}

bool project_item::get_selected()
{
	return m_object_base->get_selected();
}

void project_item::set_selected(bool b)
{
	if( b )
	{
		R_ASSERT(m_tree_node);
		m_tree_node->ForeColor			= System::Drawing::SystemColors::HighlightText;
		m_tree_node->BackColor			= System::Drawing::SystemColors::MenuHighlight;
	}
	else
	{
		m_tree_node->ForeColor			= System::Drawing::SystemColors::WindowText;
		m_tree_node->BackColor			= System::Drawing::SystemColors::Window;
	}
}

System::String^ project_item::get_path()
{
	return m_parent->get_path			();
}

void project_item::set_last_selected()
{
	m_project->m_last_selected_item		= this;
}

void project_group_treeitem_wrapper::on_mouse_down()
{
	super::on_mouse_down				();
	m_project_group->set_last_selected	();
}

void project_item_treeitem_wrapper::on_mouse_down()
{
	super::on_mouse_down				();

	if(m_project_item->m_object_base->get_selectable())
	{
		m_project_item->set_last_selected	();
		m_project_item->m_project->select_object(m_project_item->m_object_base, enum_selection_method_set);
	}
}

} // namespace editor
} // namespace xray
