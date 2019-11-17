////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCES_FS_IMPL_H_INCLUDED
#define RESOURCES_FS_IMPL_H_INCLUDED

#include <xray/resources_fs.h>
#include <xray/fs_path.h>
#include <xray/intrusive_list.h>

namespace xray {
namespace resources {

//----------------------------------------------------------
// mount task
//----------------------------------------------------------
	
class fs_task;
struct fs_task_base
{
	fs_task_base				() : next(NULL) {}
	fs_task *					next;
}; // struct fs_task_base

class fs_task : public fs_task_base
{
public:
	enum	type_enum		{	type_undefined, 
								type_fs_iterator_task,
								type_mount_operations_start,
								type_update_file_size_in_fat, 
								type_update_disk_node_name,
								type_erase_file,
								type_mount_disk, 
								type_mount_db, 
								type_mount_disk_node, 
								type_unmount_disk, 
								type_unmount_db,
								type_unmount_disk_node,
								type_mount_composite, // list of mount/unmount operations
								type_mount_operations_end,
								// take care to insert all mount operations between guards
							};

	type_enum					type;

	fs::path_string 			logical_path;
	fs::path_string & 			fs_iterator_path;
	fs::path_string 			disk_dir_path;
	fs::path_string & 			physical_path;
	fs::path_string & 			fat_file_path;
	fs::path_string & 			old_physical_path;
	fs::path_string 			db_file_path;
	fs::path_string &			new_physical_path;
	bool  						need_replication;
	bool						reverse_byte_order;
	query_mount_callback		mount_callback;
	query_fs_iterator_callback	fs_iterator_callback;

	intrusive_list<fs_task, fs_task_base, & fs_task_base::next>	children;
	fs_iterator					iterator;
	memory::base_allocator *	allocator;
	u32							thread_id;
	bool						result;

	fs_task						(type_enum type);

	void						add_child			(fs_task * const child) { children.push_back(child); }
	bool						execute				();
	void						call_user_callback	();

	bool						is_mount_task		() const
	{
		 return (type > type_mount_operations_start) && (type < type_mount_operations_end);
	}

private:
	void operator =				(fs_task const &) const;
};


} // namespace resources
} // namespace xray

#endif // #ifndef RESOURCES_FS_IMPL_H_INCLUDED