//-------------------------------------------------------------------------------------------
//	Created		: 16.03.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "dialog_editor_resources_cooker.h"
#include "dialog.h"
#include "dialog_graph_node_layout.h"

#pragma unmanaged
using xray::dialog_editor::dialog_resources;
using xray::dialog_editor::dialog_editor_resources_cooker;
using xray::dialog_editor::dialog;
using namespace xray::resources;
//-------------------------------------------------------------------------------------------
//- struct dialog_resources -----------------------------------------------------------------
//-------------------------------------------------------------------------------------------
dialog_resources::dialog_resources()
{
	m_dialog = NEW(dialog_node_base_ptr)();
	m_layout = NEW(dialog_graph_node_layout)();
}

dialog_resources::~dialog_resources()
{
	*m_dialog = NULL;
	DELETE(m_dialog);
	DELETE(m_layout);
}

void dialog_resources::save(pcstr path)
{
	R_ASSERT(m_dialog!=NULL);

	string512 file_path;
	sprintf_s(file_path, "%s.dlg", path);
	configs::lua_config_ptr const& cfg = configs::create_lua_config(file_path);
	get_dialog()->save(cfg->get_root());
	cfg->save();

	sprintf_s(file_path, "%s.lt", path);
	configs::lua_config_ptr const& cfg2 = configs::create_lua_config(file_path);
	m_layout->save(cfg2->get_root());
	cfg2->save();
}

dialog* dialog_resources::get_dialog()
{
	R_ASSERT(m_dialog!=NULL);
	dialog_node_base* obj = m_dialog->c_ptr();
	return dynamic_cast<dialog*>(obj);
}

//-------------------------------------------------------------------------------------------
//- class dialog_editor_resources_cooker ----------------------------------------------------
//-------------------------------------------------------------------------------------------
dialog_editor_resources_cooker::dialog_editor_resources_cooker()
:translate_query_cook(dialog_resources_class, reuse_true, use_user_thread_id)
{

}

void dialog_editor_resources_cooker::translate_query(xray::resources::query_result& parent)
{
	LOGI_INFO("dialog_editor_cook", "dialog_editor_cook::translate_query called");

	fs::path_string request_without_postfix = parent.get_requested_path();
	char const request_postfix[] = ".resource";
	u32 const length_without_postfix = request_without_postfix.length() - array_size(request_postfix) + 1;
	if(request_without_postfix.find(request_postfix)==length_without_postfix)
		request_without_postfix.set_length(length_without_postfix);

	fs::path_string layout_path;
	layout_path += "resources/dialogs/configs/";
	layout_path += request_without_postfix;
	layout_path += ".lt";

	request	arr[]	= {
		{ request_without_postfix.c_str(), dialog_class },
		{ layout_path.c_str(), config_lua_class },
	};

	query_resources(arr,
		2,
		boost::bind(&dialog_editor_resources_cooker::on_loaded, this, _1),
		parent.get_user_allocator(),
		NULL,
		0,
		&parent);

}

void dialog_editor_resources_cooker::on_loaded(queries_result & result)
{
	LOGI_INFO("dialog_editor_resources_cooker", "dialog_editor_resources_cooker::on_loaded called");

	query_result_for_cook * parent = result.get_parent_query();
	dialog_resources * dr = NEW(dialog_resources)();

	if(result[0].is_success())
	{
		*(dr->m_dialog) = static_cast_resource_ptr<dialog_node_base_ptr>(result[0].get_unmanaged_resource());
		if(result[1].is_success())
			dr->m_layout->load(static_cast_resource_ptr<configs::lua_config_ptr>(result[1].get_unmanaged_resource())->get_root());
	}
	else
	{
		parent->finish_query(result_error);
		return;
	}

	parent->set_unmanaged_resource(dr, resources::memory_type_non_cacheable_resource, sizeof(dialog_resources));
	parent->finish_query(result_success);
}

void dialog_editor_resources_cooker::delete_resource (unmanaged_resource * resource)
{
	LOGI_INFO	("dialog_editor_resources_cooker", "dialog_editor_resources_cooker::delete_unmanaged called");
	DELETE		(resource);
}
