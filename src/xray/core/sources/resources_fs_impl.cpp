////////////////////////////////////////////////////////////////////////////
//	Created 	: 03.02.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_fs_impl.h"
#include "resources_manager.h"
#include <xray/resources_fs.h>

namespace xray {
namespace resources {

using namespace fs;

fs_task::fs_task (type_enum const type) :	fs_iterator_path(logical_path),
											fat_file_path(disk_dir_path),
											physical_path(disk_dir_path),
											old_physical_path(disk_dir_path),
											new_physical_path(db_file_path),
											type(type), 
											need_replication(false),
											reverse_byte_order(false),
											mount_callback(NULL),
											fs_iterator_callback(NULL),
											allocator(NULL),
											thread_id(threading::current_thread_id()),
											result(false) 
{
	if ( is_mount_task() )
		g_resources_manager->change_count_of_pending_mount_operations(+1);
}


void   query_update_disk_node_name (pcstr const 					old_physical_path, 
									pcstr const						new_physical_path, 
									query_mount_callback const 		callback, 
									memory::base_allocator * const 	allocator)
{
	fs_task *	new_task			=	XRAY_NEW_IMPL(* allocator, fs_task)(fs_task::type_update_disk_node_name);

	new_task->old_physical_path		=	old_physical_path;
	new_task->new_physical_path		=	new_physical_path;
	new_task->allocator				=	allocator;
	new_task->mount_callback		=	callback;

	g_resources_manager->add_fs_task		(new_task);
}

void   query_mount_disk_node (pcstr	const physical_path, query_mount_callback const callback, memory::base_allocator * const allocator)
{
	fs_task *	new_task			=	XRAY_NEW_IMPL(* allocator, fs_task)(fs_task::type_mount_disk_node);

	new_task->physical_path			=	physical_path;
	new_task->allocator				=	allocator;
	new_task->mount_callback		=	callback;

	g_resources_manager->add_fs_task		(new_task);
}

void   query_unmount_disk_node (pcstr const physical_path, query_mount_callback const callback, memory::base_allocator * const allocator)
{
	fs_task *	new_task			=	XRAY_NEW_IMPL(* allocator, fs_task)(fs_task::type_unmount_disk_node);

	new_task->physical_path			=	physical_path;
	new_task->allocator				=	allocator;
	new_task->mount_callback		=	callback;

	g_resources_manager->add_fs_task		(new_task);
}

void   query_update_file_size_in_fat (pcstr const physical_path, query_mount_callback const callback, memory::base_allocator * const allocator)
{
	fs_task *	new_task			=	XRAY_NEW_IMPL(* allocator, fs_task)(fs_task::type_update_file_size_in_fat);

	new_task->physical_path			=	physical_path;
	new_task->allocator				=	allocator;
	new_task->mount_callback		=	callback;

	g_resources_manager->add_fs_task		(new_task);
}

void   query_erase_file (pcstr const physical_path, query_mount_callback const callback, memory::base_allocator * const allocator)
{
	fs_task *	new_task			=	XRAY_NEW_IMPL(* allocator, fs_task)(fs_task::type_erase_file);

	new_task->physical_path			=	physical_path;
	new_task->allocator				=	allocator;
	new_task->mount_callback		=	callback;

	g_resources_manager->add_fs_task		(new_task);
}

bool   fs_task::execute ()
{
	bool	result					=	true;

	if ( type == type_fs_iterator_task )
	{
		fat_iterator it				=	g_fat->find(fs_iterator_path.c_str());
		iterator.m_fat_it			=	fat_it_to_wrapper(it);
	} 
	else if ( type == type_mount_disk )
	{
		g_fat->mount_disk				(logical_path.c_str(), disk_dir_path.c_str());
	}
	else if ( type == type_mount_db )
	{
		result						=	g_fat->mount_db(logical_path.c_str(), 
														 fat_file_path.c_str(), 
														 db_file_path.c_str(), 
														 need_replication);
	}
	else if ( type == type_unmount_disk )
	{
		g_fat->unmount_disk				(logical_path.c_str(), disk_dir_path.c_str());
	}
	else if ( type == type_unmount_db )
	{
		g_fat->unmount_db				(logical_path.c_str(), fat_file_path.c_str());
	}
	else if ( type == type_erase_file )
	{
		LOGI_INFO						("file_system", "erase file(%s)", physical_path.c_str());
		g_fat->erase_disk_node			(physical_path.c_str());
		unlink							(physical_path.c_str());
	}
	else if ( type == type_mount_disk_node )
	{
		g_fat->mount_disk_node_by_physical_path	(physical_path.c_str());
	}
	else if ( type == type_unmount_disk_node )
	{
		g_fat->unmount_disk_node		(physical_path.c_str());
	}
	else if ( type == type_update_file_size_in_fat )
	{
		g_fat->update_file_size_in_fat	(physical_path.c_str());
	}
	else if ( type == type_update_disk_node_name )
	{
		g_fat->rename_disk_node			(old_physical_path.c_str(), new_physical_path.c_str());
	}
	else if ( type == type_mount_composite )
	{
		// thats ok, just do nothing
	}
	else
		UNREACHABLE_CODE();

	return								result;
}

void   fs_task::call_user_callback ()
{
	R_ASSERT							(type != type_undefined);
	if ( type == type_fs_iterator_task )
	{
		g_resources_manager->get_execute_task_mutex().lock	();
		fs_iterator_callback			(iterator);
		g_resources_manager->get_execute_task_mutex().unlock	();
	}
	else
		mount_callback					(result);
}

} // namespace resources
} // namespace xray