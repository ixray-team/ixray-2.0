//-------------------------------------------------------------------------------------------
//	Created		: 19.04.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "game_dialog.h"
#include "dialog_node_base.h"
#include "dialog_phrase.h"
#include "dialog_action.h"
#include "dialog_precondition.h"
#include "dialog_operation_and.h"
#include "dialog_operation_or.h"
#include "dialog_operation_not.h"
#include <xray/buffer_vector.h>
#include "dialogs_manager.h"

#pragma unmanaged
using xray::dialog_editor::game_dialog;
using xray::buffer_vector;
using namespace xray::resources;
using xray::dialog_editor::dialog_manager::dialogs_manager;

game_dialog::game_dialog(queries_result& result)
{
	query_result_for_cook* parent = result.get_parent_query();

	if(result[0].is_success())
	{
		m_parent_cfg_ptr = static_cast_resource_ptr<configs::lua_config_ptr>(result[0].get_unmanaged_resource());
		configs::lua_config_value cfg = m_parent_cfg_ptr->get_root();

		R_ASSERT(cfg.value_exists("dialog"));
		R_ASSERT(cfg["dialog"].value_exists("nodes"));

		m_roots_counter = 0;
		m_request_counter = 0;
		m_created_nodes = created_nodes_type();

		u32 const total_node_count = cfg["dialog"]["nodes"].size();
		m_request = (request*)ALLOCA(total_node_count*sizeof(request));

		string512 file_path;
		pcstr this_name = result[0].get_requested_path();
		for(int i=0; i<3; ++i)
			this_name = strings::get_token(this_name, file_path, strings::length(this_name), '/');

		u32 sz = calculate_size(cfg["dialog"]);
		sz += math::align_up(strings::length(this_name) + 1, u32(4));

		R_ASSERT(m_roots_counter!=0);
		m_buffer_raw = MALLOC(sz, "game_dialog");
		m_buffer = mutable_buffer(m_buffer_raw, sz);
		m_text = new_text_from_buffer(this_name);

		if(m_request_counter > 0)
		{
			m_wait_for_subdialogs = true;
			query_resources(m_request, 
					m_request_counter, 
					boost::bind(&game_dialog::on_subdialogs_loaded, this, _1),
					g_allocator,
					NULL,
					0,
					parent); 

		}
		else
		{
			m_wait_for_subdialogs = false;
			on_subdialogs_loaded(result);
		}
		dialog_manager::get_dialogs_manager()->add_dialog(this);
	}
	else
	{
		parent->finish_query(cook_base::result_error);
		return;
	}
}

game_dialog::~game_dialog()
{
	dialog_manager::get_dialogs_manager()->remove_dialog(m_text);
	for(int i=m_roots_counter-1; i>=0; --i)
	{
		delete_childs(m_roots[i].c_ptr());
		if(dynamic_cast<dialog_phrase*>(m_roots[i].c_ptr()))
			dynamic_cast<dialog_phrase*>(m_roots[i].c_ptr())->~dialog_phrase();
		else
			m_roots[i] = NULL;
	}

	FREE(m_buffer_raw);
}

void game_dialog::on_subdialogs_loaded(xray::resources::queries_result& result)
{
	configs::lua_config_value parent_cfg = m_parent_cfg_ptr->get_root();

	if(m_wait_for_subdialogs)
	{
		R_ASSERT(!result.is_failed());	

		if(result.is_failed())
			return;

		if(result.size() > 0)
		{
			for(u16 i=0;i<result.size();i++)
			{
				CURE_ASSERT(result[i].is_success(), return);

				dialog_node_base_ptr d = static_cast_resource_ptr<dialog_node_base_ptr>(result[i].get_unmanaged_resource());
				
//				game_dialog* dlg = dynamic_cast<game_dialog*>(d.get());
				dialog_node_base* dlg = dynamic_cast<dialog_node_base*>(d.c_ptr());
				pcstr dlg_name = dlg->text();

				configs::lua_config::const_iterator parent_iter = parent_cfg["dialog"]["nodes"].begin();
				for(; parent_iter!=parent_cfg["dialog"]["nodes"].end(); ++parent_iter) 
				{
					if((bool)(*parent_iter)["is_dialog"]==true)
					{
						string512 node_name;
						pcstr str_id = (*parent_iter)["string_table"];
						sprintf_s(node_name, sizeof(node_name), "%s.dlg", str_id);
						if(strings::compare(dlg_name, node_name)==0)
						{
							dlg->set_id((*parent_iter)["id"]);
							m_created_nodes.insert(std::pair<u32, dialog_node_base*>((*parent_iter)["id"], dlg));
						}
					}
				}
			}
		}
	}

	create_roots(parent_cfg["dialog"]["nodes"]);
	if(parent_cfg["dialog"].value_exists("links"))
	{
		for(u32 i = 0; i<m_roots_counter; ++i)
			create_childs(m_roots[i].c_ptr(), parent_cfg["dialog"]);
	}

	result.get_parent_query()->finish_query(cook_base::result_success);
	m_request = NULL;
}


