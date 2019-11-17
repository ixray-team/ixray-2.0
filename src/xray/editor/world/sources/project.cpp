////////////////////////////////////////////////////////////////////////////
//	Created		: 11.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "project.h"
#include "project_items.h"
#include "project_tab.h"
#include "object_inspector_tab.h"
#include "scene_graph_tab.h"
#include "library_object_tab.h"
#include "level_editor.h"
#include "editor_world.h"
#include "object_base.h"
#include "tool_base.h"
#include "editor_control_base.h"
#include <xray/editor/base/managed_delegate.h>
#include "property_connection_editor.h"
#include <xray/render/base/editor_renderer.h>

#pragma managed(push)
#include <xray/fs_path.h>
#pragma managed(pop)

namespace xray {
namespace editor {

#define UNTITLED_PROJECT_NAME	"Untitled Project"

project::project(xray::editor::level_editor^ le):
m_level_editor			( le ),
m_last_selected_group	( nullptr ),
m_last_selected_item	( nullptr ),
m_root_group			( nullptr ),
m_tmp_is_save_to_file	( false ),
m_project_path			( UNTITLED_PROJECT_NAME )
{	
	m_config			= NEW(configs::lua_config_ptr)();
	m_project_tab		= m_level_editor->get_project_explorer();
	m_selection_list	= gcnew object_list;
	m_link_storage		= gcnew link_storage;
	subscribe_on_selection_changed( gcnew selection_changed(this, &project::on_selection_changed) );
	
	query_result_delegate* q = NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &project::on_config_ready));

	pcstr str_buf		= " ";
	resources::query_create_resource(
						"Untitled Project.xprj",
						xray::memory::buffer(str_buf, strings::length(str_buf)),
						xray::resources::config_lua_class,
						boost::bind(&query_result_delegate::callback, q, _1),
						g_allocator
						);
}

void project::on_config_ready( xray::resources::queries_result& data )
{
	R_ASSERT			( data.is_successful() );
	(*m_config)			= static_cast_checked<configs::lua_config*>(data[0].get_unmanaged_resource().c_ptr());

	LOG_INFO	("loaded config %s", resource_name().c_str() );
	if( !(*m_config)->empty() )
		load			( (*m_config)->get_root() , false );
}

fs::path_string project::resource_name( )
{
	return (*m_config)->reusable_request_name ();
}

project::~project()
{
	DELETE				( m_config );
}

object_base^ project::aim_object() 
{ 
	return (m_selection_list->Count>0)? m_selection_list[0] : nullptr; 
}

xray::editor::project_ui project::ui()
{
	return m_project_tab;
}

bool project::empty()
{
	return (m_root_group==nullptr);
}

void project::on_selection_changed()
{
	System::String^ props	= System::String::Format("Selected: [{0}]", m_selection_list->Count);
	m_level_editor->ide()->set_status_label(1, props);
}

bool project::name_assigned( )
{
	return m_project_path->Length>0 && m_project_path != UNTITLED_PROJECT_NAME;
}

void project::create_new()
{
	m_project_path				= UNTITLED_PROJECT_NAME;
	ASSERT						( empty() );
	m_root_group				= gcnew project_group(this, nullptr);

	object_base^ o				= get_tool("terrain")->create_raw_object("base:terrain_core");
	o->set_lib_name				( "base:terrain_core" );
	o->assign_id				( 0 );
	o->set_name					( "terrain", false );
	o->load_defaults			( );

	project_item^ terrain_item	= gcnew project_item( this, o, m_root_group );
	m_root_group->m_items->Add	( terrain_item );
	terrain_item->m_object_base->set_visible( true );

	fill_tree_view				( m_project_tab->treeView->nodes );
}

void project::clear()
{
	m_project_path			= "";
	if(!empty())
	{
		select_object		(nullptr, enum_selection_method_set);

		m_root_group->clear	();
		delete				m_root_group;
		m_root_group		= nullptr;
	}
	fill_tree_view		(m_project_tab->treeView->nodes);
}

void project::load( System::String^ project_name )
{
	clear					( );
	m_root_group			= gcnew project_group(this, nullptr);

	m_project_path			= project_name;
	m_root_group->name		= System::IO::Path::GetFileNameWithoutExtension(project_name);

	System::String^ fn		= "resources/projects/";
	fn						+= project_name;
	fn						+= "/project.xprj";

	unmanaged_string		sss(fn);
	fs::path_string pth		= fs::convert_to_portable( sss.c_str() );

	query_result_delegate* q = NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &project::on_config_ready));

	xray::resources::query_resource(
		pth.c_str(), 
		xray::resources::config_lua_class,
		boost::bind(&query_result_delegate::callback, q, _1),
		g_allocator);
}

