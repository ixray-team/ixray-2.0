////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "command_snap_base.h"
#include "object_base.h"
#include "project.h"
#include "transform_control_base.h"
#include "collision_object_dynamic.h"

namespace xray {
namespace editor {

bool command_snap_base::get_valid_collision	( collision::ray_objects_type& collision_results, object_list^ objects, float& distance )
{
	collision::ray_objects_type::iterator		it = collision_results.begin();
	collision::ray_objects_type::const_iterator	en = collision_results.end();
	
	for( ; it != en ; ++it )
	{
		collision_object const* obj_coll = static_cast_checked<collision_object const*> (it->object);
		ASSERT					( obj_coll, "The obj must not be NULL.");

		if( obj_coll->get_type() & collision_type_dynamic )
		{
			collision_object_dynamic const* dyn_coll = static_cast_checked<collision_object_dynamic const*>(obj_coll);
			ASSERT( dyn_coll, "The obj must not be NULL.");

			bool not_found = true;
			for each(object_base^ o in objects)
			{
				if( dyn_coll->get_owner() == o )
				{	
					not_found = false;
					break;
				}
			}

			if( not_found )
			{
				distance = (*it).distance;
				return true;
			}
		}
	}

	return false;
}

} // namespace editor
} // namespace xray

