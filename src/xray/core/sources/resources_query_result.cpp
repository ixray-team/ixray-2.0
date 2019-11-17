////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <xray/resources_query_result.h>
#include <xray/resources_queries_result.h>
#include <xray/resources_cook_classes.h>
#include "resources_manager.h"
#include "resources_managed_allocator.h"
#include "resources_device_manager.h"
#include "fs_helper.h"

namespace xray {
namespace resources {

using namespace fs;

//----------------------------------------------------------
// query_result_for_user
//----------------------------------------------------------
query_result_for_user::query_result_for_user () : 
						m_request_path			(NULL), 
						m_error_type			(error_type_unset), 
						m_create_resource_result(cook_base::result_undefined), 
						m_class_id				(raw_data_class),
						m_unmanaged_resource	(NULL),
						m_fat_it				(NULL)			
{
}

//----------------------------------------------------------
// query_result_for_cook
//----------------------------------------------------------

query_result_for_cook::query_result_for_cook (queries_result * parent) :
						m_request_path_max_size	(array_size(m_request_path_default_storage)),
						m_user_allocator		(NULL),
						m_parent				(parent)
{
	m_request_path							=	& m_request_path_default_storage[0];
	m_request_path_default_storage[0]		=	NULL;
}

query_result_for_cook *   query_result_for_cook::get_parent_query () const 
{ 
	return										m_parent->get_parent_query(); 
}

//----------------------------------------------------------
// query_result
//----------------------------------------------------------

query_result::query_result (u16						const flags,
							queries_result*			const parent,
							memory::base_allocator* const allocator,
							u32						const user_thread_id)
	:	query_result_for_cook				(parent),
		resource_base						(resource_base::is_query_result_flag),
		m_flags								(flags),
		m_priority							(0),
		m_is_unmovable						(false),
		m_final_resource_size				(0),
		m_data_to_save_generator			(NULL),
		m_next_in_device_manager			(NULL),
		m_next_in_generate_if_no_file_queue	(NULL),
		m_prev_in_generate_if_no_file_queue	(NULL),
		m_next_in_pending_query_list		(NULL),
		m_prev_in_pending_query_list		(NULL),
		m_next_out_of_memory				(NULL),
		m_out_of_memory_type				(out_of_memory_type_unset),
		m_user_thread_id					(user_thread_id),
		m_temp_disk_fat_it_allocator		(NULL),
		m_offset_to_file					(0),
		m_loaded_bytes						(0)
{
	m_user_allocator					=	allocator;
	m_next_referer						=	this; // connect intrusive links

	g_resources_manager->register_pending_query	(this);
	set_flag								(flag_in_pending_list);
}

query_result::~query_result	()
{
	R_ASSERT							(!m_is_unmovable);
	R_ASSERT							(!has_flag(flag_in_pending_list));
	clear_reference						();

	if ( m_generated_data_to_save )
	{
		pvoid generated_data_to_save	=	m_generated_data_to_save.c_ptr();
		MT_FREE							(generated_data_to_save);
	}

	if ( has_flag(flag_uses_physical_path) )
	{
		fat_iterator	it			=	wrapper_to_fat_it(m_fat_it);
		g_fat->destroy_temp_disk_it		(m_temp_disk_fat_it_allocator, it);
		m_fat_it					=	NULL;
		unset_flag						(flag_uses_physical_path);
	}

	if ( has_flag(flag_should_free_request_path) )
	{
		ASSERT							(m_user_allocator);
		XRAY_FREE_IMPL					(*m_user_allocator, m_request_path);
		unset_flag						(flag_should_free_request_path);
	}
}

void   query_result::add_referrer (query_result* const referer, bool log_that_referer_query_added)
{
	referer->free_unmanaged_buffer		();

	ASSERT								( !has_flag(flag_is_referer) );
	ASSERT								( !referer->has_flag(flag_is_referer) );
	if ( log_that_referer_query_added )
		LOGI_INFO						("resources:manager", "referer query: '%s'", m_request_path);

	referer->set_flag					(flag_is_referer);
	
	referer->m_next_referer			=	m_next_referer;
	m_next_referer					=	referer;
}

bool   query_result::append_data_if_needed (const_buffer data, file_size_type data_pos_in_file)
{
	file_system::iterator fat_it	=	wrapper_to_fat_it(m_fat_it);
	R_ASSERT							(!fat_it.is_end());

	file_size_type const file_start	=	fat_it.get_file_offs();
	file_size_type const file_end	=	file_start + compressed_or_raw_file_size();
	file_size_type const data_start	=	data_pos_in_file;
	file_size_type const data_end	=	data_pos_in_file + data.size();

	if ( file_end <= data_start || data_end <= file_start )
		return false;

	file_size_type dest_start		=	file_start;
	file_size_type src_start		=	data_start;
	if ( file_start < data_start )
		dest_start					=	data_start;
	else
		src_start					=	file_start;		

	file_size_type dest_end			=	file_end;
	file_size_type src_end			=	data_end;
	if ( file_end < data_end )
		src_end						=	file_end;
	else
		dest_end					=	data_end;

	mutable_buffer file_data		=	cast_away_const(pin_compressed_or_raw_file());
	u32 const dest_offs				=	u32(dest_start - file_start);
	u32 const dest_size				=	u32(dest_end - dest_start);
	u32 const src_offs				=	u32(src_start - data_start);
	u32 const src_size				=	u32(src_end - src_start);

	R_ASSERT							(dest_offs == m_loaded_bytes);
	R_ASSERT							(dest_size == src_size);

	mutable_buffer const dest_data	=	file_data.slice(dest_offs, dest_size);
	const_buffer const src_data		=	data.slice(src_offs, src_size);

	memory::copy						(dest_data, src_data);
	m_loaded_bytes					+=	dest_size;

	unpin_compressed_or_raw_file		(file_data);
	return								true;
}

bool   query_result::check_fat_for_resource_reusage ()
{
	if ( cook_base::reuse_type(m_class_id) != cook_base::reuse_true )
		return							false;

	file_system::iterator fat_it	=	wrapper_to_fat_it( m_fat_it );

	if ( fat_it.is_end() )
		return							false;

	bool out_result					=	false;

	if ( resources::unmanaged_resource_ptr const cached_unmanaged = fat_it.get_associated_unmanaged_resource_ptr() )
	{
		m_unmanaged_resource		=	cached_unmanaged;
		out_result					=	true;
	}
	else if ( resources::managed_resource_ptr const cached_resource = fat_it.get_associated_managed_resource_ptr() )
	{
		if ( !cached_resource->needs_cook() )
		{
			m_managed_resource		=	cached_resource;
			out_result				=	true;
		}
	}

	if ( out_result )
	{
		set_flag						(flag_reused_resource);
		on_query_end					(push_to_ready_queries_false);
	}

	return								out_result;
}

bool   query_result::is_compressed () const
{
	file_system::iterator fat_it	=	wrapper_to_fat_it( m_fat_it );
	ASSERT								(!fat_it.is_end());

	return								fat_it.is_compressed();
}

bool   query_result::check_file_crc ()
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	if ( !fat_it.is_db() )
		return							true;
	