void project::load(xray::configs::lua_config_value const& cfg, bool load_ids )
{
	if(!cfg["camera"].empty())
	{
		float3 p	=	cfg["camera"]["position"];
		float3 d	=	cfg["camera"]["direction"];
		m_level_editor->set_camera_props(p, d);
	}

	enum_id_action id_action = enum_id_action_NONE;
	if( load_ids )
		id_action = enum_id_action_load;

	m_link_tmp_list				= gcnew System::Collections::Generic::List<link_temporary_data^>();
	m_paste_tmp_list			= gcnew System::Collections::Generic::List<paste_temporary_data^>();

	m_root_group->load			( cfg["project"]["objects_tree"], id_action );
	fill_tree_view				( m_project_tab->treeView->nodes );

	for( int i  = 0; i < m_link_tmp_list->Count; ++i )
	{
		object_base^ src_obj = get_item_by_path( m_link_tmp_list[i]->src_name );

		ASSERT( src_obj );
		if( src_obj )
			m_link_storage->create_link( src_obj, m_link_tmp_list[i]->src_member, m_link_tmp_list[i]->dst_obj, m_link_tmp_list[i]->dst_member );
	}

	delete m_link_tmp_list;
	m_link_tmp_list = nullptr;

	delete m_paste_tmp_list;
	m_paste_tmp_list = nullptr;
}

void project::paste(xray::configs::lua_config_value const& cfg, id_list^% ids)
{
	System::Collections::IDictionary^ path_to_obj = gcnew System::Collections::Hashtable;
	System::String^ curr_path;

	m_link_tmp_list		= gcnew System::Collections::Generic::List<link_temporary_data^>();
	m_paste_tmp_list	= gcnew System::Collections::Generic::List<paste_temporary_data^>();

	m_root_group->paste		( cfg["project"]["objects_tree"], path_to_obj, curr_path );

	for( int i  = 0; i < m_link_tmp_list->Count; ++i )
	{
		String^ src_name = m_link_tmp_list[i]->src_name;
		object_base^ src_obj = nullptr;
		for( int j = 0; j < m_paste_tmp_list->Count; ++j )
		{
			if( src_name == m_paste_tmp_list[j]->orig_name )
			{
				src_obj = m_paste_tmp_list[j]->object;
				break;
			}
		}

		if( !src_obj )
			src_obj = get_item_by_path( src_name );

		ASSERT( src_obj );
		if( src_obj )
			m_link_storage->create_link( src_obj, m_link_tmp_list[i]->src_member, m_link_tmp_list[i]->dst_obj, m_link_tmp_list[i]->dst_member );
	}

	delete m_link_tmp_list;
	m_link_tmp_list = nullptr;

	delete m_paste_tmp_list;
	m_paste_tmp_list = nullptr;

	ids->Clear();
	for each (System::Collections::DictionaryEntry^ it in path_to_obj)
	{
		object_base^ o = safe_cast<object_base^>(it->Key);
		ids->Add( o->id() );
	}
}

void project::save( System::String^ project_name )
{
	m_tmp_is_save_to_file			= true;

	m_root_group->name				= System::IO::Path::GetFileNameWithoutExtension(project_name);
	m_project_path					= project_name;

	fs::path_string resource_path	= m_level_editor->get_editor_world().engine().get_resource_path();
	String^ path					= gcnew System::String(resource_path.c_str());
	path							+= "/projects/";
	path							+= project_name;
	path							+= "/project.xprj";

	System::String^ file_name	= IO::Path::GetFullPath(path);

	save_internal					( );

	(*m_config)->save_as			( unmanaged_string(file_name).c_str() );
	m_tmp_is_save_to_file			= false;
}

void project::save_internal( )
{
	(*m_config)->get_root().clear	( );
	save_to_config					( (*m_config)->get_root() );
}

void project::save_to_config( xray::configs::lua_config_value cfg )
{
	float3 p, d;
	m_level_editor->get_camera_props	(p, d);
	cfg["camera"]["position"]			= p;
	cfg["camera"]["direction"]			= d;

	m_root_group->save			( cfg["project"]["objects_tree"] ); 
}

