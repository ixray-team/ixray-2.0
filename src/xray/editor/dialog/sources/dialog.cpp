//-------------------------------------------------------------------------------------------
//	Created		: 16.03.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "dialog.h"
#include "dialog_node_base.h"
#include "dialog_phrase.h"
#include "game_dialog.h"
#include "dialog_operation.h"
#include "dialog_precondition.h"
#include "dialog_action.h"
#include "dialogs_manager.h"

#pragma unmanaged
using xray::dialog_editor::dialog;
using xray::dialog_editor::dialog_node_base;
using xray::dialog_editor::dialog_phrase;
using xray::dialog_editor::game_dialog;
using xray::dialog_editor::dialog_action;
using xray::dialog_editor::dialog_manager::dialogs_manager;

dialog::dialog()
{
	m_last_id = 0;
	m_phrases = phrases_list();
	m_phrases.reserve(30*sizeof(dialog_node_base_ptr*));
}

void delete_preconditions(xray::dialog_editor::dialog_expression* root)
{
	xray::dialog_editor::dialog_precondition* root_prec = dynamic_cast<xray::dialog_editor::dialog_precondition*>(root);
	if(root_prec)
		DELETE(root_prec);
	else
	{
		xray::dialog_editor::dialog_operation* root_oper = dynamic_cast<xray::dialog_editor::dialog_operation*>(root);
		if(root_oper)
		{
			xray::dialog_editor::vector<xray::dialog_editor::dialog_expression*>::iterator prec_vec_iter = root_oper->childs()->begin();
			for(; prec_vec_iter!=root_oper->childs()->end(); ++prec_vec_iter)
				delete_preconditions(*prec_vec_iter);
			
			DELETE(root_oper);
		}
	}
}

dialog::~dialog()
{
	if(m_text!=NULL)
		dialog_manager::get_dialogs_manager()->remove_dialog(m_text);

	phrases_list::iterator i = m_phrases.begin();
	for(; i!=m_phrases.end(); ++i)
	{
		const links_list* links_lst = i->c_ptr()->get_links();
		dialog_link* cur_lnk = links_lst->front();
		while(cur_lnk!=NULL)
		{
			vector<dialog_action*>::iterator actions_iter = cur_lnk->actions()->begin();
			for(; actions_iter!=cur_lnk->actions()->end(); ++actions_iter)
				DELETE(*actions_iter);

			if(cur_lnk->root_precondition()!=NULL)
				delete_preconditions(cur_lnk->root_precondition());

			dialog_link* tmp_lnk = links_lst->get_next_of_object(cur_lnk);
			DELETE(cur_lnk);
			cur_lnk = tmp_lnk;
		}

		if(!(dynamic_cast<dialog_phrase*>(i->c_ptr())))
			dialog_manager::get_dialogs_manager()->remove_dialog(i->c_ptr()->text());
		else
			DELETE(i->c_ptr());
	}
	FREE(m_text);
}

void dialog::save(xray::configs::lua_config_value cfg)
{
//	saving nodes
	phrases_list::const_iterator i = m_phrases.begin();
	for(; i!=m_phrases.end(); ++i)
	{
		u32 id = (*i)->id();
		xray::configs::lua_config_value val = cfg["dialog"]["nodes"][id];
		val["id"] = id;
		val["is_root"] = (*i)->is_root();
		dialog_node_base* n = i->c_ptr();
		if(dynamic_cast<dialog*>(n))
		{
			val["is_dialog"] = true;
			val["string_table"] = (*i)->text();
		}
		else
		{
			val["is_dialog"] = false;
			val["string_table"] = dynamic_cast<dialog_phrase*>(n)->string_table_id();
		}
	}
//	saving links
	u32 link_id = 0;
	i = m_phrases.begin();
	for(; i!=m_phrases.end(); ++i)
	{
		const links_list* lnks = (*i)->get_links();
		dialog_link* cur_lnk = lnks->front();
		while(cur_lnk!=NULL)
		{
			dialog_node_base_ptr* p = get_node_by_id(cur_lnk->child()->id());
			if(p!=NULL && p->c_ptr())
			{
				xray::configs::lua_config_value val = cfg["dialog"]["links"][link_id];
				val["src_id"] = (*i)->id();
				val["dst_id"] = (cur_lnk->child())->id();
				cur_lnk->save(val);
			}
			cur_lnk = lnks->get_next_of_object(cur_lnk);
			++link_id;
		}
	}
}

