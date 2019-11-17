////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "command_delete_object.h"

#include "level_editor.h"
#include "tool_base.h"
#include "object_base.h"
#include "project.h"
#include "command_select.h"
#include "project_items.h"

namespace xray {
namespace editor {

command_delete_selected_objects::command_delete_selected_objects( level_editor^ le ):
m_level_editor(le)
{
}

bool command_delete_selected_objects::commit		()
{
	id_list^ ids		= gcnew id_list;
	object_list objects	= m_level_editor->get_project()->selection_list();

	for each(object_base^ o in objects)
		ids->Add( o->id() );

	m_level_editor->get_command_engine()->run( gcnew command_select(m_level_editor) );

	return	m_level_editor->get_command_engine()->run( gcnew command_delete_object_impl(m_level_editor, ids ));
}

command_delete_object::command_delete_object( level_editor^ le, int id ):
m_level_editor	( le ),
m_id			( id )
{
}

bool command_delete_object::commit		()
{
		
	m_level_editor->get_command_engine()->run( gcnew command_select(m_level_editor) );

	id_list^ ids		= gcnew id_list;
	ids->Add			( m_id );

	return	m_level_editor->get_command_engine()->run( gcnew command_delete_object_impl(m_level_editor, ids ));
}

command_delete_object_impl::command_delete_object_impl	(level_editor^ le, id_list^ objects_id ):
m_level_editor		( le )
{
	m_ids		= objects_id;
	m_objects	= gcnew object_list;
	m_configs	= NEW (configs::lua_config_ptr)();
	*m_configs	= configs::create_lua_config( "d:\\asd.lua");
}

command_delete_object_impl::~command_delete_object_impl()
{
	DELETE (m_configs);
}

bool command_delete_object_impl::commit		()
{
	xray::configs::lua_config_value value = (*m_configs)->get_root();

	u32 index = 0;
	id_list^ idlist = m_ids;
	for each (System::UInt32^ it in idlist)
	{
 		object_base^ o		= object_base::object_by_id(*it);
		m_objects->Add		(o);
		m_level_editor->get_project()->save_to_config(value, o, true );
		m_level_editor->get_project()->remove_item( o->m_owner_project_item, true ); 
		++index;
	}
	return true;
}

void command_delete_object_impl::rollback	()
{
	configs::lua_config_value value = (*m_configs)->get_root();

	m_level_editor->get_project()->load( value, true );
}

}// namespace editor
}// namespace xray