u32 calculate_preconditions_size(xray::configs::lua_config_value const& cfg)
{
	u32 sz = 0;
	int t = (int)cfg["type"];
	switch(t)
	{
	case 0: 
		{
			sz += sizeof(xray::dialog_editor::dialog_precondition);
			sz += xray::math::align_up(xray::strings::length((pcstr)cfg["param1"]) + 1, u32(4));
			break;
		}
	case 1: 
		{
			if(cfg.value_exists("childs"))
			{
				sz += sizeof(xray::dialog_editor::dialog_operation_and);
				xray::configs::lua_config::const_iterator i = cfg["childs"].begin();
				for(; i!=cfg["childs"].end(); ++i) 
					sz += calculate_preconditions_size(*i);
			}
			break;
		}
	case 2: 
		{
			if(cfg.value_exists("childs"))
			{
				sz += sizeof(xray::dialog_editor::dialog_operation_or);
				xray::configs::lua_config::const_iterator i = cfg["childs"].begin();
				for(; i!=cfg["childs"].end(); ++i) 
					sz += calculate_preconditions_size(*i);
			}
			break;
		}
	case 3: 
		{
			if(cfg.value_exists("childs"))
			{
				sz += sizeof(xray::dialog_editor::dialog_operation_not);
				xray::configs::lua_config::const_iterator i = cfg["childs"].begin();
				for(; i!=cfg["childs"].end(); ++i) 
					sz += calculate_preconditions_size(*i);
			}
			break;
		}
	}
	return sz;
}

u32 game_dialog::calculate_size(xray::configs::lua_config_value const& cfg)
{
	u32 dialog_size = 0;
	configs::lua_config::const_iterator i = cfg["nodes"].begin();
	for(; i!=cfg["nodes"].end(); ++i) 
	{
		if((bool)(*i)["is_root"]==true)
		{
			dialog_size += sizeof(dialog_node_base_ptr);
			++m_roots_counter;
		}
		
		if((bool)(*i)["is_dialog"]==true)
		{
			request rq = request();
			rq.path = (*i)["string_table"];
			rq.id = game_dialog_class;
			m_request[m_request_counter] = rq;
			++m_request_counter;
		}
		else
		{
			dialog_size += sizeof(xray::dialog_editor::dialog_phrase);
			dialog_size += math::align_up(strings::length((pcstr)(*i)["string_table"]) + 1, u32(4));
		}
	}

	if(cfg.value_exists("links"))
	{
		i = cfg["links"].begin();
		for(; i!=cfg["links"].end(); ++i) 
		{
			dialog_size += sizeof(dialog_link);
			if((*i).value_exists("actions"))
			{
				configs::lua_config::const_iterator i_a = (*i)["actions"].begin();
				for(; i_a!=(*i)["actions"].end(); ++i_a) 
				{
					dialog_size += sizeof(dialog_action);
					dialog_size += math::align_up(strings::length((pcstr)(*i_a)["param1"]) + 1, u32(4));
				}
			}
			if((*i).value_exists("preconditions"))
				dialog_size += calculate_preconditions_size((*i)["preconditions"]);
		}
	}

	return dialog_size;
}

pstr const game_dialog::new_text_from_buffer(pcstr const string_from_config)
{
	pstr const new_text = static_cast<pstr>(m_buffer.c_ptr());
	u32 const buffer_size = (strings::length(string_from_config) + 1)*sizeof(char);
	memory::copy(new_text, buffer_size, string_from_config, buffer_size);
	m_buffer += math::align_up(buffer_size, u32(4));
	return new_text;
}

