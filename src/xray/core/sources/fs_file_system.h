////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef FS_FILE_SYSTEM_H_INCLUDED
#define FS_FILE_SYSTEM_H_INCLUDED

#include <xray/fs_path.h>
#include <xray/fs_platform_configuration.h>
#include <xray/resources_resource.h>
#include <xray/compressor.h>

namespace xray {

namespace resources {
	class resources_manager;
	class device_manager;
	class query_result;
	class resource_allocator;
	class managed_resource_allocator;	
	class resource_base;
} // namespace resources

namespace fs {

bool				initialize				();
void				finalize				();
void				initialize_watcher		();
void    			flush_replications		();
void XRAY_CORE_API	set_allocator_thread_id	(u32 thread_id);

typedef				boost::function< void ( resources::resource_base *	resource ) >
					on_resource_leaked_callback;
void XRAY_CORE_API	set_on_resource_leaked_callback	(on_resource_leaked_callback);

//-----------------------------------------------------------------------------------
// fat_inline_data
//-----------------------------------------------------------------------------------

struct XRAY_CORE_API fat_inline_data
{
public:
	struct item
	{
							item	() : max_size(0),  compression_rate(0), allow_compression_in_db(true) {}
		fixed_string<32>	extension;
		u32					max_size;
		float				compression_rate; // 0 - no compression, 1 - compress all
		bool				allow_compression_in_db;
		enum				{ no_limit = u32(-1) };
	};

	typedef	buffer_vector<item>	container;

public:
	fat_inline_data (container * items) : m_items(items), m_highest_compression_rate(0) {}

	void	push_back (item const & item);

	bool	find_by_extension (item * * out_item, pcstr extension);
	bool	find_by_extension (item const * * out_item, pcstr extension) const;

	bool	try_fit_for_inlining (pcstr file_name, u32 file_size, u32 compressed_size);
	u32		total_size_for_extensions_with_limited_size () const;

	float	highest_compression_rate () const { return m_highest_compression_rate; }
	
private:
	float					m_highest_compression_rate;
	buffer_vector<item> *	m_items;
};

//-----------------------------------------------------------------------------------
// file_system
//-----------------------------------------------------------------------------------

class XRAY_CORE_API file_system : public detail::noncopyable {
public:
	// Warning: taking upper bytes for flags
	enum {	is_folder				=	1 << 15, 
			is_disk					=	1 << 14, 
			is_db					=	1 << 13,
			is_replicated			=	1 << 12,	
			is_compressed			=	1 << 11, 
			is_inlined				=	1 << 10,	
			is_locked				=	1 << 00,	}; // special value flags = 1 OK!

	enum	db_target_platform_enum		{ db_target_platform_unset, db_target_platform_pc, db_target_platform_xbox360, db_target_platform_ps3, };
	static  bool   is_big_endian_format	(db_target_platform_enum const db_format) { return db_format == db_target_platform_xbox360 || db_format == db_target_platform_ps3; };

public:
 	class					iterator;
 	typedef const iterator	const_iterator;

	typedef	fastdelegate::FastDelegate< void (const u32 num_saved, u32 num_whole, pcstr name, u32 flags) >::BaseType			db_callback;
	
	enum				watch_directory_bool	{ watch_directory_false, watch_directory_true };

public:
						file_system			();
					   ~file_system			();

	void				mount_disk			(pcstr logical_path, pcstr disk_dir_path, watch_directory_bool watch_directory = watch_directory_true);
	
	bool				mount_db			(pcstr logical_path, 
											 pcstr fat_file_path, 
											 pcstr db_file_path,
											 bool  need_replication);

	void				unmount_disk		(pcstr logical_path, pcstr disk_dir_path);
	void				unmount_db			(pcstr logical_path, pcstr fat_file_path);

	bool				save_db				(pcstr 		 				fat_file_path, 
											 pcstr 		 				db_file_path,
											 bool		 				no_duplicates,
											 u32		 				fat_alignment,
											 memory::base_allocator *	alloc,
											 compressor * 				compressor,
											 float		 				compress_smallest_rate,
											 db_target_platform_enum	db_format,
											 fat_inline_data &			inline_data,
											 db_callback 				callback);

	bool				unpack				(pcstr fat_dir, pcstr dest_dir, db_callback);

