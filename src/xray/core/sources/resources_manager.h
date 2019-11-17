////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED

#include "resources_helper.h"
#include "resources_fs_impl.h"
#include "resources_impl.h"

#include <xray/resources_resource.h>
#include <xray/resources_callbacks.h>
#include <xray/resources.h>
#include <xray/intrusive_list.h>
#include <xray/intrusive_double_linked_list.h>
#include <xray/limited_intrusive_list.h>
#include <xray/resources_cook_classes.h>
#include <xray/threading_event.h>
#include <xray/compressor_ppmd.h>

namespace xray {
namespace core {
	namespace configs {
		class lua_config_cook;
		class binary_config_cook;
		class binary_config_cook_impl;
	} // namespace configs
} // namespace core

namespace resources {

class		device_manager;
class		fs_task;
class		thread_local_data;

class resources_manager
{
public:
								resources_manager					();
								~resources_manager					();

	void						resources_tick_impl					();
	void						cooker_tick_impl					();
	void						dispatch_callbacks					(bool finaling_thread);

	enum query_type_enum		{ query_type_normal, query_type_helper_for_mount };

	long						query_resources_impl				(request const  			requests[], 
																	 creation_request const  	requests_create[],
										 							 u32 const					requests_count,
										 							 query_callback const &		callback,
										 							 memory::base_allocator *	allocator,
																	 user_data_variant const * const	user_data[],
										 							 int						priority,
																	 query_result_for_cook *	parent,
																	 query_type_enum			= query_type_normal);

	void						query_resources_by_mask				(pcstr						request_mask, 
																	 class_id					class_id, 
																	 query_callback const &		callback, 
																	 memory::base_allocator *	allocator,
											   					     query_flag_enum			flags, 
																	 int						priority,
																	 query_result_for_cook *	parent);

	void						push_new_query						(query_result * query, query_type_enum query_type);
	void						start_query_transaction				();
	void						end_query_transaction				();

	threading::event &			get_resources_wakeup_event			();

	threading::mutex &			get_execute_task_mutex				();
	threading::mutex &			get_mount_transaction_mutex			();

	static bool					need_replication					(pcstr file_path);
	device_manager *			find_capable_device_manager 		(pcstr file_path);

	void						init_new_queries					(query_result * new_query_list);
	void						push_delayed_delete_resource		(managed_resource * res);
	void						push_delayed_delete_unmanaged_resource	(unmanaged_resource* res);
	void						delete_delayed_resources			();
	void						delete_unmanaged_resource			(unmanaged_resource * dying_resource);
	void						delete_delayed_unmanaged_resource	(unmanaged_resource * dying_resource);
	void						delete_delayed_unmanaged_resources	();
	void						dispatch_delete_delayed_unmanaged_resources	();

	void						init_query_result					(query_result &		result);

	bool						replicate_resource  				(fat_iterator			fat_it,
																	 managed_resource_ptr	resource,
																	 query_callback			callback,
																	 int					priority);

	void						on_query_finished					(queries_result *);

	void						add_fs_task							(fs_task *			mount_task);

	void						resources_thread_proc				();
	u32							resources_thread_id					() const { return m_resources_thread_id; }

	typedef	associative_vector<u32, thread_local_data *, vector>	thread_local_data_container;
	thread_local_data *			get_thread_local_data				(u32			thread_id, 
																	 bool			create_if_not_exist);

	void						finalize_thread_usage				(bool 			call_from_main_thread);
	void						wait_and_dispatch_callbacks			(bool 			call_from_main_thread, 
																	 bool 			finalizing_thread);

	void						register_cook						(cook_base *	cook);
	cook_base *					unregister_cook						(class_id		resoure_class);

	void						wakeup_resources_thread				();
	void						wakeup_cooker_thread				();

	void						inc_num_current_fs_ops				()			{ threading::interlocked_increment(m_num_current_fs_ops); }
	void						dec_num_current_fs_ops				()			{ threading::interlocked_decrement(m_num_current_fs_ops); }
	long						has_no_pending_mount_ops			() const	{ return !m_count_of_pending_mount_operations && !m_count_of_pending_helper_query_for_mount; }
	u32							num_device_managers					() const	{ return m_device_managers.size(); }

	void						start_cooks_registration			();
	void						finish_cooks_registration			();

	u32							cooker_thread_id					() const	{ return m_cooker_thread_id; }

	core::configs::lua_config_cook &	get_lua_config_cook			();
	core::configs::binary_config_cook &	get_binary_config_cook		();
	core::configs::binary_config_cook_impl& get_binary_config_cook_impl	();

	void						mount_mounts_path					(pcstr mount_path);