void dialog::load(xray::configs::lua_config_value const& cfg)
{
	associative_vector<u32, dialog_node_base*, vector> vec = associative_vector<u32, dialog_node_base*, vector>();
//	loading nodes	
	if(cfg.value_exists("dialog") && cfg["dialog"].value_exists("nodes"))
	{
		configs::lua_config::const_iterator i = cfg["dialog"]["nodes"].begin();
		for(; i!=cfg["dialog"]["nodes"].end(); ++i) 
		{
			u32 id = (u32)(*i)["id"];
			if((bool)(*i)["is_dialog"]==true)
			{
				dialog* nd = new_dialog();
				nd->set_id(id);
				nd->set_text((pcstr)(*i)["string_table"]);
				nd->set_is_root((bool)(*i)["is_root"]);
				vec.insert(std::pair<u32, dialog_node_base*>(id, nd));
			}
			else
			{
				dialog_phrase* np = new_phrase();
				np->set_id(id);
				np->set_string_table_id((pcstr)(*i)["string_table"]);
				np->set_is_root((bool)(*i)["is_root"]);
				vec.insert(std::pair<u32, dialog_node_base*>(id, np));
			}
		}
		m_last_id = vec.rbegin()->second->id();
	}
//	loading links
	if(cfg.value_exists("dialog") && cfg["dialog"].value_exists("links"))
	{
		configs::lua_config::const_iterator	il = cfg["dialog"]["links"].begin();
		for(; il!=cfg["dialog"]["links"].end(); ++il) 
		{
			associative_vector<u32, dialog_node_base*, vector>::const_iterator n1 = vec.find((u32)(*il)["src_id"]);
			associative_vector<u32, dialog_node_base*, vector>::const_iterator n2 = vec.find((u32)(*il)["dst_id"]);
			if(n1!=vec.end() && n2!=vec.end())
			{
				dialog_link* lnk = n1->second->new_link(n2->second, false);
				lnk->load(*il);
			}
		}
	}
}

dialog_phrase* dialog::new_phrase()
{
	dialog_phrase* n = NEW(dialog_phrase)();
	n->set_id(++m_last_id);
	dialog_node_base_ptr p = dialog_node_base_ptr(n);
	m_phrases.push_back(p);
	return n;
}

dialog* dialog::new_dialog()
{
	dialog* n = NEW(dialog)();
	n->set_id(++m_last_id);
	dialog_node_base_ptr p = dialog_node_base_ptr(n);
	m_phrases.push_back(p);
	return n;
}

void dialog::remove_node(xray::dialog_editor::dialog_node_base_ptr n)
{
	phrases_list::iterator i = std::remove(m_phrases.begin(), m_phrases.end(), n);
	m_phrases.erase(i, m_phrases.end());
}

void dialog::remove_node(u32 id)
{
	dialog_node_base_ptr* p = get_node_by_id(id);
	R_ASSERT(p!=NULL);
	if(p->c_ptr())
		remove_node(p->c_ptr());
}

void dialog::add_node(xray::dialog_editor::dialog_node_base_ptr n)
{
	dialog_node_base_ptr* found_node_ptr = get_node_by_id(n->id());
	if(found_node_ptr==NULL)
	{
		m_phrases.push_back(n);
		if(n->id()>=m_last_id)
			m_last_id = n->id();
	}
	else if((*found_node_ptr)!=n)
	{
		n->set_id(++m_last_id);
		m_phrases.push_back(n);
	}
}

xray::dialog_editor::dialog_node_base_ptr* dialog::get_node_by_id(u32 id)
{
	phrases_list::iterator i = m_phrases.begin();
	for(; i!=m_phrases.end(); ++i)
		if((*i)->id()==id)
			return &(*i);

	return NULL;
}

void dialog::set_text(pcstr new_text)
{
	m_text = strings::duplicate(g_allocator, new_text);
}

pcstr dialog::text()
{
	return m_text;
}

void dialog::clear()
{
	m_text = NULL;
	m_phrases.clear();
}