	void				set_replication_dir (pcstr replication_dir);

	void				commit_replication	(iterator it);

	void				replicate_path		(pcstr path2replicate, path_string& out_path) const;
	void				get_disk_path		(const iterator& it, path_string& out_path) const;
	bool				equal_db			(iterator it1, iterator it2) const;

	bool				replicate_file		(iterator it, pcbyte data);

	bool				operator ==			(const file_system& f) const;

	iterator			find				(pcstr path);
	const_iterator		find				(pcstr path) const;
 	iterator			end					() const;
	
	iterator			create_temp_disk_it					(memory::base_allocator* alloc, pcstr disk_path);
	void				destroy_temp_disk_it				(memory::base_allocator* alloc, iterator it);
	signalling_bool		get_disk_path_to_store_file			(pcstr logical_path, buffer_string * out_disk_path);
	bool				mount_disk_node_by_logical_path		(pcstr logical_path, iterator * out_iterator = NULL);
	bool				mount_disk_node_by_physical_path	(pcstr physical_path, iterator * out_iterator = NULL);
	bool				unmount_disk_node					(pcstr physical_path);
	bool				update_file_size_in_fat				(pcstr physical_path);
	bool				rename_disk_node					(pcstr old_physical_path, pcstr new_physical_path);
	void				erase_disk_node						(pcstr physical_path);

	void				set_on_resource_leaked_callback		(on_resource_leaked_callback callback);

private:
	void				clear				();

	file_system_impl *	get_impl			() const { return m_impl; }
	file_system_impl *	m_impl;

	template <pointer_for_fat_size_enum pointer_for_fat_size>
	friend class		fat_node;
	friend class		::xray::fs::file_system_impl;
	friend void			initialize_watcher ();
};

class XRAY_CORE_API	file_system::iterator
{
public:
	pcstr				get_name			() const;
	void				get_full_path		(path_string &) const;
	path_string			get_full_path		() const;
	
	u32					get_num_children	() const;
	u32					get_num_nodes		() const;
	bool				is_folder			() const;

	bool				is_disk				() const;
	bool				is_db				() const;
	bool				is_replicated		() const;
	bool				is_compressed		() const;
	bool				is_inlined			() const;

	int					get_flags			() const;
	void				set_flags			(u32 flags);
	bool				get_hash			(u32 * out_hash) const;
	u32					get_compressed_file_size	() const;
	u32					get_raw_file_size	() const;
	u32					get_file_size		() const;
	file_size_type		get_file_offs		() const;

	iterator			children_begin		() const;
	iterator			children_end		() const;

	void				print				(pcstr offs="") const;

	void				set_end				() { m_cur = NULL; }
	bool				is_end				() const { return m_cur == NULL; }

	void				set_is_locked		(bool lock);

	iterator			operator ++			();
	iterator			operator ++			(int);
	bool				operator ==			(const iterator& it) const;
	bool				operator !=			(const iterator& it) const;
	bool				operator <			(const iterator it) const;

private:
	void								set_associated							(resources::resource_base * resource);
	bool  								is_associated							() const;
	bool  								is_associated_with						(resources::resource_base * resource) const;
	resources::managed_resource_ptr		get_associated_managed_resource_ptr		() const;
	resources::unmanaged_resource_ptr	get_associated_unmanaged_resource_ptr	() const;
	resources::query_result *			get_associated_query_result				() const;
	bool								get_inline_data							(const_buffer * out_buffer) const;
	bool								try_clean_associated_if_zero_reference_resource	() const;

	fat_node<>*			m_cur;

	friend class		file_system;
	friend class		resources::resources_manager;
	friend class		resources::managed_resource_allocator;
	friend class		resources::device_manager;
	friend class		resources::managed_resource;
	friend class		resources::managed_intrusive_base;
	friend class		resources::base_of_intrusive_base;
	friend class		resources::unmanaged_resource;
	friend class		resources::unmanaged_intrusive_base;
	friend class		resources::query_result;
	friend class		resources::resource_allocator;
};

//-----------------------------------------------------------------------------------
// file_system global object
//-----------------------------------------------------------------------------------

extern xray::uninitialized_reference<file_system>	g_fat;

} // namespace fs
} // namespace xray

#endif // FS_FILE_SYSTEM_H_INCLUDED