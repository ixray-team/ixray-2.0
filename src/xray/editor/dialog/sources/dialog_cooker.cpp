//-------------------------------------------------------------------------------------------
//	Created		: 15.04.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "dialog_cooker.h"
#include "dialog.h"
#include "dialogs_manager.h"
#include "dialog_link.h"
#include "dialog_node_visitors.h"
#include "game_dialog.h"

#pragma unmanaged
using xray::dialog_editor::dialog_cooker;
using xray::dialog_editor::dialog;
using xray::dialog_editor::dialog_manager::dialogs_manager;
using xray::dialog_editor::dialog_link;
using namespace xray::resources;
using xray::dialog_editor::dialog_node_gatherer_visitor;

using xray::dialog_editor::game_dialog;

dialog_cooker::dialog_cooker()
:translate_query_cook(dialog_class, reuse_true, use_user_thread_id)
{
}

void dialog_cooker::translate_query(xray::resources::query_result& parent)
{
	LOGI_INFO("dialog_cooker", "dialog_cooker::translate_query called");

	dialog_node_base* dlg = dialog_manager::get_dialogs_manager()->get_dialog_by_name(parent.get_requested_path());
	if(dlg!=NULL)
	{
		parent.set_unmanaged_resource(dlg, dlg->memory_usage());
		parent.finish_query(result_success);
		return;
	}

	fs::path_string dialog_path;
	dialog_path += "resources/dialogs/configs/";
	dialog_path += parent.get_requested_path();
	dialog_path += ".dlg";

	query_resource(
		dialog_path.c_str(),
		config_lua_class,
		boost::bind(&dialog_cooker::on_loaded, this, _1),
		parent.get_user_allocator(),
		NULL,
		0,
		&parent
		);
}

void dialog_cooker::on_loaded(queries_result& result)
{
	LOGI_INFO("dialog_cooker", "dialog_cooker::on_loaded called");

	query_result_for_cook* parent = result.get_parent_query();

	if(result[0].is_success())
	{
		configs::lua_config_ptr cfg = static_cast_resource_ptr<configs::lua_config_ptr>(result[0].get_unmanaged_resource());

		dialog* dlg = NEW(dialog)();
		string512 file_path;

		{
			pcstr val = result[0].get_requested_path();
			for(int i=0; i<3; ++i)
				val = strings::get_token(val, file_path, strings::length(val), '/');

			val = strings::get_token(val, file_path, strings::length(val), '.');
		}

		dlg->set_text(file_path);
		dlg->load(cfg->get_root());
		dialog_manager::get_dialogs_manager()->add_dialog(dlg);

		dialog_node_gatherer_visitor* visitor = NEW(dialog_node_gatherer_visitor)();
		const vector<dialog_node_base_ptr>* vec = dlg->get_phrases();
		vector<dialog_node_base_ptr>::const_iterator i = vec->begin();
		for(; i!=vec->end(); ++i)
			(*i)->accept(visitor);

		parent->set_unmanaged_resource(dlg, resources::memory_type_non_cacheable_resource, sizeof(dialog));
		visitor->request_subresources(parent);
	}
	else
	{
		parent->finish_query(result_error);
		return;
	}
}

void dialog_cooker::delete_resource(unmanaged_resource* res)
{
	LOGI_INFO	("dialog_cooker", "dialog_cooker::delete_unmanaged called");
	DELETE		(res);
}