void project::save_to_config(xray::configs::lua_config_value cfg, object_base^ object, bool save_id)
{
	ASSERT(object != nullptr);

	System::String^ path = object->m_owner_project_item->get_path();
	m_root_group->save( cfg["project"]["objects_tree"], path, object, save_id ); 
}

void project::save_to_config(xray::configs::lua_config_value cfg, object_list^ objects, bool save_ids)
{
	for each (object_base^ obj in objects)
		save_to_config(cfg, obj, save_ids);
}

//void project::save_links(System::String^ project_name)
//{
//	String^ ext					= System::IO::Path::GetExtension (project_name);
//	String^ links_file_name		= project_name->Remove( project_name->Length - ext->Length ) + ".xlnk";
//	unmanaged_string s			(links_file_name);
//	configs::lua_config_ptr cfg = configs::create_lua_config(s.c_str());
//	configs::lua_config_value cfg_links = cfg->get_root()["links"];
//
//	System::Collections::ArrayList^ links = m_link_storage->get_links();
//	for( int i = 0; i < links->Count; ++i )
//	{
//		System::String^ path	= ((property_link^)links[i])->m_src_object->m_owner_project_item->get_path();
//		path					+= ((property_link^)links[i])->m_src_object->get_name();
//		unmanaged_string path_str_src(path);
//		cfg_links[i]["src_name"] = path_str_src.c_str();
//
//		path					= ((property_link^)links[i])->m_dst_object->m_owner_project_item->get_path();
//		path					+= ((property_link^)links[i])->m_dst_object->get_name();
//		unmanaged_string path_str_dst(path);
//		cfg_links[i]["dst_name"] = path_str_dst.c_str();	
//
//		unmanaged_string src_property_name(((property_link^)links[i])->m_src_property_name);
//		cfg_links[i]["src_member"] = src_property_name.c_str();
//
//		unmanaged_string dst_property_name(((property_link^)links[i])->m_dst_property_name);
//		cfg_links[i]["member"] = dst_property_name.c_str();
//	}
//	cfg->save	();
//}

//void project::load_links(System::String^ project_name)
//{
//	m_link_storage->clear_all(	);
//	String^ ext = System::IO::Path::GetExtension (project_name);
//	String^ links_file_name = project_name->Remove( project_name->Length - ext->Length ) + ".xlnk";
//	unmanaged_string s(links_file_name);
//
//	fs::path_string	request_path = "@";
//	request_path += s.c_str();
//
//	xray::resources::request  requests[] = { request_path.c_str(), xray::resources::config_lua_class };
//
//	query_result_delegate* q = NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &project::on_project_links_loaded));
//
//	xray::resources::query_resources(requests, 
//		boost::bind(&query_result_delegate::callback, q, _1),
//		g_allocator);
//}

//void project::on_project_links_loaded	(xray::resources::queries_result& data)
//{
//	R_ASSERT						(!data.is_failed());
//	if( data.size() > 0 )
//	{
//		configs::lua_config_ptr cfg = static_cast_checked<configs::lua_config*>(data[0].get_unmanaged_resource().c_ptr());
//		
//		configs::lua_config_value const& cfg_links = cfg->get_root()["links"];
//
//		//System::Collections::ArrayList^ links = m_link_storage->get_links();
//		configs::lua_config_value::const_iterator	i = cfg_links.begin( );
//		configs::lua_config_value::const_iterator	e = cfg_links.end( );
//
//		for ( ; i != e; ++i )
//		{
//			configs::lua_config_value const& t	= *i;
//			System::String^ src_full_name			= gcnew System::String(t["src_name"]);
//			object_base^ src_obj = get_item_by_path		(src_full_name);
//			System::String^ dst_full_name			= gcnew System::String(t["dst_name"]);
//			object_base^ dst_obj = get_item_by_path		(dst_full_name);
//			ASSERT( src_obj != nullptr && dst_obj != nullptr );
//
//			m_link_storage->create_link( src_obj, gcnew System::String(t["src_member"]), dst_obj, gcnew System::String(t["member"]) );
//		}
//	}
//}

object_base^ project::get_item_by_path		(System::String^ path)
{
	int del_int = path->LastIndexOf("/");			
	project_group^ parent = nullptr;
	System::String^ name;
	if( del_int > 0 )
	{
		System::String^ parent_path;
		parent_path	= path->Substring(0,del_int);
		name		= path->Substring(del_int+1);
		parent		= find_group( parent_path, false );
	}
	else
	{
		name = path;
	}

	if( parent == nullptr )
		parent = m_root_group;

	project_item^ src_itm = parent->get_item(name);
	if( src_itm != nullptr )
		return src_itm->m_object_base;

	return nullptr;
}

