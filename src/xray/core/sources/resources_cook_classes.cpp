#include "pch.h"
#include <xray/resources_cook_classes.h>

namespace xray {
namespace resources {

managed_cook::managed_cook	(class_id resource_class, reuse_enum reuse_type, u32 creation_thread_id) 
:	cook_base(resource_class, reuse_type, flag_creates_managed_resource, thread_id_unset, creation_thread_id)
{
}

inplace_managed_cook::inplace_managed_cook	(class_id resource_class, reuse_enum reuse_type, u32 creation_thread_id)
:	cook_base(resource_class, reuse_type, flag_creates_managed_resource | flag_does_inplace_cook, thread_id_unset, creation_thread_id)
{
}

unmanaged_cook::unmanaged_cook (class_id resource_class, reuse_enum reuse_type, u32 creation_thread_id, u32 allocate_thread_id)
:	cook_base(resource_class, reuse_type, 0, allocate_thread_id, creation_thread_id)
{
}

inplace_unmanaged_cook::inplace_unmanaged_cook (class_id resource_class, reuse_enum reuse_type, 
												u32 creation_thread_id, u32 allocate_thread_id)
:	cook_base(resource_class, reuse_type, flag_does_inplace_cook, allocate_thread_id, creation_thread_id)
{
}

translate_query_cook::translate_query_cook (class_id resource_class, reuse_enum reuse_type, u32 translate_query_thread)
:	cook_base(resource_class, reuse_type, flag_does_translate_query, translate_query_thread, thread_id_unset)
{
}

} // namespace resources
} // namespace xray
