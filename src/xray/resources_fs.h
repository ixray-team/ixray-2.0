////////////////////////////////////////////////////////////////////////////
//	Created 	: 03.02.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_FS_ITERATOR_H_INCLUDED
#define XRAY_RESOURCES_FS_ITERATOR_H_INCLUDED

#include <xray/fs_path_string.h>
#include <xray/fs_utils.h>
#include <xray/resources_resource.h>
#include <xray/fs_platform_configuration.h>

namespace xray {
namespace resources {

class fs_task;

//----------------------------------------------------------
// mounting & unmounting
//----------------------------------------------------------

typedef					boost::function< void (bool result) >				query_mount_callback;

void XRAY_CORE_API		query_mount				(pcstr 						mount_id,
												 query_mount_callback		callback,
												 memory::base_allocator*	allocator);

void XRAY_CORE_API		query_unmount			(pcstr 						mount_id,
												 query_mount_callback		callback,
												 memory::base_allocator*	allocator);

void XRAY_CORE_API		query_mount_disk		(pcstr 						logical_path, 
												 pcstr 						disk_dir_path, 
												 query_mount_callback		callback,
												 memory::base_allocator*	allocator,
												 fs_task *					parent_composite_task = NULL);

void XRAY_CORE_API		query_mount_db			(pcstr 						logical_path, 
												 pcstr 						fat_file_path, 
												 pcstr 						db_file_path,
												 bool  						need_replication,
												 query_mount_callback		callback,
												 memory::base_allocator*	allocator,
												 bool						reverse_byte_order = false,
												 fs_task *					parent_composite_task = NULL);

void XRAY_CORE_API		query_unmount_disk		(pcstr 						logical_path, 
												 pcstr 						disk_dir_path, 
												 query_mount_callback		callback,
												 memory::base_allocator*	allocator,
												 fs_task *					parent_composite_task = NULL);

void XRAY_CORE_API		query_unmount_db		(pcstr 						logical_path, 
												 pcstr 						fat_file_path, 
												 query_mount_callback		callback,
												 memory::base_allocator*	allocator,
												 fs_task *					parent_composite_task = NULL);

void XRAY_CORE_API		query_erase_file		(pcstr const				physical_path, 
												 query_mount_callback const callback, 
												 memory::base_allocator * const allocator);

//----------------------------------------------------------
// fs iterator
//----------------------------------------------------------

class					fs_iterator;
typedef					boost::function< void (fs_iterator begin) >		query_fs_iterator_callback;

// hurry up to return from callback, because while in it, 
// other filesystem's operations are waiting
void XRAY_CORE_API	   	query_fs_iterator	(pcstr						path, 
											 query_fs_iterator_callback,
											 memory::base_allocator *	allocator);

void XRAY_CORE_API	   	query_fs_iterator_and_wait	(pcstr						path, 
							 						 query_fs_iterator_callback, 
							 						 memory::base_allocator*	allocator);

void XRAY_CORE_API		log_fs_iterator		(fs_iterator it);

class XRAY_CORE_API fs_iterator
{
public:
	fs_iterator			find_child			(pcstr relative_path);	pcstr				get_name			() const;
	void				get_full_path		(fs::path_string &) const;
	void				get_disk_path		(fs::path_string &) const;
	
	u32					get_num_children	() const; // O(n)
	u32					get_num_nodes		() const; // O(n)
	bool				is_folder			() const;

	bool				is_disk				() const;
	bool				is_db				() const;
	bool				is_replicated		() const;
	bool				is_compressed		() const;

	u32					get_file_size		() const;
	file_size_type		get_file_offs		() const;
	u32					get_compressed_file_size	() const;
	u32					get_raw_file_size	() const;

	fs_iterator			children_begin		() const;
	fs_iterator			children_end		() const;

	bool				is_end				() const { return m_fat_it == NULL; }

	fs_iterator			operator ++			();
	fs_iterator			operator ++			(int);
	bool				operator ==			(const fs_iterator & it) const;
	bool				operator !=			(const fs_iterator & it) const;
	bool				operator <			(const fs_iterator it)	const;

private:
	fat_it_wrapper		m_fat_it;

	friend	class		fs_task;

	friend void XRAY_CORE_API query_fs_iterator	(pcstr, query_fs_iterator_callback, memory::base_allocator*);
};


// temporary and for Andy ONLY! 
fs::path_info::type_enum   XRAY_CORE_API get_path_info_by_logical_path (fs::path_info * out_path_info, pcstr logical_path, memory::base_allocator * allocator);

} // namespace resources
} // namespace xray

#endif // #ifndef XRAY_RESOURCES_FAT_ITERATOR_H_INCLUDED