tool_base^ project::get_tool(System::String^ tool_name)
{
	return m_level_editor->get_tool(tool_name);
}

project_group^ project::find_group(System::String^ name, bool bcreate)
{
	project_group^	result	= m_root_group;

	cli::array<System::String^>^ tokens = name->Split('/');

	for(int i=0; i<tokens->GetLength(0); ++i)
	{
		System::String^ tok = tokens[i];
		project_group^	tmp		= nullptr;
		tmp		= result->get_group(tok);
		if(!tmp && bcreate )
		{
			tmp							= gcnew project_group(this, result);
			tmp->name					= tok;
			result->m_groups->Add		(tmp);
		}

		result		= tmp;
	}

	return result;
}

void project::fill_tree_view(tree_node_collection^ nodes)
{
	nodes->Clear								();
	if(!empty())
	{
		u32 count									= 0;
		m_root_group->fill_tree_view				(nodes, count);
		m_root_group->m_tree_node->ImageKey			= "default_object_set";
		m_root_group->m_tree_node->SelectedImageKey	= "default_object_set";
		m_root_group->m_tree_node->Expand			();
		m_project_tab->toolStripStatusLabel1->Text = System::String::Format("Total:{0} items", count);
	}else
	{
		nodes->Add									("Create new or load existing");
		m_project_tab->toolStripStatusLabel1->Text = "Empty project";
	}
}

void project::render()
{
	if(!empty())
		m_root_group->render							();

	m_level_editor->get_editor_renderer().render_visuals();
}

void project::add_new_group()
{
	project_group^ parent		= m_root_group;

	if(m_last_selected_group)
		parent = m_last_selected_group;
	else
	if(m_last_selected_item)
		parent = m_last_selected_item->m_parent;

	project_group^ pg			= gcnew project_group(this, parent);

	parent->m_groups->Add		(pg);
	u32 count					= 0;
	pg->fill_tree_view			(parent->m_tree_node->Nodes, count);
	pg->m_tree_node->BeginEdit	();
}

void project::select_id(u32 id, enum_selection_method method)
{
	id_list^ tmp_vector	= gcnew id_list;
	object_base^ obj		= object_base::object_by_id(id);
	if( obj != nullptr )
		tmp_vector->Add		(obj->id());

	select_ids				(tmp_vector, method);

}
void project::select_ids(id_list^ objs, enum_selection_method method)
{
	object_list^ tmp_vector = gcnew object_list;
	
	for each( System::UInt32^ it in objs)
	{
		object_base^ obj	= object_base::object_by_id( *it );
		
		if(obj->get_selectable())
			tmp_vector->Add		(obj);
	}

	select_objects			(tmp_vector, method);
}

void project::select_object( object_base^ obj, enum_selection_method method )
{
	object_list^ tmp_vector = gcnew object_list;

	if(obj != nullptr)
		tmp_vector->Add		(obj);

	select_objects			(tmp_vector, method);
}

ref class objects_enumerator
{
public:
	objects_enumerator(System::Collections::IEnumerator^ en):m_enumerator(en),m_b_valid(true){}
	bool MoveNext	(){m_b_valid = m_enumerator->MoveNext(); return m_b_valid;}
	bool IsValid	(){return m_b_valid;}
	object_base^	current() {return safe_cast<object_base^>(m_enumerator->Current);}
private:
	bool			m_b_valid;
	System::Collections::IEnumerator^ m_enumerator;
};

ref class object_id_predicate:public System::Collections::Generic::IComparer<object_base^>
{
public:
	virtual int Compare( object_base^ first, object_base^ second )
	{
		u32 id1 = first->id();
		u32 id2 = second->id();
		if(id1==id2) return 0;
		else
			return (id1>id2)?1:-1;
	}
};