	void						set_query_finished_callback			(query_finished_callback callback) { m_query_finished_callback  = callback; }
	void						set_out_of_memory_callback			(out_of_memory_callback	callback) { m_out_of_memory_functionality.set_out_of_memory_callback_impl(callback); }
	out_of_memory_callback		get_out_of_memory_callback			() const { return m_out_of_memory_functionality.get_out_of_memory_callback_impl(); }
	void						push_out_of_memory_query			(query_result * query) { m_out_of_memory_functionality.push_out_of_memory_query_impl(query); }

	pvoid						allocate_unmanaged_memory			(u32 size, pcstr type_name);
	managed_resource *			allocate_managed_resource			(u32 size);
	void						free_managed_resource				(managed_resource * );

private:
	void						register_cooks						();
	void						init_query_with_no_fat_it			(query_result &		query);
	void						add_to_generate_if_no_file_queue	(query_result &		query);
	void						remove_from_generate_if_no_file_queue	(query_result &		query);

	void						push_to_device_manager				(query_result &		query);
	void						dispatch_query_callback				(queries_result *	ready_query,
																	 bool const			finalizing_thread);
	void						dispatch_query_callbacks			(queries_result *	ready_query_list, 
																	 bool const			finalizing_thread);

	void						dispatch_fs_tasks_callbacks			(fs_task *			ready_fs_tasks, 
																	 bool const			finalizing_thread);

	bool						thread_can_exit						();
	void						execute_fs_task						(fs_task * task);
	void						execute_fs_tasks 					(fs_task * task);

	void						register_unmanaged_resource			(unmanaged_resource * res);
	void						unregister_unmanaged_resource		(unmanaged_resource * res);

	//----------------------------------------------------------
	// create resource
	//----------------------------------------------------------
	void						cooker_thread_proc					();
	void						add_resource_to_create				(query_result *	query);
	void						dispatch_created_resources			();

	template <class query_list> // for query_list::get_next_of_object
	void						create_resources					(query_list const &, query_result * it_query, bool finalizing_thread);

	void						on_created_resource					(query_result * query);

	cook_base *					find_cook							(class_id		resoure_class);

	//void						finish_postponed_query				(query_result * query);

	//----------------------------------------------------------
	// translating query
	//----------------------------------------------------------
	void						push_to_translate_query				(query_result * query);
	template <class queries_list>
	void						translate_queries					(queries_list const &, query_result * to_translate_query);
	void						translate_query						(query_result * query);

	//----------------------------------------------------------
	// decompressing resources
	//----------------------------------------------------------
	void						add_resource_to_decompress			(query_result * query);
	void						decompress_resource					(query_result * query);
	void						decompress_resources				();
	void						dispatch_decompressed_resources		();

	//----------------------------------------------------------
	// counting queries being processed by resource manager per thread
	//----------------------------------------------------------
	threading::atomic32_type	m_pending_queries_count;

	void						on_added_queries					(u32 num_queries);
	void						on_dispatched_queries				(u32 num_queries);

	//----------------------------------------------------------
	// save_generated_resource
	//----------------------------------------------------------
	void						push_generated_resource_to_save		(query_result * in_query);
	void						save_generated_resource				(query_result * in_query);
	void						save_generated_resources			();

	//----------------------------------------------------------
	// delayed registering of cookers
	//----------------------------------------------------------
	threading::atomic32_type	m_num_cook_registrators;
	bool						cooks_are_registering				() const { return m_num_cook_registrators != 0; }

	//----------------------------------------------------------
	// misc
	//----------------------------------------------------------
	void						mark_unmovables_before_defragmentation	();
	long						next_unique_id_for_queries				() { return threading::interlocked_increment(m_last_unique_id_for_queries); }

	threading::atomic32_type	m_count_of_pending_query_with_fat_it;
	threading::atomic32_type	m_count_of_pending_helper_query_for_mount;

	void						change_count_of_pending_query_with_fat_it		(long change);
	void						change_count_of_pending_mount_operations		(long change);
	void						change_count_of_pending_helper_query_for_mount	(long change);	

	threading::mutex &			mutex_mount_task_add				() { return m_mutex_mount_task_add; }

	//----------------------------------------------------------
	// mounts
	//----------------------------------------------------------
	void						do_mount_mounts_path				();
	threading::event												m_mounts_initialized_event;
	fs::path_string													m_mounts_path;
	threading::atomic32_type										m_do_mount_mounts_path;

