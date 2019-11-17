////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "command_apply_control_transform.h"
#include "object_base.h"
#include "level_editor.h"
#include "project.h"
#include "transform_control_base.h"

namespace xray {
namespace editor {

command_apply_control_transform::command_apply_control_transform ( level_editor^ le, xray::editor::transform_control_base^ control ):
m_level_editor			(le),
m_transform_control		(control),
m_first_run				(true)
{
	ASSERT( control != nullptr );

	object_list^ selection = m_level_editor->get_project()->selection_list();

	for each(object_base^ o in selection)
	{
		id_matrix	itm;
		itm.id		= o->id();
		itm.matrix	= NEW (math::float4x4)(o->get_transform());
		m_id_matrices.Add(itm);
	}
}

bool command_apply_control_transform::commit	()
{
	math::float4x4 tmp_mat;

	for each(id_matrix^ idm in m_id_matrices)
	{
		object_base^ object = object_base::object_by_id(idm->id);

		if( m_first_run )
		{
			const math::float4x4 transform = m_transform_control->create_object_transform( *idm->matrix, enum_transform_mode_local );
			object->set_transform( transform );
		}else
		{
			tmp_mat				= object->get_transform();
			object->set_transform(*idm->matrix);
			*idm->matrix		= tmp_mat;
		}
	}

	return			true;
}

bool command_apply_control_transform::end_preview	()
{
	m_first_run		= false;
	return			true;
}

void command_apply_control_transform::rollback		()
{
	m_first_run		= false;
	commit			();
}

} // namespace editor
} // namespace xray