void project::select_objects(object_list^ objs, const enum_selection_method method)
{
	if( method == enum_selection_method_set )
	{
		for each (object_base^ it_up in m_selection_list)
			it_up->set_selected(false);

		m_selection_list->Clear();
//		update_aim_history( nullptr );
	}

	if( objs->Count== 0 )
	{
		object_inspector_tab^ inspector_tab	= m_level_editor->get_object_inspector_tab();
		if(inspector_tab->Visible)
			inspector_tab->show_properties(selection_list());
		
		m_selection_changed();
		
		return;
	}

	object_list t(objs);
	objs->Clear	();
	for each (object_base^ o in t)
	{
		ASSERT(o->id()!=0);

		if (!objs->Contains(o))
			objs->Add(o);
	}

	// *********************** This assertion need to be uncommented
	//ASSERT(  objs.size() <= m_object_list.size() );

	objs->Sort(gcnew object_id_predicate);
	// *********************** This assertion need to be uncommented
// 	ASSERT( std::includes( m_object_list.begin(), m_object_list.end(), objs.begin(), objs.end() ), 
// 		"Some objecs are not contained int the editor objects list!" );

	object_list^ tmp_vec	= gcnew object_list;

	objects_enumerator^ it		= gcnew objects_enumerator(m_selection_list->GetEnumerator());

	objects_enumerator^ it_o	= gcnew objects_enumerator(objs->GetEnumerator());

	object_base^				control_object = nullptr;

	// Selective adding elements which are intersected between old selections list and new selection objects 
	// depending on select mode.
	it->MoveNext	();
	it_o->MoveNext	();
	while(it->IsValid() && it_o->IsValid())
	{
		if( it->current()->id() < it_o->current()->id() )
		{
			tmp_vec->Add	(it->current());
			it->MoveNext	();
		}else if( it->current()->id() > it_o->current()->id() )
		{
			if( method == enum_selection_method_set || 
				method == enum_selection_method_add || 
				method == enum_selection_method_invert )
			{
				tmp_vec->Add		(it_o->current());

				it_o->current()->set_selected(true);
				control_object		= it_o->current();
			}

			it_o->MoveNext();
		}else // ( *it == *it_o )
		{
			if( method == enum_selection_method_set || 
				method == enum_selection_method_add )
			{
				tmp_vec->Add		(it->current());
			}else
				it_o->current()->set_selected(false);

			it->MoveNext();
			it_o->MoveNext();
		}
	}

	// Add all elements from new select list if the selection mode is one of followings
	if( method == enum_selection_method_set || 
		method == enum_selection_method_add || 
		method == enum_selection_method_invert )
		while( it_o->IsValid() )
		{
			tmp_vec->Add			(it_o->current());
			it_o->current()->set_selected	(true);
			control_object			= it_o->current();

			it_o->MoveNext			();
		}

	// Add all elements in the old selection list that aren't intersect with the new list
	while( it->IsValid() )
	{
		tmp_vec->Add				(it->current());
		it->MoveNext				();
	}


	m_selection_list				= tmp_vec;

//	update_aim_history				( control_object );

	object_inspector_tab^ inspector_tab	= m_level_editor->get_object_inspector_tab();
	if(inspector_tab->Visible)
		inspector_tab->show_properties(selection_list());

	m_selection_changed();
}

object_list^ project::get_all_objects()
{
	object_list^ objects		= gcnew object_list;
	m_root_group->get_all_items(objects);
	return objects;
}

object_list^ project::selection_list()
{
	return m_selection_list;
}

project_item^ project::add_new_item(tool_base^ tool, System::String^ lib_name, float4x4 const& transform, u32 id, System::String^ path)
{
	object_base^ obj		= tool->create_object(lib_name);
	
	obj->set_transform		(transform);

	project_group^ parent	= m_root_group;

	if(path!=nullptr)
		parent	= find_group( path, false );
	else
		if(m_last_selected_group!=nullptr)
			parent			= m_last_selected_group;
		else
		if(m_last_selected_item!=nullptr)
			parent			= m_last_selected_item->m_parent;

	obj->assign_id				(id);
	project_item^ new_item =	parent->add_item(obj);

	return new_item;
}

project_item^ project::add_new_item(xray::configs::lua_config_value cfg)
{
	project_group^ parent		= m_root_group;

	if( cfg.value_exists("path") )
	{
		System::String^ pth		= gcnew System::String(cfg["path"]);
		parent					= find_group(pth, false );
	}else
	{
		if(m_last_selected_group!=nullptr)
			parent				= m_last_selected_group;
		else
			if(m_last_selected_item!=nullptr)
				parent			= m_last_selected_item->m_parent;
	}

	project_item^ result = parent->add_item	(cfg, true, enum_id_action_load);

	ASSERT( result!=nullptr && result->m_object_base!=nullptr);

	return result;
}

void project::remove_group(project_group^ pg)
{
	pg->m_parent->remove	(pg);
	pg->clear				();
	delete					pg;
}

