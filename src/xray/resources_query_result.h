////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_QUERY_RESULT_H_INCLUDED
#define XRAY_RESOURCES_QUERY_RESULT_H_INCLUDED

#include <xray/resources_resource.h>
#include <xray/resources_classes.h>
#include <xray/resources_cook_base.h>
#include <xray/type_variant.h>

namespace xray {
namespace resources {

typedef	variant<32>		user_data_variant;

class device_manager;
class queries_result;

class XRAY_CORE_API query_result_for_user
{
public:
	enum error_type_enum		{ 	error_type_unset,
								  	error_type_file_not_found,
								  	error_type_cannot_open_file,
								  	error_type_cannot_read_file,
								  	error_type_cannot_write_file,
								  	error_type_canceled_by_finalization,
								  	error_type_out_of_memory, 
								  	error_type_cannot_decompress_file,
								  	error_type_hash_not_equal_to_db_hash,
									error_type_query_translation_failed,
									error_type_cook_not_registered,
									error_type_name_registry_error };
public:
									query_result_for_user	();
	virtual						   ~query_result_for_user	() {}

	bool							is_success				() const { return (m_error_type == error_type_unset) && (m_create_resource_result != cook_base::result_error); }
	error_type_enum					get_error_type			() const { return m_error_type; }
	
	managed_resource_ptr			get_managed_resource	() const { return m_managed_resource; }
	unmanaged_resource_ptr			get_unmanaged_resource	() const { return m_unmanaged_resource; }

	class_id						get_class_id			() const { return m_class_id; }
	pcstr							get_requested_path		() const { return m_request_path; }
	const_buffer					creation_data_from_user	() const { return m_creation_data_from_user; }
	fs::path_string					get_full_path			() const;
	mutable_buffer					generated_data_to_save	() const { return m_generated_data_to_save; }
	mutable_buffer					grab_generated_data_to_save	() { mutable_buffer out_data = m_generated_data_to_save; m_generated_data_to_save = mutable_buffer(NULL, 0); return out_data; }

protected:
	const_buffer					m_creation_data_from_user;
	mutable_buffer					m_generated_data_to_save;
	
	managed_resource_ptr			m_managed_resource;
	unmanaged_resource_ptr			m_unmanaged_resource;

	pstr							m_request_path;
	error_type_enum					m_error_type;
	cook_base::result_enum			m_create_resource_result;
	class_id						m_class_id;
	fat_it_wrapper					m_fat_it;

}; // query_result_for_user

class XRAY_CORE_API query_result_for_cook : public query_result_for_user
{
public:
									query_result_for_cook	(queries_result * parent);
	virtual						   ~query_result_for_cook	() {}

