//-------------------------------------------------------------------------------------------
//	Created		: 15.04.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "game_dialog_cooker.h"
#include "game_dialog.h"
#include "dialogs_manager.h"

#pragma unmanaged
using xray::dialog_editor::game_dialog_cooker;
using xray::dialog_editor::game_dialog;
using namespace xray::resources;
using xray::dialog_editor::dialog_manager::dialogs_manager;

game_dialog_cooker::game_dialog_cooker()
:translate_query_cook(game_dialog_class, reuse_true, use_user_thread_id)
{
}

void game_dialog_cooker::translate_query(xray::resources::query_result& parent)
{
	LOGI_INFO("game_dialog_cooker", "game_dialog_cooker::translate_query called");
	dialog_node_base* dlg = dialog_manager::get_dialogs_manager()->get_dialog_by_name(parent.get_requested_path());
	if(dlg!=NULL)
	{
		parent.set_unmanaged_resource(dlg, dlg->memory_usage());
		parent.finish_query(result_success);
		return;
	}

	fs::path_string dialog_path;
	dialog_path.append("resources/dialogs/configs/");
	dialog_path.append(parent.get_requested_path());
	dialog_path.append(".dlg");
	query_resource(
		dialog_path.c_str(),
		config_lua_class,
		boost::bind(&game_dialog_cooker::on_loaded, this, _1),
		parent.get_user_allocator(),
		NULL,
		0,
		&parent
		);
}

void game_dialog_cooker::on_loaded(queries_result& result)
{
	LOGI_INFO("game_dialog_cooker", "game_dialog_cooker::on_loaded called");

	query_result_for_cook* parent = result.get_parent_query();

	if(result[0].is_success())
	{
		game_dialog* d = NEW(game_dialog)(result);
		parent->set_unmanaged_resource(d, resources::memory_type_non_cacheable_resource, sizeof(game_dialog));
	}
	else
	{
		parent->finish_query(result_error);
		return;
	}
}

void game_dialog_cooker::delete_resource(unmanaged_resource* res)
{
	LOGI_INFO("game_dialog_cooker", "game_dialog_cooker::delete_unmanaged called");
	DELETE(res);
}
