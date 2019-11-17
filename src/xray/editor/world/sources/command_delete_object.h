////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef COMMAND_DELETE_OBJECT_H_INCLUDED
#define COMMAND_DELETE_OBJECT_H_INCLUDED

#include "project_defines.h"
#include <xray/configs_lua_config.h>

namespace xray {
namespace editor {

ref class level_editor;

public ref class command_delete_selected_objects :	public xray::editor_base::command
{
public:

	command_delete_selected_objects	( level_editor^ le );

	virtual bool commit		() override;
	virtual void rollback	() override{}

private:
	level_editor^		m_level_editor;
}; // class command_delete_selected_objects

public ref class command_delete_object :	public xray::editor_base::command
{
public:

	command_delete_object	( level_editor^ le, int id );

	virtual bool commit		() override;
	virtual void rollback	() override{}

private:
	int						m_id;		
	level_editor^			m_level_editor;
}; // class command_delete_selected_objects

public ref class command_delete_object_impl :	public xray::editor_base::command
{
public:

						command_delete_object_impl	( level_editor^ le, id_list^ objects_id );
	virtual				~command_delete_object_impl	();

	virtual bool		commit		() override;
	virtual void		rollback	() override;

private:	// Data for execute

	level_editor^	m_level_editor;
	id_list^		m_ids;

private:	// Data for undo

	object_list^				m_objects;
	configs::lua_config_ptr*	m_configs;

}; // class command_delete_object_impl

}// namespace editor
}// namespace xray


#endif // #ifndef COMMAND_DELETE_OBJECT_H_INCLUDED