xray::dialog_editor::dialog_node_base* game_dialog::new_node(xray::configs::lua_config_value const& cfg)
{
	created_nodes_type::iterator created_nodes_iter = m_created_nodes.find(cfg["id"]);
	if(created_nodes_iter==m_created_nodes.end())
	{
		dialog_phrase* const ph = static_cast<dialog_phrase*>(m_buffer.c_ptr());
		m_buffer += sizeof(dialog_phrase);
		new (ph) dialog_phrase(new_text_from_buffer(cfg["string_table"]));
		ph->set_id(cfg["id"]);
		m_created_nodes.insert(std::pair<u32, dialog_node_base*>(cfg["id"], ph));
		return ph;
	}
	else
		return created_nodes_iter->second;
}

void game_dialog::create_roots(xray::configs::lua_config_value const& cfg)
{
	m_roots = static_cast<dialog_node_base_ptr*>(m_buffer.c_ptr());
	for(u32 j = 0; j<m_roots_counter; ++j)
	{
		new (m_buffer.c_ptr()) dialog_node_base_ptr();
		m_buffer += sizeof(dialog_node_base_ptr);
	}

	u32 idx = 0;
	configs::lua_config::const_iterator i = cfg.begin();
	for(; i!=cfg.end(); ++i) 
	{
		if((bool)(*i)["is_root"]==true)
		{
			dialog_node_base* nd = new_node(*i);
			nd->set_is_root(true);
			m_roots[idx] = nd;
			++idx;
		}
	}
}

void game_dialog::create_childs(dialog_node_base* parent, xray::configs::lua_config_value const& cfg)
{
	configs::lua_config::const_iterator links_iter = cfg["links"].begin();
	for(; links_iter!=cfg.end(); ++links_iter) 
	{
		if((u32)(*links_iter)["src_id"]==parent->id())
		{
			dialog_link* const dl = static_cast<dialog_link*>(m_buffer.c_ptr());
			m_buffer += sizeof(dialog_link);

			dialog_expression* root_prec = NULL;
			if((*links_iter).value_exists("preconditions"))
				root_prec = create_precondition((*links_iter)["preconditions"]);

			vector<dialog_action*> act_vec = vector<dialog_action*>();
			if((*links_iter).value_exists("actions"))
			{
				configs::lua_config::const_iterator i_a = (*links_iter)["actions"].begin();
				for(; i_a!=(*links_iter)["actions"].end(); ++i_a) 
				{
					dialog_action* const act = static_cast<dialog_action*>(m_buffer.c_ptr());
					m_buffer += sizeof(dialog_action);
					new (act) dialog_action((*i_a)["func"], new_text_from_buffer((*i_a)["param1"]));
					act_vec.push_back(act);
				}
			}

			dialog_node_base* chld = NULL;
			created_nodes_type::iterator created_nodes_iter = m_created_nodes.find((*links_iter)["dst_id"]);
			if(created_nodes_iter!=m_created_nodes.end())
				chld = created_nodes_iter->second;
			else
			{
				configs::lua_config::const_iterator nodes_iter = cfg["nodes"].begin();
				for(; nodes_iter!=cfg["nodes"].end(); ++nodes_iter) 
				{
					if((u32)(*nodes_iter)["id"]==(u32)(*links_iter)["dst_id"])
					{
						chld = new_node(*nodes_iter);
						chld->set_is_root(false);
						create_childs(chld, cfg);
						break;
					}
				}
			}

			new (dl) dialog_link(chld);
			dl->set_root_precondition(root_prec);
			vector<dialog_action*>::iterator act_vec_iter = act_vec.begin();
			for(; act_vec_iter!=act_vec.end(); ++act_vec_iter) 
				dl->add_action(*act_vec_iter);

			parent->add_link(dl);
		}
	}
}

