////////////////////////////////////////////////////////////////////////////
//	Created		: 21.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "command_paste_objects.h"

#include "level_editor.h"
#include "tool_base.h"
#include "object_base.h"
#include "project.h"
#include "command_select.h"
#include "project_items.h"

namespace xray {
namespace editor {

command_paste_objects::command_paste_objects	( level_editor^ le, xray::configs::lua_config_value const& cfg ):
m_level_editor		( le ),
m_config_is_valis	( true )
{
	m_cfg			= NEW(configs::lua_config_value)(cfg);
}

command_paste_objects::~command_paste_objects()
{
	DELETE (m_cfg);
}

bool command_paste_objects::commit()
{
	if( !m_config_is_valis )
		return false;

	id_list			ids;
	m_level_editor->get_project()->paste( *m_cfg, %ids );

	m_level_editor->get_command_engine()->run( gcnew command_select(m_level_editor, %ids, enum_selection_method_set));
	return true;
}

void command_paste_objects::rollback()
{
	m_level_editor->get_project()->remove( *m_cfg );
}

}// namespace editor
}// namespace xray
