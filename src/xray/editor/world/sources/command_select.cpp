////////////////////////////////////////////////////////////////////////////
//	Created		: 01.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "command_select.h"
#include "level_editor.h"
#include "project.h"
#include "object_base.h"


namespace xray {
namespace editor {

command_select::command_select( level_editor^ le ):
m_level_editor		(le),
m_selection_method	(enum_selection_method_set),
m_objects			(gcnew id_list)
{
}

command_select::command_select( level_editor^ le, object_list^ objects_list, enum_selection_method method ):
m_level_editor		(le),
m_selection_method	(method),
m_objects			(gcnew id_list)
{
	if(dynamic_cast<object_base^>(objects_list[0]))
	{
		for each (object_base^ obj in objects_list)
			m_objects->Add(obj->id());
	}

	R_ASSERT(m_objects->Count>0);
}

command_select::command_select	( level_editor^ le, object_base^ object, enum_selection_method method ):
m_level_editor		(le),
m_selection_method	(method),
m_objects			(gcnew id_list)
{
	if( object != nullptr )
		m_objects->Add( object->id() );
}

command_select::command_select	( level_editor^ le, id_list^ objects, enum_selection_method method ):
m_level_editor		(le),
m_objects			(gcnew id_list(objects)),
m_selection_method	(method)
{
}

command_select::command_select	( level_editor^ le, u32 id, enum_selection_method method ):
m_level_editor		(le),
m_objects			(gcnew id_list),
m_selection_method	(method)
{
	m_objects->Add	( id );
}


bool command_select::commit	() 
{
	return work( m_selection_method );
}

bool command_select::work		( enum_selection_method method )
{
	//////////////////////////////////////////////////////////////////////////
	// Here need to be code identifying the object by ID
	//////////////////////////////////////////////////////////////////////////

	object_list^ old_selection		=  m_level_editor->get_project()->selection_list();

	if( m_objects->Count == old_selection->Count  && m_objects->Count == 0 )
		return false;

	id_list^ tmp_objects = gcnew id_list;

	for each(object_base^ o in old_selection )
		tmp_objects->Add	( o->id() );

	m_level_editor->get_project()->select_ids( m_objects, method );

	m_objects	= tmp_objects;

	// Followed calls will only set the list of object IDs.
	m_selection_method = enum_selection_method_set;

	return true;
}

void command_select::rollback	()
{
	(void)work( m_selection_method );
}

} // namespace editor
} // namespace xray