	//----------------------------------------------------------
	// allocation
	//----------------------------------------------------------
	void						send_to_allocate_final_resource_impl	(query_result * query) { m_allocate_resource_functionality.send_to_allocate_final_resource_impl(query); }
	void						prepare_raw_resource					(query_result * query) { m_allocate_resource_functionality.prepare_raw_resource_impl(query); }
	void						on_allocated_raw_resource				(query_result * query) { m_queries_with_allocated_raw_resources.push_back(query); }
	void						dispatch_allocated_raw_resources		();
	
	//----------------------------------------------------------
	// pending_query_list
	//----------------------------------------------------------
	typedef intrusive_double_linked_list<query_result, 
								 query_result, 
								 & query_result::m_prev_in_pending_query_list, 
								 & query_result::m_next_in_pending_query_list,
								 threading::multi_threading_mutex_policy>	pending_query_list;
								 
	pending_query_list												m_pending_query_list;

	void						register_pending_query					(query_result * query) { m_pending_query_list.push_back(query); }
	void						unregister_pending_query				(query_result * query) { m_pending_query_list.erase(query); }

#ifndef MASTER_GOLD
	//----------------------------------------------------------	
	// global_delay_delete_list	
	//----------------------------------------------------------	
	typedef intrusive_double_linked_list<unmanaged_resource, 								 
										 unmanaged_resource, 								 
										 & unmanaged_resource::m_prev_in_global_delay_delete_list, 								 
										 & unmanaged_resource::m_next_in_global_delay_delete_list,								 
										 threading::multi_threading_mutex_policy>	global_delay_delete_list_unmanaged_list;								 	
	
	global_delay_delete_list_unmanaged_list							m_global_delay_delete_unmanaged_list;	

	void						register_delay_delete_unmanaged		(unmanaged_resource * ptr) { m_global_delay_delete_unmanaged_list.push_back(ptr); }
	void						unregister_delay_delete_unmanaged	(unmanaged_resource * ptr) { m_global_delay_delete_unmanaged_list.erase(ptr); }
#endif // #ifndef MASTER_GOLD

	//----------------------------------------------------------	
	// name_registry	
	//----------------------------------------------------------	
	typedef hash_multiset< name_registry_entry, xray::detail::fixed_size_policy<1024*32>, detail::name_registry_hash, detail::name_registry_equal > name_registry_type;

	name_registry_type												m_name_registry;
	threading::mutex												m_name_registry_mutex;

	threading::mutex &			name_registry_mutex ()				{ return m_name_registry_mutex; }
	name_registry_type &		name_registry		()				{ return m_name_registry; }

	typedef intrusive_list<name_registry_entry, name_registry_entry, & name_registry_entry::next_to_delete> name_registry_delete_queue;
	name_registry_delete_queue										m_name_registry_delete_queue;

	void						push_name_registry_to_delete		(name_registry_entry * entry);
	void						delete_name_registry_entries		();

	//----------------------------------------------------------	
	// global callbacks
	//----------------------------------------------------------	
	void						push_to_call_query_finished_callback	(resource_base * resource);
	void						dispatch_query_finished_callbacks		();
	typedef	intrusive_list<resource_base, resource_base, & resource_base::m_next_for_query_finished_callback>	query_finished_callback_list;
	query_finished_callback_list	m_query_finished_callback_list;

private:
	threading::mutex												m_ready_queries_mutex;
	threading::mutex												m_mutex_mount_task_add;
	typedef intrusive_double_linked_list<query_result, 
								 query_result, 
								 & query_result::m_prev_in_generate_if_no_file_queue, 
								 & query_result::m_next_in_generate_if_no_file_queue,
								 threading::multi_threading_mutex_policy>	generate_if_no_file_queue;
								 
	generate_if_no_file_queue										m_generate_if_no_file_queue;

	intrusive_double_linked_list<unmanaged_resource, 
								 unmanaged_resource, 
								 & unmanaged_resource::m_prev_in_global_list, 
								 & unmanaged_resource::m_next_in_global_list>	m_unmanaged_resources;

	typedef	vector<device_manager*>									device_managers;
	device_managers													m_device_managers;

	bool															m_debug_single_thread;
	timing::timer													m_flush_timer;

	threading::mutex												m_mount_transaction_mutex;

	typedef intrusive_list<query_result, query_result, &query_result::m_next_to_init> new_queries_list;
	new_queries_list												m_new_queries_with_locked_fat_it;
	new_queries_list												m_new_queries_with_unlocked_fat_it;
	new_queries_list												m_new_queries_waiting_for_cook_register;
	new_queries_list												m_new_queries_for_mount;

	intrusive_list<managed_resource, managed_resource, &managed_resource::m_next_delay_delete>	m_delayed_delete_resources;
	intrusive_list<unmanaged_resource, unmanaged_resource, &unmanaged_resource::m_next_delay_delete>	
																	m_delayed_delete_unmanaged_resources;
	threading::atomic32_type										m_num_delay_delete_unmanaged_resources;

