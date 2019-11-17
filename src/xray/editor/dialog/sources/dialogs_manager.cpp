//-------------------------------------------------------------------------------------------
//	Created		: 15.04.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "dialogs_manager.h"
#include "dialog_node_base.h"

#pragma unmanaged
using xray::dialog_editor::dialog_manager::dialogs_manager;
using xray::dialog_editor::dialog_node_base;

static xray::uninitialized_reference<dialogs_manager> g_dialogs_manager;
dialogs_manager* xray::dialog_editor::dialog_manager::create_dialogs_manager()
{
	XRAY_CONSTRUCT_REFERENCE(g_dialogs_manager, dialogs_manager)();
	return (&*g_dialogs_manager);
}

void xray::dialog_editor::dialog_manager::destroy_dialogs_manager()
{
	XRAY_DESTROY_REFERENCE(g_dialogs_manager);
}

dialogs_manager* xray::dialog_editor::dialog_manager::get_dialogs_manager()
{
	return (&*g_dialogs_manager);
}
//-------------------------------------------------------------------------------------------
//- class dialogs_manager -------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
dialogs_manager::dialogs_manager()
{
	m_return_exsisting_dialog = true;
	m_last_id = 0;
	m_dialogs_list = dialogs_list();
}

dialogs_manager::~dialogs_manager()
{
	m_dialogs_list.clear();
}

dialog_node_base* dialogs_manager::get_dialog_by_name(pcstr name)
{	
	if(!m_return_exsisting_dialog)
		return NULL;

	dialogs_list::iterator it = m_dialogs_list.find(name);
	if(it!=m_dialogs_list.end())
		return it->second;

	return NULL;
}

void dialogs_manager::add_dialog(dialog_node_base* dlg)
{
	dialogs_list::iterator it = m_dialogs_list.find(dlg->text());
	if(it==m_dialogs_list.end())
		m_dialogs_list.insert(std::pair<pcstr, dialog_node_base*>(dlg->text(), dlg));
}

void dialogs_manager::remove_dialog(pcstr name)
{
	dialogs_list::iterator it = m_dialogs_list.find(name);
	if(it!=m_dialogs_list.end())
		m_dialogs_list.erase(it);
}