	void							set_managed_resource	(managed_resource * ptr) { R_ASSERT(!m_managed_resource); m_managed_resource = ptr; }
	void							set_zero_unmanaged_resource	();
	memory::base_allocator *		get_user_allocator		() const { return m_user_allocator; }
	virtual signalling_bool			select_disk_path_from_request_path	(buffer_string * out_disk_path) const = 0;
	void							save_generated_resource	(const_buffer const & in_data);
	void							set_request_path		(pcstr path);
	void							finish_query			(cook_base::result_enum result);
	u32								get_raw_file_size		() const;
	query_result_for_cook *			get_parent_query		() const;
	void							set_error_type			(error_type_enum error_type, bool check_was_unset = true);
	void							set_unmanaged_resource	(unmanaged_resource_ptr ptr, memory_type const & memory_type, u32 resource_size, u32 pool_id = 0);
	void							set_unmanaged_resource	(unmanaged_resource_ptr ptr, memory_usage const & memory_usage);
	void							set_managed_resource	(managed_resource_ptr ptr) { R_ASSERT(!m_managed_resource); m_managed_resource = ptr; }
	user_data_variant *				user_data				() const { return m_user_data; }
	void							set_out_of_memory		(memory_usage const & memory_request) { m_out_of_memory = memory_request; }

protected:
	managed_resource_ptr			get_managed_resource	() const { return m_managed_resource; }
	memory_usage					m_out_of_memory;
	memory::base_allocator *		m_user_allocator;
	queries_result *				m_parent;
	user_data_variant *				m_user_data;
	string_path						m_request_path_default_storage;
	u32								m_request_path_max_size;

}; // query_result_for_cook

class XRAY_CORE_API query_result :	public	resource_base, 
									public	query_result_for_cook,
									public	xray::detail::noncopyable
{

public:
									query_result			(u16 flags							=	0,
															 queries_result * parent			=	NULL,
															 memory::base_allocator * allocator	=	NULL,
															 u32 user_thread_id					=	0);

									~query_result			();

	fat_it_wrapper					get_fat_it				() const { return m_fat_it; }
	u32								get_priority			() const { return m_priority; }
	bool							is_load_type			() const { return has_flag(flag_load); }
	bool							is_save_type			() const { return has_flag(flag_save); }
	bool							is_replication_type		() const { return has_flag(flag_replication); }
	bool							is_compressed			() const;
	virtual signalling_bool			select_disk_path_from_request_path	(buffer_string * out_disk_path) const;

private:
	enum						{	flag_unset									=	1 << 0,
									flag_load									=	1 << 1,
									flag_replication							=	1 << 2,
									flag_save									=	1 << 3,
									flag_locked_fat_iterator					=	1 << 4,
									flag_in_generating_because_no_file_queue	=	1 << 5,
									flag_is_referer								=	1 << 6,	
									flag_processed_request_path					=	1 << 7,
									flag_finished								=	1 << 8,	
									flag_should_free_request_path				=	1 << 9,
									flag_uses_physical_path						=	1 << 10,
									flag_inplace_in_user_data_for_creation_or_inline_data	=	1 << 11,
									flag_refers_to_raw_file						=	1 << 12,
									flag_translated_query						=	1 << 13,
									flag_zero_unmanaged_resource_was_set		=	1 << 14,
									flag_in_pending_list						=	1 << 15,
									flag_reused_resource						=	1 << 16,
									flag_in_create_resource						=	1 << 17,
									flag_finished_create_resource				=	1 << 18,
									flag_need_saving_of_generated_resource		=	1 << 19,
								};

	enum push_to_ready_queries_bool	{	push_to_ready_queries_false, push_to_ready_queries_true	};

	cook_base::result_enum			get_create_resource_result			() const { return m_create_resource_result; }

	bool							is_finished				() const { return has_flag(flag_finished); }
	bool							append_data_if_needed	(const_buffer data, file_size_type data_pos_in_file);

	bool							allocate_compressed_resource_if_needed		();
	bool							allocate_raw_unmanaged_resource_if_needed	();
	bool							allocate_raw_managed_resource_if_needed		();
	bool							need_create_resource_inplace_in_creation_or_inline_data				();
	void							bind_unmanaged_resource_buffer_to_creation_or_inline_data	();
	bool							allocate_final_unmanaged_resource_if_needed	();
	bool							allocate_final_managed_resource_if_needed	();
	void							free_unmanaged_buffer		 				();
	mutable_buffer					raw_unmanaged_buffer						() const { return m_raw_unmanaged_buffer; }
	bool							raw_managed_resource						() const { return m_raw_managed_resource; }

	bool							try_synchronous_cook_from_inline_data	();

	void							clear_reference			();
	void							add_referrer			(query_result *	referrer, bool log_that_referer_query_added = true);
	void							set_flag				(u32 flag);
	bool							has_flag				(u32 flag) const { return !!(m_flags & flag); }
	void							unset_flag				(u32 flag);

	bool							check_fat_for_resource_reusage				();

	bool							is_flag					() const;

	void							replicate				();

	// called by device_manager, or when file was taken from fat_it cache
	void							on_file_operation_end	(query_result *	referred = NULL);
	void							on_save_operation_end	();
	void							on_load_operation_end	(query_result *	referred = NULL);
	bool							check_file_crc			();
	// called for referer queries by refered query when its ending
	void							on_refered_query_ended  (query_result * refered_query);

	void							send_to_allocate_final_resource	();
	void							on_allocated_final_resource		();
	// sends resource to cook later in right thread
	void							send_to_create_resource		();
	// is called from cooker right thread
	void							do_create_resource						();
	void							do_managed_create_resource				(managed_cook * cook);
	void							do_inplace_managed_create_resource		(inplace_managed_cook * cook);
	void							do_unmanaged_create_resource			(unmanaged_cook * cook);
	void							do_inplace_unmanaged_create_resource	(inplace_unmanaged_cook * cook);

	void							do_create_resource_end_part		();
	// is called by resource_manager::dispatch_created_resources or directly if no cooking needed
	void							on_create_resource_end			(push_to_ready_queries_bool push_to_ready_queries);
	void							associate_created_resource_with_fat_or_name_registry	();
	
	// called when query_result is fully ready
	void							on_query_end			(push_to_ready_queries_bool push_to_ready_queries = push_to_ready_queries_true);

	void							init_load				(fat_it_wrapper			fat_it, 
															 int					priority);

	void							init_replication		(fat_it_wrapper			fat_it, 
															 managed_resource_ptr	resource,
															 int					priority);

	void							init_save				(pcstr					physical_path, 
															 mutable_buffer const & data, 
															 query_result *			data_to_save_generator, 
															 int					priority);

	void							set_raw_is_unmovable		(bool is_unmovable);
	device_manager *				find_capable_device_manager ();
	u32								get_user_thread_id			() const { return m_user_thread_id; }

	void							process_request_path		();

	void							translate_request_path		();

	bool							translate_query_if_needed	();
	bool							is_translate_query			() const;

	void							on_decompressing_end		();
	bool							file_loaded					() const;

	enum consider_with_name_registry_result_enum {	consider_with_name_registry_result_error,
													consider_with_name_registry_result_no_action,
													consider_with_name_registry_result_added_as_referer,
													consider_with_name_registry_result_added_self_as_host_for_referers,
													consider_with_name_registry_result_got_associated_resource,	};

	enum only_try_to_get_associated_resource_bool { only_try_to_get_associated_resource_false, only_try_to_get_associated_resource_true };
	consider_with_name_registry_result_enum   consider_with_name_registry		(only_try_to_get_associated_resource_bool  only_try_to_get_associated_resource);
	consider_with_name_registry_result_enum   consider_with_name_registry_impl	(pcstr name, only_try_to_get_associated_resource_bool only_try_to_get_associated_resource);

	struct request_path_iterator
	{
		pcstr						iterator;
		fs::path_string				item;
		request_path_iterator () : iterator(0) {}
	};

	bool							next_item_in_request_path			(request_path_iterator & out_it)	const;
	bool							has_same_item_in_request_path		(query_result const & other)		const;
	bool							need_saving_of_generated_resource	() const;
	void							late_set_fat_it						(fat_it_wrapper it);
	void							lock_fat_it							();
	void							unlock_fat_it						();

	const_buffer					pin_raw_buffer						();
	void							unpin_raw_buffer					(const_buffer const & pinned_raw_buffer);
	const_buffer					pin_raw_file						();
	void							unpin_raw_file						(const_buffer const & pinned_raw_file);
	const_buffer					pin_compressed_file					();
	void							unpin_compressed_file				(const_buffer const & pinned_compressed_file);
	const_buffer					pin_compressed_or_raw_file	();
	void							unpin_compressed_or_raw_file	(const_buffer const & pinned_file);
	u32								raw_buffer_size						();

	bool							has_uncompressed_inline_data				();
	bool							has_inline_data								();
	bool							need_create_resource_if_no_file						();

	bool							copy_inline_data_to_resource_if_needed		();
	bool							copy_creation_data_to_resource_if_needed	();
	void							copy_data_to_resource						(const_buffer data);
	u32								loaded_bytes								() const { return m_loaded_bytes; }
	void							set_loaded_bytes							(u32 byte_count);
	void							add_loaded_bytes							(u32 byte_count);
	u32								compressed_or_raw_file_size					() const;
	fat_it_wrapper					get_fat_it_zero_if_physical_path_it			() const;
	void							set_creation_source_for_resource 			(unmanaged_resource_ptr resource);
	void							set_creation_source_for_resource 			(managed_resource_ptr resource);
	resource_base::creation_source_enum   creation_source_for_resource			();

	void							set_deleter_object							(unmanaged_resource * resource);
	u32								allocate_thread_id							() const;
	virtual void					recalculate_memory_usage_impl				() { ; } // not used
	void							set_create_resource_result					(cook_base::result_enum result);
	void							finish_normal_query							(cook_base::result_enum create_resource_result);
	void							finish_translated_query						(cook_base::result_enum create_resource_result);

private:
	enum out_of_memory_type_enum	{	out_of_memory_type_unset, 
										out_of_memory_on_translate_query, 
									};
	query_result *					m_next_out_of_memory;
	out_of_memory_type_enum			m_out_of_memory_type;
	
	union {
	query_result *					m_next_in_device_manager;
	query_result *					m_next_to_create_resource; // used in processed query-s lists 
	query_result *					m_next_generated_to_save;
	query_result *					m_next_decompress;
	query_result *					m_next_to_allocate_raw_resource;
	query_result *					m_next_to_allocate_resource;
	query_result *					m_next_to_translate_query;
	query_result *					m_next_to_init;
	}; // union
	query_result *					m_next_in_generate_if_no_file_queue;
	query_result *					m_prev_in_generate_if_no_file_queue;
	query_result *					m_next_in_pending_query_list;
	query_result *					m_prev_in_pending_query_list;
	query_result *					m_next_referer;
	query_result *					m_data_to_save_generator;

	managed_resource_ptr			m_compressed_resource;	// may be NULL
	managed_resource_ptr			m_raw_managed_resource; 
	mutable_buffer					m_raw_unmanaged_buffer; 
	mutable_buffer					m_unmanaged_buffer; 
	name_registry_entry				m_name_registry_entry;

	u32								m_offset_to_file;
	u32								m_loaded_bytes;

	u32								m_priority;
	threading::atomic32_type		m_flags;
	u32								m_final_resource_size;
	u32								m_user_thread_id;
	memory::base_allocator *		m_temp_disk_fat_it_allocator;
	bool							m_is_unmovable;

	friend class					device_manager;
	friend class					queries_result;
	friend class					resources_manager;
	friend class					thread_local_data;
	friend class					cook;
	friend class					unmanaged_resource;
	friend class					query_result_for_cook;
	friend class					game_resources_manager;
};

} // namespace resources
} // namespace xray

#endif // #ifndef XRAY_RESOURCES_QUERY_RESULT_H_INCLUDED