#include "pch.h"

#include <xray/resources_cook_classes.h>
#include "resources_managed_allocator.h"
#include "resources_manager.h"

namespace xray {
namespace resources {

static u32	 correct_thread_id (u32 const id) 
{
	return	(id == cook_base::use_current_thread_id) ? threading::current_thread_id() : id;
}

cook_base::cook_base  (class_id					resource_class, 
					   reuse_enum				reuse_type,
					   enum_flags<flags_enum>	flags,
					   u32						allocate_thread_it,
					   u32 						creation_thread_id)
					   : 
					   m_class_id				(resource_class),
					   m_reuse_type				(reuse_type),
					   m_flags					(flags),
					   m_allocate_thread_id		(correct_thread_id(allocate_thread_it)),
					   m_creation_thread_id		(correct_thread_id(creation_thread_id)),
					   m_next					(NULL)
{
}

class_id   cook_base::get_class_id () const 
{ 
	return										m_class_id; 
}

bool   cook_base::cooks_managed_resource ()  const 
{ 
	return										has_flag(flag_creates_managed_resource); 
}

bool   cook_base::cooks_managed_resource (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									true;

	return										cook->cooks_managed_resource();
}

bool   cook_base::cooks_inplace ()  const 
{ 
	return										has_flag(flag_does_inplace_cook); 
}

bool   cook_base::cooks_inplace (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									true;

	return										cook->cooks_inplace();
}

cook_base::reuse_enum   cook_base::reuse_type	() const
{
	return										m_reuse_type;
}

cook_base::reuse_enum   cook_base::reuse_type	(class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									reuse_true;

	return										cook->reuse_type();
}

unmanaged_cook *   cook_base::cast_unmanaged_cook ()
{
	if ( has_flag(flag_creates_managed_resource) || has_flag(flag_does_inplace_cook) || has_flag(flag_does_translate_query) )
		return									NULL;
	
	return										static_cast_checked<unmanaged_cook *>(this);
}

managed_cook *   cook_base::cast_managed_cook ()
{
	if ( !has_flag(flag_creates_managed_resource) || has_flag(flag_does_inplace_cook) || has_flag(flag_does_translate_query) )
		return									NULL;

	return										static_cast_checked<managed_cook *>(this);
}

inplace_unmanaged_cook *   cook_base::cast_inplace_unmanaged_cook ()
{
	if ( has_flag(flag_creates_managed_resource) || !has_flag(flag_does_inplace_cook) || has_flag(flag_does_translate_query) )
		return									NULL;

	return										static_cast_checked<inplace_unmanaged_cook *>(this);
}

inplace_managed_cook *   cook_base::cast_inplace_managed_cook ()
{
	if ( !has_flag(flag_creates_managed_resource) || !has_flag(flag_does_inplace_cook) || has_flag(flag_does_translate_query) )
		return									NULL;

	return										static_cast_checked<inplace_managed_cook *>(this);
}

translate_query_cook *   cook_base::cast_translate_query_cook ()
{
	if ( !has_flag(flag_does_translate_query) )
		return									NULL;

	return										static_cast_checked<translate_query_cook *>(this);
}

cook_base *	  cook_base::find_cook (class_id resource_class)
{
	return										g_resources_manager->find_cook(resource_class);
}

managed_cook *   cook_base::find_managed_cook (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									NULL;
	return										cook->cast_managed_cook();
}

unmanaged_cook *   cook_base::find_unmanaged_cook (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									NULL;
	return										cook->cast_unmanaged_cook();
}

inplace_unmanaged_cook *   cook_base::find_inplace_unmanaged_cook (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									NULL;
	return										cook->cast_inplace_unmanaged_cook();
}

inplace_managed_cook *   cook_base::find_inplace_managed_cook (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									NULL;
	return										cook->cast_inplace_managed_cook();
}

translate_query_cook *   cook_base::find_translate_query_cook (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									NULL;
	return										cook->cast_translate_query_cook();
}

bool   cook_base::allow_sync_load_from_inline (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									false;

	return										cook->allow_sync_load_from_inline();
}

void   cook_base::call_destroy_resource	(unmanaged_resource * resource)
{
	if ( inplace_unmanaged_cook * const cook = cast_inplace_unmanaged_cook() )
		cook->destroy_resource					(resource);
	else if ( unmanaged_cook * const cook = cast_unmanaged_cook() )
		cook->destroy_resource					(resource);
	else
		NOT_IMPLEMENTED							();	
}

void   cook_base::call_destroy_resource	(managed_resource * resource)
{
	if ( inplace_managed_cook * const cook = cast_inplace_managed_cook() )
		cook->destroy_resource					(resource);
	else if ( managed_cook * const cook = cast_managed_cook() )
		cook->destroy_resource					(resource);
	else
		NOT_IMPLEMENTED							();
}

u32   cook_base::cook_users_count () const
{ 
	return										m_cook_users_count.cook_users_count();
}

u32   cook_base::creation_thread_id ()
{ 
	if ( m_creation_thread_id == use_cooker_thread_id )
	{
		R_ASSERT								(g_resources_manager.initialized());
		m_creation_thread_id				=	g_resources_manager->cooker_thread_id();
	}
	else if ( m_creation_thread_id == use_resource_manager_thread_id )
	{
		R_ASSERT								(g_resources_manager.initialized());
		m_creation_thread_id				=	g_resources_manager->resources_thread_id();
	}

	return										m_creation_thread_id; 
}

u32	  cook_base::allocate_thread_id ()
{
	if ( m_allocate_thread_id == use_cooker_thread_id )
	{
		R_ASSERT								(g_resources_manager.initialized());
		m_allocate_thread_id				=	g_resources_manager->cooker_thread_id();
	}
	else if ( m_allocate_thread_id == use_resource_manager_thread_id )
	{
		R_ASSERT								(g_resources_manager.initialized());
		m_allocate_thread_id				=	g_resources_manager->resources_thread_id();
	}

	return										m_allocate_thread_id;
}

u32   cook_base::allocate_thread_id (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									false;

	return										cook->allocate_thread_id();
}

bool   cook_base::does_create_resource () const
{
	if ( has_flag(flag_does_translate_query) )
		return									false;

	return										true;
}

bool   cook_base::does_create_resource (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									false;

	return										cook->does_create_resource();
}

void   cook_base::cook_users_count_functionality::change_cook_users_count (int change)
{
	if ( change == +1 )
	{
		threading::interlocked_increment		(m_count);
	}
	else if ( change == -1 )
	{
		R_ASSERT								(m_count);
		threading::interlocked_decrement		(m_count);
	}
}

bool   cook_base::does_create_resource_if_no_file ()
{
	if ( has_flag(flag_does_translate_query) )
		return									false;

	if ( has_flag(flag_does_inplace_cook) )
	{
		if ( has_flag(flag_creates_managed_resource) )
			return								cast_inplace_managed_cook()->get_create_resource_if_no_file_delegate();
		else
			return								cast_inplace_unmanaged_cook()->get_create_resource_if_no_file_delegate();
	}

	if ( has_flag(flag_creates_managed_resource) )
		return									cast_managed_cook()->get_create_resource_if_no_file_delegate();
	else
		return									cast_unmanaged_cook()->get_create_resource_if_no_file_delegate();
}

bool   cook_base::does_create_resource_if_no_file (class_id resource_class)
{
	cook_base * const cook					=	g_resources_manager->find_cook(resource_class);
	if ( !cook )
		return									false;

	return										cook->does_create_resource_if_no_file();
}

void   cook_base::to_string (buffer_string * out_string)
{
	R_ASSERT									(out_string);
	if ( has_flag(flag_does_translate_query) )
	{
		* out_string						+=	"translate_query";
		return;
	}

	if ( has_flag(flag_does_inplace_cook) )
		* out_string						+=	"inplace_";

	if ( has_flag(flag_creates_managed_resource) )
		* out_string						+=	"managed";
	else
		* out_string						+=	"unmanaged";

	if ( does_create_resource_if_no_file() )
		* out_string						+=	"+create_resource_if_no_file";
}

void   cook_base::register_object_to_delete (unmanaged_resource * resource, u32 deallocation_thread_id)
{
	resource->set_deleter_object				(this, deallocation_thread_id);
}

pcstr   convert_cook_reuse_value_to_string (cook_base::reuse_enum value)
{
	if ( value == cook_base::reuse_false )
		return									"reuse_false";
	else if ( value == cook_base::reuse_true )
		return									"reuse_true";
	else if ( value == cook_base::reuse_raw )
		return									"reuse_raw";
	
	R_ASSERT									(identity(false));
	return										"";
}
} // namespace resources
} // namespace xray

