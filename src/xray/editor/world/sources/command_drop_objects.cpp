////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "command_drop_objects.h"
#include "object_base.h"
#include "level_editor.h"
#include "picker.h"
#include "project.h"
#include "transform_control_base.h"
#include "collision_object_dynamic.h"

namespace xray {
namespace editor {

command_drop_objects::command_drop_objects ( level_editor^ le, bool drop_from_top ):
m_level_editor			(le),
m_first_pass			(true),
m_drop_from_top			(drop_from_top)
{
	object_list^ selection		= m_level_editor->get_project()->selection_list();

	// Sorting objects by height to drop them starting from the lower objects
	object_list objects_sorted	= selection;
	objects_sorted.Sort			(gcnew object_height_predicate);

	for each(object_base^ o in objects_sorted)
	{
		id_matrix	itm;
		itm.id		= o->id();
		itm.matrix	= NEW (math::float4x4)(o->get_transform());
		m_obj_matrices.Add(itm);
	}
}

command_drop_objects::~command_drop_objects( )
{
	for each(id_matrix^ itm in m_obj_matrices)
		DELETE ( itm->matrix	);

	m_obj_matrices.Clear( );
}

bool command_drop_objects::commit	()
{
	object_list objects;


	// Dropping direction: straight down.
	float3 const direction	( 0, -1, 0 );

	math::float4x4 tmp_mat;

	// If we are in first execution then calculating collision to drom the objects
	// else just replace old/new matrices of the objects.
	if( m_first_pass )
	{
		// Get corresponding objects to their ids
		for each(id_matrix^ idm in m_obj_matrices)
		{
			object_base^ object = object_base::object_by_id(idm->id);
			ASSERT				( object != nullptr );
			objects.Add			(object);
		}

		for each (object_base^ o in objects)
		{
			float4x4 transform	= o->get_transform();
			float	distance;
			
			collision::ray_objects_type collision_results( g_allocator );
			float3	origin;

			// If dropping objects from very top than just assigning ray pick origin to a constant 
			// height else keep the current height of the object by just lifting it up a bit to
			// be sure that the object will not drop from the plane on which it is placed before.
			if( m_drop_from_top )
				origin = float3( 1, 0, 1 )*transform.c.xyz() + float3( 0, 500.f, 0 );
			else
				origin = transform.c.xyz() + float3( 0, 0.1f, 0 );

			// Picking down to find the place for the object
			m_level_editor->get_picker( ).ray_query(collision_type_dynamic, origin, direction, collision_results );
			
			// Pass picking result by a object filter to exclude objects from picking results 
			// that are not placed yet.
			int idx_of = objects.IndexOf(o);
			if( get_valid_collision	( collision_results, objects.GetRange(idx_of, objects.Count-idx_of), distance ) )
			{
				transform.c.xyz()		= origin + direction*distance;
				o->set_transform		( transform );
			}
			else
			{
				transform.c.xyz()	= float3( 1, 0, 1 )*origin;
				o->set_transform	( transform );
			}
		}
	}else // first_pass
	{
		// Just replace old/new matrices of the objects.
		for each(id_matrix^ idm in m_obj_matrices)
		{
			object_base^ object = object_base::object_by_id(idm->id);

			tmp_mat				= object->get_transform();
			object->set_transform( *idm->matrix );
			*idm->matrix		= tmp_mat;
		}
	}

	m_first_pass		= false;
	return				true;
}

void command_drop_objects::rollback		()
{
	// Just switch old/new matrices.
	commit	( );
}

} // namespace editor
} // namespace xray