	const_buffer const pinned_file	=	pin_compressed_or_raw_file();

	u32 const	raw_file_size		=	fat_it.get_raw_file_size();
	u32 const	raw_file_hash		=	fs::crc32((pcstr)pinned_file.c_ptr(), raw_file_size);

	u32			hash_in_fat			=	0;
	bool const  got_hash			=	fat_it.get_hash(& hash_in_fat);
	ASSERT_U							(got_hash);

	R_ASSERT							(raw_file_hash == hash_in_fat);

	unpin_compressed_or_raw_file		(pinned_file);

	return								raw_file_hash == hash_in_fat;
}

void   query_result::clear_reference ()
{
	if ( !has_flag(flag_is_referer) )
	{
		R_ASSERT						(m_next_referer == this);
		return;
	}

	// last links to first, which has no flag_is_referer flag
	query_result* 	query			=	m_next_referer;
	while ( !query->has_flag(flag_is_referer) )
		query						=	query->m_next_referer;

	while ( query->m_next_referer != this )
		query						=	query->m_next_referer;

	query->m_next_referer			=	m_next_referer;
	unset_flag							(flag_is_referer);
	m_next_referer					=	this;
}

void   query_result::init_load (fat_it_wrapper fat_it, int priority)
{
	set_flag							(flag_load);
	m_fat_it						=	fat_it;
	m_priority						=	priority;
}

void   query_result::set_raw_is_unmovable (bool const is_unmovable)
{
	if ( m_raw_managed_resource )
		m_raw_managed_resource->set_is_unmovable (is_unmovable);
}

void   query_result::on_refered_query_ended (query_result * refered_query)
{
	set_error_type							(refered_query->get_error_type());

	m_raw_managed_resource				=	refered_query->m_raw_managed_resource;
	m_raw_unmanaged_buffer				=	refered_query->m_raw_unmanaged_buffer;
	m_managed_resource					=	refered_query->m_managed_resource;
	m_unmanaged_resource				=	refered_query->m_unmanaged_resource;
	set_create_resource_result				(refered_query->get_create_resource_result());
	set_flag								(flag_refers_to_raw_file);
	clear_reference							();

	if ( refered_query->get_error_type() != error_type_unset )
	{
		on_query_end					();
		return;
	}
	
	if ( cook_base::reuse_type(m_class_id) == cook_base::reuse_raw )
	{
		R_ASSERT						(cook_base::does_create_resource(m_class_id));
		m_unmanaged_resource		=	NULL;
		m_managed_resource			=	NULL;
		
		if ( cook_base::cooks_inplace(m_class_id) )
		{ 
			m_raw_managed_resource	=	NULL; // this are not real raw resources
			m_raw_unmanaged_buffer	=	mutable_buffer(NULL, 0); // this are not real raw resources
		}

		if ( !m_raw_managed_resource && !m_raw_unmanaged_buffer )
		{
			finish_query				(cook_base::result_requery);
			return;
		}

		R_ASSERT						(m_raw_unmanaged_buffer || m_raw_managed_resource);
		send_to_allocate_final_resource	();
		return;
	}
	else
		set_flag						(flag_reused_resource);

	on_query_end						(push_to_ready_queries_true);
}

device_manager *   query_result::find_capable_device_manager ()
{
	fat_iterator	fat_it			=	wrapper_to_fat_it(m_fat_it);
	fs::path_string	file_path;
	if ( !fat_it.is_end() )
		fs::g_fat->get_disk_path		(fat_it, file_path);

	device_manager * const manager	=	g_resources_manager->find_capable_device_manager
										(file_path.c_str());
	R_ASSERT( manager, "No device manager can process path: %s", file_path.c_str() );
	return								manager;
}

query_result::consider_with_name_registry_result_enum   query_result::consider_with_name_registry (only_try_to_get_associated_resource_bool  only_try_to_get_associated_resource)
{
	if ( cook_base::reuse_type(m_class_id) != cook_base::reuse_true || !strings::length(m_request_path) )
		return								consider_with_name_registry_result_no_action;

	pstr	cur_path					=	m_request_path;
	while ( cur_path )
	{
		pstr const separator_pos		=	strchr(cur_path, request::path_separator);
		if ( separator_pos )
			*separator_pos				=	NULL;

		if ( *cur_path != physical_path_char )
		{
			consider_with_name_registry_result_enum const out_result	=	
				consider_with_name_registry_impl(cur_path, only_try_to_get_associated_resource);
			if ( out_result == consider_with_name_registry_result_no_action )
			{
				;
			}
			else 
			{
				if ( out_result != consider_with_name_registry_result_error && cur_path != m_request_path )
				{
					pstr					path	=	NULL;
					STR_JOINA				(path, cur_path);
					// we're sure buffer is ok
					strings::copy			(m_request_path, strings::length(path)+1, path);
				}

				return						out_result;
			}
		}

		if ( separator_pos )
		{
			*separator_pos				=	request::path_separator;
			cur_path					=	separator_pos + 1;
		}
		else
			break;
	}

	return									consider_with_name_registry_result_no_action;
}

query_result::consider_with_name_registry_result_enum   query_result::consider_with_name_registry_impl (pcstr name, only_try_to_get_associated_resource_bool only_try_to_get_associated_resource)
{
	bool const only_get_associated_resource	=	(only_try_to_get_associated_resource == only_try_to_get_associated_resource_true);

	resources_manager::name_registry_type & name_registry	=	g_resources_manager->name_registry();

	m_name_registry_entry.name			=	name;
	m_name_registry_entry.associated	=	this;
	
#pragma message(XRAY_TODO("consider using hashmap with per-hash interlocked synchronization if profiler shows too much waiting for name_registry_mutex"))

	threading::mutex_raii	raii			(g_resources_manager->name_registry_mutex());
	resources_manager::name_registry_type::iterator it =	name_registry.find(& m_name_registry_entry);
	name_registry_entry * const entry	=	* it;
	if ( !entry && !only_get_associated_resource )
	{
		R_ASSERT							(is_translate_query() || creation_data_from_user());
		name_registry.insert				(& m_name_registry_entry);
		return								consider_with_name_registry_result_added_self_as_host_for_referers;
	}

	if ( entry )
	{
		R_ASSERT							(entry->associated);
		query_result * active_query		=	NULL;
		u32 resource_class_id			=	0;
		if ( (active_query = entry->associated->cast_query_result()) != 0 )
		{
			if ( only_get_associated_resource )
				return						consider_with_name_registry_result_no_action;

			if ( active_query == this )
				return						consider_with_name_registry_result_no_action;

			R_ASSERT						(is_translate_query() || creation_data_from_user());

			CURE_ASSERT						(active_query->get_class_id() == m_class_id, 
											 return consider_with_name_registry_result_error,
											 "active_query associated with path '%s' has different class id: '%d', self class id: '%d'", 
											 get_requested_path(), 
											 active_query->get_class_id(), 
											 m_class_id);

			active_query->add_referrer		(this);
			return							consider_with_name_registry_result_added_as_referer;
		}
		else if ( unmanaged_resource * const resource = entry->associated->cast_unmanaged_resource() )
		{
			set_unmanaged_resource			(resource, resource->memory_usage());
			resource_class_id			=	resource->get_class_id();
			set_flag						(flag_reused_resource);
		}
		else if ( managed_resource * const resource = entry->associated->cast_managed_resource() )
		{
			set_managed_resource			(resource);
			resource_class_id			=	resource->get_class_id();
			set_flag						(flag_reused_resource);
		}
		else
		{
			m_error_type				=	error_type_name_registry_error;
			NOT_IMPLEMENTED					(return consider_with_name_registry_result_error);
		}

		if ( has_flag(flag_reused_resource) )
		{
			R_ASSERT						((u32)m_class_id == resource_class_id, 
											 "Omg! Associated resource '%s' is of a different class: '%d'",
											 name, resource_class_id);
		}

		return								consider_with_name_registry_result_got_associated_resource;
	}
	
	return									consider_with_name_registry_result_no_action;
}

bool   query_result::translate_query_if_needed ()
{
	R_ASSERT							(!has_flag(flag_translated_query));
	if ( !is_translate_query() )
		return							false;

	translate_query_cook * const cook	=	cook_base::find_translate_query_cook(m_class_id);
	if ( allocate_thread_id() != threading::current_thread_id() )
		return							false;

	if ( cook->reuse_type() == cook_base::reuse_true && strings::length(m_request_path) > 0 )
	{
		consider_with_name_registry_result_enum const result	=	consider_with_name_registry(only_try_to_get_associated_resource_false);

		if ( result == consider_with_name_registry_result_got_associated_resource )
		{
			on_query_end						(push_to_ready_queries_false);
			return								true;
		}
		else if ( result == consider_with_name_registry_result_error )
		{
			on_query_end						(push_to_ready_queries_false);
			return								true;
		}
		else if ( result == consider_with_name_registry_result_added_as_referer )
			return								true;
	}

	cook->translate_query						(*this);

	set_flag									(flag_translated_query);
	return										true;
}

void   query_result::on_file_operation_end (query_result * const refered)
{
	if ( is_load_type() )
	{
		on_load_operation_end			(refered);
	}
	else
	{
		R_ASSERT						(!refered);
		on_save_operation_end			();
	}
}

void   query_result::on_save_operation_end ()
{
	if ( m_raw_managed_resource && m_raw_managed_resource->is_unmovable() )
		m_raw_managed_resource->set_is_unmovable	(false);

	if ( is_save_type() )
	{
		R_ASSERT									(m_data_to_save_generator);
		m_data_to_save_generator->set_error_type	(get_error_type(), false);
		m_data_to_save_generator->late_set_fat_it	(get_fat_it());
		m_data_to_save_generator->finish_query		(m_data_to_save_generator->get_create_resource_result());
	}

	on_query_end						();
	query_result *	this_ptr		=	this;
	RES_DELETE							(this_ptr);
}

void   query_result::on_load_operation_end (query_result * const	refered)
{
	XRAY_UNREFERENCED_PARAMETER			(refered);
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	ASSERT								(has_flag(flag_is_referer) == (refered != NULL));

	if ( m_error_type != error_type_unset )
	{
		if ( cook_base::does_create_resource_if_no_file(m_class_id) )
		{
			send_to_allocate_final_resource		();
			return;
		}

		on_query_end					();
		return;
	}

	if ( !check_file_crc() )
	{
		set_error_type					(error_type_hash_not_equal_to_db_hash);
		on_query_end					();
		return;
	}

	if ( !fat_it.is_replicated() )
	{
		fs::path_string	path;
		fs::g_fat->get_disk_path			(fat_it, path);
		if ( resources_manager::need_replication(path.c_str()) )
			replicate					();
	}

	if ( fat_it.is_compressed() )
	{
		g_resources_manager->add_resource_to_decompress	(this);
		return;
	}

	on_decompressing_end				();
}

void   query_result::on_decompressing_end ()
{
	if ( m_error_type != error_type_unset )
	{
		on_query_end					();
		return;
	}

	send_to_allocate_final_resource		();
}

void   query_result::on_query_end (push_to_ready_queries_bool const push_to_ready_queries)
{	
	if ( is_success() && has_flag(flag_reused_resource) )
	{
		//LOGI_INFO								("resources:resource",	"reused resource: %s", 
		//										 m_managed_resource ? m_managed_resource->log_string ().c_str() 
		//															: m_unmanaged_resource->log_string ().c_str());
	}

	g_resources_manager->unregister_pending_query	(this);	
	unset_flag							(flag_in_pending_list);

	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	if ( !fat_it.is_end() && fat_it.get_associated_query_result() )
		fat_it.set_associated			(NULL);

	unlock_fat_it						();

	if ( has_flag(flag_in_generating_because_no_file_queue) )
		g_resources_manager->remove_from_generate_if_no_file_queue	(* this);

	set_flag							(flag_finished);

	if ( m_parent )
		m_parent->on_child_query_end	(this, m_error_type == error_type_unset && m_create_resource_result != cook_base::result_error, push_to_ready_queries);
}

void   query_result::associate_created_resource_with_fat_or_name_registry ()
{
	fat_iterator fat_it						=	wrapper_to_fat_it(m_fat_it);

	if ( m_error_type == error_type_unset && !fat_it.is_end() )
	{
		cook_base * const cook				=	g_resources_manager->find_cook(m_class_id);

		resources::resource_base * resource_base	=	NULL;
		if ( !cook )
		{
			R_ASSERT							(m_managed_resource);
			resource_base					=	m_managed_resource.c_ptr();
			fat_it.set_associated				(m_managed_resource.c_ptr());
		}
		else if ( cook->cooks_managed_resource() && cook->reuse_type() == cook_base::reuse_true && m_managed_resource )
		{
			resource_base					=	m_managed_resource.c_ptr();
			fat_it.set_associated				(m_managed_resource.c_ptr());
		}
		else if ( cook->cooks_unmanaged_resource() && cook->reuse_type() == cook_base::reuse_raw && m_raw_managed_resource )
		{
			resource_base					=	m_raw_managed_resource.c_ptr();
			m_raw_managed_resource->set_need_cook	();
			fat_it.set_associated				(m_raw_managed_resource.c_ptr());
		}
		else if ( cook->cooks_unmanaged_resource() && cook->reuse_type() == cook_base::reuse_true && m_unmanaged_resource )
		{
			resource_base					=	m_unmanaged_resource.c_ptr();
			fat_it.set_associated				(m_unmanaged_resource.c_ptr());
		}

		if ( resource_base )
			g_resources_manager->push_to_call_query_finished_callback	(resource_base);
		return;
	}
	
	if ( (is_translate_query() || creation_data_from_user()) && 
		  cook_base::reuse_type(m_class_id) == cook_base::reuse_true &&
		  strings::length(m_request_path) > 0 )
	{
		resources_manager::name_registry_type & name_registry	=	g_resources_manager->name_registry();
		R_ASSERT								(m_name_registry_entry.name	== m_request_path);

		name_registry_entry * new_entry			=	NULL;
		if ( m_error_type == error_type_unset )
		{
			u32 const name_registry_entry_size	=	sizeof(name_registry_entry) + strings::length(m_request_path) + 1;
			mutable_buffer allocation				(RES_ALLOC(char, name_registry_entry_size), name_registry_entry_size);
			new_entry						=	new (allocation.c_ptr()) name_registry_entry;
			allocation						+=	sizeof(name_registry_entry);
			new_entry->name					=	(pcstr)allocation.c_ptr();
			strings::copy						((pstr)allocation.c_ptr(), allocation.size(), m_request_path);
			
			if ( m_managed_resource )
			{
				new_entry->associated		=	m_managed_resource.c_ptr();
				m_managed_resource->set_name_registry_entry		(new_entry);
			}
			else if ( m_unmanaged_resource )
			{
				new_entry->associated		=	m_unmanaged_resource.c_ptr();
				m_unmanaged_resource->set_name_registry_entry	(new_entry);
			}
			else
				NOT_IMPLEMENTED					();
		}

		threading::mutex & name_registry_mutex	=	g_resources_manager->name_registry_mutex();

		name_registry_mutex.lock					();
		resources_manager::name_registry_type::iterator const it =	name_registry.find(& m_name_registry_entry);
		name_registry_entry const * const entry	=	* it;
		R_ASSERT_U									(entry == & m_name_registry_entry);
		name_registry.erase							(it);
		if ( m_error_type == error_type_unset )
			name_registry.insert					(new_entry);
		name_registry_mutex.unlock					();
	}
}

void   query_result_for_cook::set_error_type (error_type_enum error_type, bool check_was_unset)
{ 
	if ( check_was_unset )
		R_ASSERT						(m_error_type == error_type_unset);

	m_error_type					=	error_type; 
}

void   query_result_for_cook::set_unmanaged_resource (unmanaged_resource_ptr	ptr, 
													  memory_type const &		memory_type, 
													  u32						resource_size, 
													  u32						pool_id)
{
	m_unmanaged_resource			=	ptr;
	if ( ptr )
	{
		R_ASSERT						(resource_size, "you cannot pass 0 value as a resource size");
		ptr->set_memory_usage			(memory_usage(& memory_type, resource_size, pool_id));
	}
}

void   query_result_for_cook::set_unmanaged_resource (unmanaged_resource_ptr ptr, memory_usage const & memory_usage)
{
	set_unmanaged_resource				(ptr, * memory_usage.type, memory_usage.size, memory_usage.pool);
}

const_buffer   query_result::pin_compressed_file ()
{
	if ( m_compressed_resource )
		return							memory::buffer(m_compressed_resource->pin(), m_compressed_resource->get_size());

	R_ASSERT							(has_inline_data());
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	R_ASSERT							(fat_it.is_compressed());
	const_buffer						inline_data;
	fat_it.get_inline_data				(& inline_data);
	return								inline_data;
}

void   query_result::unpin_compressed_file (const_buffer const & pinned_compressed_data)
{
	if ( m_compressed_resource )
		m_compressed_resource->unpin	((pcbyte)pinned_compressed_data.c_ptr());
}

const_buffer   query_result::pin_raw_buffer ()
{
	if ( m_raw_managed_resource )
		return							memory::buffer(m_raw_managed_resource->pin(), m_raw_managed_resource->get_size());
	else if ( m_creation_data_from_user )
		return							m_creation_data_from_user;
	else if ( m_raw_unmanaged_buffer )
		return							m_raw_unmanaged_buffer;

	R_ASSERT							(has_uncompressed_inline_data());
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	const_buffer						inline_data;
	
	fat_it.get_inline_data				(& inline_data);
	return								inline_data;
}

void   query_result::unpin_raw_buffer (const_buffer const & pinned_raw_buffer)
{
	if ( m_raw_managed_resource )
		m_raw_managed_resource->unpin	((pcbyte)pinned_raw_buffer.c_ptr());
}

const_buffer   query_result::pin_raw_file ()
{
	return								pin_raw_buffer() + m_offset_to_file;
}

void   query_result::unpin_raw_file (const_buffer const & pinned_raw_file)
{
	unpin_raw_buffer					(memory::buffer((pcstr)pinned_raw_file.c_ptr() - m_offset_to_file, 
										 pinned_raw_file.size() + m_offset_to_file));
}

const_buffer   query_result::pin_compressed_or_raw_file ()
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	return								(!fat_it.is_end() && fat_it.is_compressed()) ? 
										pin_compressed_file() : pin_raw_file();
}