xray::dialog_editor::dialog_expression* game_dialog::create_precondition(xray::configs::lua_config_value const& cfg)
{
	int t = (int)cfg["type"];
	switch(t)
	{
	case 0: 
		{
			dialog_precondition* const pr = static_cast<dialog_precondition*>(m_buffer.c_ptr());
			m_buffer += sizeof(dialog_precondition);
			new (pr) dialog_precondition(cfg["func"], new_text_from_buffer(cfg["param1"]));
			return pr;
		}
	case 1: 
		{
			if(cfg.value_exists("childs"))
			{
				dialog_operation_and* const op_and = new (m_buffer.c_ptr()) dialog_operation_and();
				m_buffer += sizeof(dialog_operation_and);
				xray::configs::lua_config::const_iterator i = cfg["childs"].begin();
				for(; i!=cfg["childs"].end(); ++i) 
					op_and->add_child(create_precondition(*i));

				return op_and;
			}
			break;
		}
	case 2: 
		{
			if(cfg.value_exists("childs"))
			{
				dialog_operation_or* const op_or = new (m_buffer.c_ptr()) dialog_operation_or();
				m_buffer += sizeof(dialog_operation_or);
				xray::configs::lua_config::const_iterator i = cfg["childs"].begin();
				for(; i!=cfg["childs"].end(); ++i) 
					op_or->add_child(create_precondition(*i));

				return op_or;
			}
			break;
		}
	case 3: 
		{
			if(cfg.value_exists("childs"))
			{
				dialog_operation_not* const op_not = new (m_buffer.c_ptr()) dialog_operation_not();
				m_buffer += sizeof(dialog_operation_not);
				xray::configs::lua_config::const_iterator i = cfg["childs"].begin();
				for(; i!=cfg["childs"].end(); ++i) 
					op_not->add_child(create_precondition(*i));

				return op_not;
			}
			break;
		}
	}
	return NULL;
}

void game_dialog::delete_childs(dialog_node_base* parent)
{
	m_created_nodes.erase(parent->id());
	const links_list* links_lst = parent->get_links();
	dialog_link* cur_lnk = links_lst->front();
	while(cur_lnk!=NULL)
	{
		dialog_node_base* chld = cur_lnk->child();
		created_nodes_type::iterator created_nodes_iter = m_created_nodes.find(chld->id());
		if(created_nodes_iter!=m_created_nodes.end())
			delete_childs(chld);

		vector<dialog_action*>::iterator actions_iter = cur_lnk->actions()->begin();
		for(; actions_iter!=cur_lnk->actions()->end(); ++actions_iter)
			(*actions_iter)->~dialog_action();

		if(cur_lnk->root_precondition()!=NULL)
			delete_preconditions(cur_lnk->root_precondition());

		//bool parent_is_root = false;
		//for(u32 k=0; k<m_roots_counter; ++k)
		//	if(m_roots[k]->id()==chld->id())
		//		parent_is_root = true;

		if(!chld->is_root())
		{
			if(dynamic_cast<dialog_phrase*>(chld))
				dynamic_cast<dialog_phrase*>(chld)->~dialog_phrase();
			else
				chld = NULL;
		}

		dialog_link* tmp_lnk = links_lst->get_next_of_object(cur_lnk);
		cur_lnk->~dialog_link();
		cur_lnk = tmp_lnk;
	}

}

void game_dialog::delete_preconditions(xray::dialog_editor::dialog_expression* root)
{
	dialog_precondition* root_prec = dynamic_cast<dialog_precondition*>(root);
	if(root_prec)
		root_prec->~dialog_precondition();
	else
	{
		dialog_operation* root_oper = dynamic_cast<dialog_operation*>(root);
		if(root_oper)
		{
			vector<dialog_expression*>::iterator prec_vec_iter = root_oper->childs()->begin();
			for(; prec_vec_iter!=root_oper->childs()->end(); ++prec_vec_iter)
				delete_preconditions(*prec_vec_iter);

			if(dynamic_cast<dialog_operation_and*>(root_oper))
				dynamic_cast<dialog_operation_and*>(root_oper)->~dialog_operation_and();
			else if(dynamic_cast<dialog_operation_or*>(root_oper))
				dynamic_cast<dialog_operation_or*>(root_oper)->~dialog_operation_or();
			else if(dynamic_cast<dialog_operation_not*>(root_oper))
				dynamic_cast<dialog_operation_not*>(root_oper)->~dialog_operation_not();
		}
	}
}

xray::dialog_editor::dialog_node_base_ptr const& game_dialog::get_root(u32 index) const
{
	ASSERT(index<m_roots_counter);
	return m_roots[index];
}