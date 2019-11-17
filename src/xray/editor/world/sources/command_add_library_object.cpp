////////////////////////////////////////////////////////////////////////////
//	Created		: 30.03.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "command_add_library_object.h"
#include "level_editor.h"
#include "tool_base.h"
#include "object_base.h"
#include "project_items.h"
#include "project.h"
#include "command_select.h"


namespace xray {
namespace editor {

command_add_library_object::command_add_library_object	(	level_editor^ le, 
															tool_base^ tool, 
															System::String^ library_name, 
															math::float4x4 transform,
															bool select ):
m_level_editor	( le ),
m_tool			( tool ),
m_object_id		( 0 ),
m_select		( select )
{
	m_transform			= NEW (float4x4)(transform);
	ASSERT				( library_name!=nullptr );
	ASSERT				( library_name->Length!=0 );

	m_library_name		= library_name;
}

command_add_library_object::~command_add_library_object			() 
{
	DELETE (m_transform);
}

bool command_add_library_object::commit		()
{
	// if we are in rollback stage than we need to reuse the last id.
	// else the project will generate a new one.
	project_item^	item	= m_level_editor->get_project()->add_new_item( m_tool, m_library_name, *m_transform, m_object_id, nullptr );
	object_base^	object	= item->m_object_base;

	if( object == nullptr )
		return false;

	m_object_id = object->id();

	if( m_object_id == 0)
		return false;
	
	if( m_select )
		m_level_editor->get_command_engine()->run( gcnew command_select( m_level_editor, object, enum_selection_method_set ));

	return true;
}

void command_add_library_object::rollback	()
{
	m_level_editor->get_project()->remove_item		( m_object_id, true );

	// m_object_id is saved to be reused in rollback stage
}

}// namespace editor
}// namespace xray
