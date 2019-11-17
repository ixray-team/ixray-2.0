////////////////////////////////////////////////////////////////////////////
//	Created		: 17.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "tool_light.h"
#include "object_light.h"
#include "light_tool_tab.h"
#include "property_container.h"
#include "library_object_tab.h"
#include "level_editor.h"
#include "editor_world.h"
#include <xray/editor/base/managed_delegate.h>

namespace xray{
namespace editor{

tool_light::tool_light(xray::editor::level_editor^ le)
:super(le, "light"),
m_preview_models_cnt(0)
{
	m_tool_tab 			= gcnew light_tool_tab(this);
	m_preview_models	= gcnew cli::array<render::visual_ptr*>(20);
	load_library		();
}

tool_light::~tool_light()							
{
	for( int i = 0; i < m_preview_models->Length; ++i )
		DELETE (m_preview_models[i]);
}

xray::editor::tool_tab^ tool_light::ui()
{
	return m_tool_tab;
}

void tool_light::save_library()
{
	fs::path_string name			= m_level_editor->get_editor_world().engine().get_resource_path();
	name += "/library/lights/lights.library";
	(*m_library_cfg)->save_as(name.c_str());
}

object_base^ tool_light::create_raw_object(System::String^ id)
{
	object_base^ result = nullptr;
	if(id=="base:dynamic")
	{
		result = gcnew object_dynamic_light(this);
	}else
	if(id=="base:static")
	{
		result = gcnew object_static_light(this);
	}
	ASSERT(result, "raw object not found", unmanaged_string(id).c_str());
	m_object_list->Add			(result);
	initialize_property_holder	(result); //tmp
	return result;
}

object_base^ tool_light::load_object(xray::configs::lua_config_value const& t)
{
	System::String^ library_name = gcnew System::String(t["lib_name"]);
	lib_item^ li				= get_library_item(library_name, true);
	
	System::String^ ll			= gcnew System::String( (*(li->m_config))["lib_name"]);
	object_base^ result			= create_raw_object(ll);
	result->load				(t);
	result->set_lib_name		(library_name);

	return						result;
}

object_base^ tool_light::create_object(System::String^ name)
{
	lib_item^ li				= get_library_item(name, true);
	System::String^ ln			= gcnew System::String((*(li->m_config))["lib_name"]);
	object_base^ result			= create_raw_object(ln);
	result->load				(*li->m_config);
	result->set_lib_name		(name);

	return						result;
}

void tool_light::destroy_object(object_base^ o)
{
	R_ASSERT					(m_object_list->Contains(o));
	m_object_list->Remove		(o);
	delete						o;
}

void tool_light::load_library()
{
	query_result_delegate* rq = NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &tool_light::on_library_files_loaded));
	xray::resources::query_resource(
		"resources/library/lights/lights.library",
		xray::resources::config_lua_class,
		boost::bind(&query_result_delegate::callback, rq, _1),
		g_allocator
		);

	//query_result_delegate* q = NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &tool_light::on_resource_loaded));

	//resources::request	resources[]	= {"editor/light_point", xray::resources::solid_visual_class};
	//resources::query_resources	(
	//	resources,
	//	array_size(resources),
	//	resources::query_callback(q, &query_result_delegate::callback),
	//	g_allocator
	//	);
}

void tool_light::on_resource_loaded( xray::resources::queries_result& data )
{
	R_ASSERT(!data.is_failed());

	for(u32 i=0; i<data.size(); ++i)
	{
		resources::query_result_for_user& r	= data[i];
		resources::unmanaged_resource_ptr gr = r.get_unmanaged_resource();
		on_preview_visual_ready( gr );
	}
}

void tool_light::on_preview_visual_ready(xray::resources::unmanaged_resource_ptr data)
{
	render::visual_ptr* v = NEW(render::visual_ptr)(static_cast_checked<render::visual*>(data.c_ptr()));

	m_preview_models[m_preview_models_cnt] = (v);
	++m_preview_models_cnt;
}

xray::render::visual_ptr tool_light::get_preview_model(u32 idx) 
{
	return *(m_preview_models[idx]);
}

tree_node^ make_hierrarchy(tree_node_collection^ root, System::String^ name)
{
	//string_path		tok;
	//pcstr p			= name;
	tree_node_collection^ current_root = root;
	xray::editor::controls::tree_node^		result;
	cli::array<System::String^>^ tokens = name->Split('/');

	for(int i=0; i<tokens->GetLength(0); ++i)
	{
		System::String^ folder		= tokens[i];
		array<tree_node^>^ node_arr = current_root->Find(folder ,false);
		if(node_arr->Length!=0)
		{
			current_root = node_arr[0]->Nodes;
		}else
		{

			result						= gcnew xray::editor::controls::tree_node(folder);
			result->ImageKey			= "group";
			result->SelectedImageKey	= "group";
			result->Name				= folder;
			current_root->Add			(result);
			current_root				= result->nodes;
		}
	}
	R_ASSERT		(result, "item[%s] already exist", name);
	return			result;
}

void tool_light::fill_tree_view(tree_node_collection^ nodes)
{
	nodes->Clear						();

	for each(lib_item^ itm in m_tool_library)
	{
		System::String^ short_name		= itm->m_lib_item_name;
		tree_node^	node				= make_hierrarchy(m_tool_tab->treeView->nodes, short_name);
		node->ImageKey					= "point_light";
		node->SelectedImageKey			= "point_light";
	}
	m_tool_tab->toolStripStatusLabel1->Text = System::String::Format("Total:{0} items", m_tool_library->Count);
}

void tool_light::on_library_loaded()
{
	super::on_library_loaded	();
	fill_tree_view				(m_tool_tab->treeView->nodes);
}

void tool_light::get_objects_list(xray::editor::names_list list, xray::editor::enum_list_mode mode)
{
	XRAY_UNREFERENCED_PARAMETER	( mode );

	//embedded types first
	list->Add(gcnew System::String("base:dynamic"));
	list->Add(gcnew System::String("base:static"));

	// from library
	for each(lib_item^ itm in m_tool_library)
		list->Add			(itm->m_lib_item_name);
}

void tool_light::on_make_new_library_object()
{
	m_level_editor->get_library_object_tab()->create_new(this);
	ide()->show_tab(m_level_editor->get_library_object_tab());
}

void tool_light::on_edit_library_object(System::String^ name)
{
	m_level_editor->get_library_object_tab()->edit_existing(this, name);
	ide()->show_tab(m_level_editor->get_library_object_tab());
}

} // namespace editor
} // namespace xray
