////////////////////////////////////////////////////////////////////////////
//	Created		: 28.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/resources_intrusive_base.h>
#include "resources_helper.h"
#include "resources_manager.h"

namespace xray {
namespace resources {

//----------------------------------------------------------
// base_of_intrusive_base
//----------------------------------------------------------

bool   base_of_intrusive_base::is_associated_with_fat (managed_resource * const object) const
{
	fat_iterator	fat_it				=	wrapper_to_fat_it(object->m_fat_it);
	return									fat_it.is_associated_with(object);
}

bool   base_of_intrusive_base::is_associated_with_fat (unmanaged_resource * const object) const
{
	fat_iterator	fat_it				=	wrapper_to_fat_it(object->m_fat_it);
	return									fat_it.is_associated_with(object);
}

void   base_of_intrusive_base::pin_reference_count_for_query_finished_callback ()	
{ 
	threading::interlocked_increment(m_reference_count); 
	m_flags				|=	flag_pinned_by_query_finished_callback;
}

void   base_of_intrusive_base::unpin_reference_count_for_query_finished_callback ()
{ 
	R_ASSERT				(m_reference_count > 0); 
	threading::interlocked_decrement(m_reference_count); 
	m_flags				&=	~flag_pinned_by_query_finished_callback;
}

void   base_of_intrusive_base::pin_reference_count_for_game_resource_manager ()
{ 
	threading::interlocked_increment(m_reference_count); 
	m_flags				|=	flag_pinned_by_game_resource_manager;
}

void   base_of_intrusive_base::unpin_reference_count_for_game_resource_manager ()
{ 
	R_ASSERT				(m_reference_count > 0); 
	threading::interlocked_decrement(m_reference_count); 
	m_flags				&=	~flag_pinned_by_game_resource_manager;
}

template <class Resource>
signalling_bool   base_of_intrusive_base::try_unregister_from_fat_or_from_name_registry (Resource * const object, u32 zero_reference_count) const
{
	fat_iterator	fat_it				=	wrapper_to_fat_it(object->m_fat_it);
	if ( !object->reference_count() )
		LOGI_TRACE							("resources:resource", "zero links to %s", object->log_string().c_str());
	else
		return								false;

	pcstr const rare_message			=	"LOL. resource was reclaimed just when we were about to delete it!";
	XRAY_UNREFERENCED_PARAMETER				( rare_message );

	if ( !fat_it.is_end() )
	{
		if ( is_associated_with_fat(object) )
		{
			if ( !fat_it.try_clean_associated_if_zero_reference_resource() )
			{
				LOG_TRACE					(rare_message);
				return						false;
			}
		}
	}
	else if ( name_registry_entry * const name_registry_entry =	object->get_name_registry_entry() )
	{
		R_ASSERT							(cook_base::reuse_type(object->get_class_id()) == cook_base::reuse_true);
		
		threading::mutex_raii	raii		(g_resources_manager->name_registry_mutex());

		// zero_reference_count is usually 0
		// value of 1 is needed for game resource manager, 
		// when he tries to de-associate resource, yet not deleting it
		if ( reference_count() > (long)zero_reference_count )
		{
			LOG_TRACE						(rare_message);
			return							false;
		}
			
		g_resources_manager->push_name_registry_to_delete	(name_registry_entry);
	}

	return									true;
}

//----------------------------------------------------------
// managed_intrusive_base
//----------------------------------------------------------

void   managed_intrusive_base::destroy (managed_resource * const resource) const
{
	if ( !try_unregister_from_fat_or_from_name_registry	(resource) )
		return; // resource was reclaimed just when we were about to delete it

#pragma message(XRAY_TODO("calling destroy_resource right in user thread, at this time it seems as sufficient"))
	if ( cook_base * const cook = cook_base::find_cook(resource->get_class_id()) )
		cook->call_destroy_resource						(resource);

	if ( threading::current_thread_id() == g_resources_manager->resources_thread_id() )
		g_resources_manager->free_managed_resource		(resource);
	else
		g_resources_manager->push_delayed_delete_resource	(resource);
}

//----------------------------------------------------------
// unmanaged_intrusive_base
//----------------------------------------------------------

void   unmanaged_intrusive_base::destroy (unmanaged_resource * const resource) const
{
	R_ASSERT	(resource->get_deleter_object());

	if ( !try_unregister_from_fat_or_from_name_registry	(resource) )
		return; // resource was reclaimed just when we were about to delete it

	u32 const deallocate_thread_id		=	resource->deallocate_thread_id();
	if ( deallocate_thread_id == threading::current_thread_id() )
		g_resources_manager->delete_unmanaged_resource				(resource); // sync deletion
	else
		g_resources_manager->push_delayed_delete_unmanaged_resource	(resource); // async deletion
}

} // namespace resources
} // namespace xray