	intrusive_list<query_result, query_result, &query_result::m_next_to_allocate_raw_resource>	
																	m_queries_with_allocated_raw_resources;
	
	threading::mutex												m_thread_local_data_mutex;
	thread_local_data_container										m_thread_local_data;
	u32																m_tls_key_thread_local_data;

	bool															m_resources_thread_exit;
	bool															m_resources_thread_exited;
	u32																m_resources_thread_id;				
	threading::event												m_resources_wakeup_event;

	bool															m_cooker_thread_exit;
	bool															m_cooker_thread_exited;
	u32																m_cooker_thread_id;
	threading::event												m_cooker_wakeup_event;

	typedef	fixed_vector<cook_base *, 512>							cooks_registry;
	cooks_registry													m_cooks_registry;

	typedef intrusive_list<query_result, query_result, &query_result::m_next_to_create_resource>	create_resource_list;
	create_resource_list											m_resources_to_create;
	create_resource_list											m_created_resources;

	typedef intrusive_list<query_result, query_result, &query_result::m_next_generated_to_save>		generated_resources_to_save_list;
	generated_resources_to_save_list								m_generated_resources_to_save_list;

	typedef intrusive_list<query_result, query_result, &query_result::m_next_decompress>	decompress_list;
	decompress_list													m_resources_to_decompress;
	decompress_list													m_decompressed_resources;

	threading::mutex												m_execute_task_mutex;
	threading::mutex												m_transaction_mutex;

	intrusive_list<fs_task, fs_task_base, & fs_task_base::next>		m_fs_tasks;

	threading::atomic32_type										m_num_unmanaged_resources;

	threading::atomic32_type										m_count_of_pending_mount_operations;
	threading::atomic32_type										m_num_current_fs_ops;
	uninitialized_reference<ppmd_compressor>						m_compressor;

	threading::atomic32_type										m_dispatching_delay_delete_unmanaged_resources_flag;

	threading::mutex												m_wait_and_dispatch_callbacks_mutex;

	threading::atomic32_type										m_last_unique_id_for_queries;

	query_finished_callback											m_query_finished_callback;

	//----------------------------------------------------
	// allocate_resource_functionality
	//----------------------------------------------------
	class allocate_resource_functionality
	{
	public:
		void						send_to_allocate_final_resource_impl	(query_result * query);
		void						prepare_raw_resource_impl				(query_result * query);
		void						tick									(bool finalizing_thread);

	private:
		void						prepare_raw_resource_for_managed_or_unmanaged_cook	(query_result * query);
		void						prepare_raw_resource_for_inplace_managed_cook		(query_result * query);
		void						prepare_raw_resource_for_inplace_unmanaged_cook		(query_result * query);
		void						continue_prepare_raw_resource_for_inplace_unmanaged_cook	(query_result * query);
		template <class query_list>
		void						allocate_final_resources		(query_list & queries, bool finalizing_thread);
		void						allocate_raw_resources			(thread_local_data * tls, bool finalizing_thread);

	private:
		typedef intrusive_list<query_result, query_result, &query_result::m_next_to_allocate_resource>	allocate_list;
		allocate_list				m_queries_to_allocate_managed_resource;
	};

	allocate_resource_functionality									m_allocate_resource_functionality;

	//----------------------------------------------------
	// allocate_resource_functionality
	//----------------------------------------------------
	class out_of_memory_functionality
	{
	public:
		void						push_out_of_memory_query_impl	(query_result * query);
		void						tick							();

		void						set_out_of_memory_callback_impl	(out_of_memory_callback	callback) { m_out_of_memory_callback = callback; }
		out_of_memory_callback		get_out_of_memory_callback_impl	() const { return m_out_of_memory_callback; }

	private:
		typedef intrusive_list< query_result, query_result, & query_result::m_next_out_of_memory > out_of_memory_queue;
		out_of_memory_queue			m_out_of_memory_queue;
		out_of_memory_callback		m_out_of_memory_callback;
	};

	out_of_memory_functionality		m_out_of_memory_functionality;

	//----------------------------------------------------
	// friends
	//----------------------------------------------------

	friend class													query_result;
	friend class													queries_result;
	friend class													unmanaged_resource;
	friend class													managed_resource;
	friend class													managed_resource_allocator;
	friend class													mount_by_config_helper;
	friend class													fs_task;
	friend class													cook_base;
	friend class													base_of_intrusive_base;

}; // class resources_manager

extern uninitialized_reference<resources_manager>					g_resources_manager;

} // namespace resources
} // namespace xray

#endif// RESOURCES_H_INCLUDED