void project::remove_item(project_item^ pi, bool b_destroy_object)
{
	ASSERT( !pi->m_object_base->get_selected(), "The items should be unselected before the removal." );
	ASSERT(!m_selection_list->Contains(pi->m_object_base), "The items should be unselected before the removal. There is dissincronisation between selection list and objects select state." );

	pi->m_parent->remove(pi);
	pi->clear			(b_destroy_object);
	delete				pi;
}

void project::remove(xray::configs::lua_config_value const& cfg)
{
	m_root_group->remove(cfg["project"]["objects_tree"] );
}

void project::remove(id_list^ ids)
{
	for each (System::UInt32^ it in ids)
		remove_item		(*it, true);
}

void project::remove_item(u32 id, bool b_destroy_object)
{	
	object_base^ obj	= object_base::object_by_id( id );
	ASSERT				(obj!=nullptr);	
	remove_item			(obj->m_owner_project_item, b_destroy_object);
}

void project::on_object_load(object_base^ object, xray::configs::lua_config_value const& t)
{
	System::String^ orig_name = gcnew System::String(t["name"]);

	// This part is needed only for paste operation. So it can be skiped for other cases.
	paste_temporary_data paste_data;
	paste_data.object		= object;
	paste_data.orig_name	= object->m_owner_project_item->get_path() + orig_name;
	m_paste_tmp_list->Add( %paste_data );

	configs::lua_config_value const& cfg_links = t["links"];

	configs::lua_config_value::const_iterator	i = cfg_links.begin( );
	configs::lua_config_value::const_iterator	e = cfg_links.end( );

	for ( ; i != e; ++i )
	{
		configs::lua_config_value const& t	= *i;
		System::String^ src_full_name			= gcnew System::String(t["src_name"]);

		link_temporary_data^ tmp_link = gcnew link_temporary_data();
		tmp_link->src_name		= src_full_name;
		tmp_link->src_member	= gcnew System::String(t["src_member"]);
		tmp_link->dst_obj		= object;
		tmp_link->dst_member	= gcnew System::String(t["member"]);
		m_link_tmp_list->Add(tmp_link);

		//m_link_storage->create_link( src_obj, gcnew System::String(t["src_member"]), object, gcnew System::String(t["member"]) );
	}
}

void project::on_object_save( object_base^ object, xray::configs::lua_config_value const& t)
{
	configs::lua_config_value cfg_links = t["links"];

	System::Collections::ArrayList^ links = m_link_storage->get_links( nullptr, nullptr, object, nullptr );
	for( int i = 0; i < links->Count; ++i )
	{
		System::String^ path = ((property_link^)links[i])->m_src_object->m_owner_project_item->get_path();
		path += ((property_link^)links[i])->m_src_object->get_name();
		unmanaged_string path_str_src(path);
		cfg_links[i]["src_name"] = path_str_src.c_str();

		unmanaged_string src_property_name(((property_link^)links[i])->m_src_property_name);
		cfg_links[i]["src_member"] = src_property_name.c_str();

		unmanaged_string dst_property_name(((property_link^)links[i])->m_dst_property_name);
		cfg_links[i]["member"] = dst_property_name.c_str();

	}
}

void project::show_connection_editor()
{
	object_base^ src		= nullptr;
	object_base^ dst		= nullptr;
	object_list^ lst		= selection_list();
	
	if(lst->Count>0)
		src					= lst[0];
	if(lst->Count>1)
		dst					= lst[1];

	show_connection_editor	(src, dst);
}

void project::show_connection_editor(object_base^ src, object_base^ dst)
{
	property_connection_editor^ tab	= m_level_editor->get_connection_editor();

	tab->set_src_object	(src);
	tab->set_dst_object	(dst);
	tab->Show			();
}

void project::show_object_inspector()
{
	object_list^ objects			= selection_list();
	show_object_inspector			(objects);
}

void project::show_object_inspector(object_list^ objects)
{
	object_inspector_tab^ inspector_tab	= m_level_editor->get_object_inspector_tab();

	inspector_tab->show_properties	(objects);
	
	if(!inspector_tab->Visible)
		m_level_editor->ide()->show_tab		(inspector_tab);
}

link_storage^ project::get_link_storage() 
{
	return m_link_storage;
}


void project::subscribe_on_selection_changed(selection_changed^ d)
{
	m_selection_changed	+= d;
}

} // namespace editor
} // namespace xray