void   query_result::unpin_compressed_or_raw_file (const_buffer const & pinned_file)
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	if ( !fat_it.is_end() && fat_it.is_compressed() )
		unpin_compressed_file			(pinned_file);
	else
		unpin_raw_file					(pinned_file);
}

u32   query_result::raw_buffer_size ()
{
	const_buffer const raw_buffer	=	pin_raw_buffer();
	u32 const out_size				=	raw_buffer.size();
	unpin_raw_buffer					(raw_buffer);
	return								out_size;
}

bool   query_result::has_uncompressed_inline_data ()
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	return								!fat_it.is_end() && fat_it.is_inlined() && !fat_it.is_compressed();
}

bool   query_result::has_inline_data ()
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	return								!fat_it.is_end() && fat_it.is_inlined();
}

bool   query_result::need_create_resource_if_no_file ()
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	return								fat_it.is_end() && !creation_data_from_user() && !has_flag(flag_refers_to_raw_file);
}

u32   query_result::compressed_or_raw_file_size () const
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	R_ASSERT							(!fat_it.is_end()); 
	return								fat_it.get_raw_file_size();
}

u32	  query_result_for_cook::get_raw_file_size () const
{
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);
	R_ASSERT							(!fat_it.is_end() || creation_data_from_user());
	if ( !fat_it.is_end() )
		return							fat_it.get_file_size();

	return								creation_data_from_user().size();
}

fat_it_wrapper   query_result::get_fat_it_zero_if_physical_path_it () const
{
	if ( has_flag(flag_uses_physical_path) )
		return							NULL;

	return								m_fat_it;	
}

void   query_result::set_loaded_bytes (u32 byte_count)
{
	m_loaded_bytes					=	byte_count;
	R_ASSERT							(m_loaded_bytes <= compressed_or_raw_file_size());
}

void   query_result::add_loaded_bytes (u32 byte_count)
{
	m_loaded_bytes					+=	byte_count;
	R_ASSERT							(m_loaded_bytes <= compressed_or_raw_file_size());
}

bool   query_result::file_loaded () const
{
	return								m_loaded_bytes == compressed_or_raw_file_size();
}

void   query_result_for_cook::set_zero_unmanaged_resource ()
{
	query_result * const this_ptr		=	static_cast_checked<query_result *>(this);
	this_ptr->set_flag						(query_result::flag_zero_unmanaged_resource_was_set);
}

void   query_result::set_flag (u32 flag) 
{ 
	threading::interlocked_or				(m_flags, flag); 
}

void   query_result::unset_flag (u32 flag) 
{ 
	threading::interlocked_and				(m_flags, ~flag); 
}

void   query_result_for_cook::finish_query (cook_base::result_enum result)
{
	if ( result == cook_base::result_out_of_memory )
	{
		R_ASSERT							(m_out_of_memory.type && m_out_of_memory.size, 
			"if you should call set_out_of_memory before finish_query(result_out_of_memory)");
	}

	query_result * const this_ptr		=	static_cast_checked<query_result *>(this);

	if ( this_ptr->is_translate_query() )
		this_ptr->finish_translated_query	(result);
	else
		this_ptr->finish_normal_query		(result);
}

bool   query_result::need_saving_of_generated_resource () const 
{ 
	if ( has_flag(flag_need_saving_of_generated_resource) )
	{
		R_ASSERT							(m_generated_data_to_save); 
		return								true;
	}

	return									false;
}

} // namespace resources
} // namespace xray
