////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/fs_utils.h>
#include "fs_file_system.h"
#include "fs_macros.h"
#include "fs_file_system_impl.h"
#include "fs_path_iterator.h"
#include "fs_helper.h"
#include "fs_watcher.h"
//#include <direct.h>

namespace xray {
namespace fs   {

//-----------------------------------------------------------------------------------
// global file_system object
//-----------------------------------------------------------------------------------

uninitialized_reference<file_system>	g_fat;

void   set_allocator_thread_id (u32 thread_id)
{
	memory::g_fs_allocator.user_thread_id	(thread_id);
}

void   set_on_resource_leaked_callback	(on_resource_leaked_callback callback)
{
	g_fat->set_on_resource_leaked_callback	(callback);
}

//-----------------------------------------------------------------------------------
// file_system
//-----------------------------------------------------------------------------------

bool   fat_inline_data::find_by_extension (item const * * out_item, pcstr extension) const
{
	item * item	=	NULL;
	fat_inline_data * this_non_const	=	const_cast<fat_inline_data *>(this);
	if ( this_non_const->find_by_extension(& item, extension) )
	{
		* out_item						=	item;
		return								true;
	}

	return									false;
}

bool   fat_inline_data::find_by_extension (item * * out_item, pcstr extension)
{
	for ( container::iterator		it	=	m_items->begin(), 
								end_it	=	m_items->end();
									it	!=	end_it;
								  ++it	)
	{
		if ( it->extension == extension )
		{
			* out_item				=	& * it;
			return						true;			
		}
	}

	return								false;
}

u32   fat_inline_data::total_size_for_extensions_with_limited_size () const
{
	u32 total_size					=	0;
	for ( container::const_iterator	it	=	m_items->begin(), 
								end_it	=	m_items->end();
									it	!=	end_it;
								  ++it	)
	{
		if ( it->max_size != item::no_limit )
			total_size				+=	(it->max_size * 2) + (it->max_size < 10 ? 10 : 0);
	}

	return								total_size;
}

void   fat_inline_data::push_back (item const & item) 
{ 
	m_items->push_back					(item); 
	m_highest_compression_rate			=	math::max(m_highest_compression_rate, item.compression_rate); 
}

bool   fat_inline_data::try_fit_for_inlining (pcstr file_name, u32 const file_size, u32 const compressed_size)
{
	typedef item	item_type;
	item * item;
	if ( find_by_extension(& item, extension_from_path(file_name)) )
	{
		if ( compressed_size )
		{
			float const rate		=	(float)compressed_size / (file_size ? file_size : 1);
			if ( rate > item->compression_rate )
				return					false;
		}

		if ( item->max_size == item_type::no_limit )
			return						true;

		u32 const size				=	compressed_size ? compressed_size : file_size;
		if ( item->max_size >= size )
		{
			item->max_size			-=	size;
			return						true;
		}
	}

	return								false;
}

//-----------------------------------------------------------------------------------
// file_system
//-----------------------------------------------------------------------------------
file_system::file_system() : m_impl( FS_NEW(file_system_impl) )
{
}

file_system::~file_system()
{
	FS_DELETE							(m_impl);
}

void   file_system::mount_disk (pcstr fat_dir, pcstr disk_dir, watch_directory_bool const watch_directory)
{
	m_impl->mount_disk					(fat_dir, disk_dir, watch_directory);
}

bool   file_system::mount_db (	pcstr logical_path, 
								pcstr fat_file_path, 
								pcstr db_file_path,
								bool  need_replication)
{
	return m_impl->mount_db			(logical_path, 
										 fat_file_path, 
										 db_file_path && db_file_path[0] ? db_file_path : fat_file_path, 
										 need_replication);
}

void   file_system::unmount_disk (pcstr logical_path, pcstr disk_dir_path)
{
	m_impl->unmount_disk				(logical_path, disk_dir_path);
}

void   file_system::unmount_db (pcstr logical_path, pcstr fat_file_path)
{
	m_impl->unmount_db					(logical_path, fat_file_path);
}

bool   file_system::save_db (pcstr							fat_file_path, 
							 pcstr							db_file_path, 
							 bool							no_duplicates, 
							 u32							fat_alignment,
							 memory::base_allocator *		alloc,
							 compressor *					compressor,
							 float							compress_smallest_rate,
							 db_target_platform_enum		db_format,
							 fat_inline_data &				inline_data,
							 db_callback					callback)
{
	CURE_ASSERT							(fat_alignment != 0, return false);
	struct cleanup
	{
		FILE*	db_file;
		FILE*	fat_file;

		cleanup() : db_file(NULL), fat_file(NULL) {}
	   ~cleanup()
		{
			if ( db_file )
				fclose					(db_file);
			if ( db_file != fat_file && fat_file )
				fclose					(fat_file);
		}
	} data;

	if ( db_file_path )
	{
		if ( !open_file(&data.db_file, open_file_write | open_file_create | open_file_truncate, db_file_path) )
			return						false;
	}

	bool fat_in_db					=	!fat_file_path ||
										(db_file_path && strings::equal(db_file_path, fat_file_path));
		
	if ( fat_in_db )
	{
		data.fat_file				=	 data.db_file;
	}
	else
	{
		if ( !open_file(&data.fat_file, open_file_write | open_file_create | open_file_truncate, fat_file_path) )
			data.fat_file			=	NULL;
	}

	if ( !data.fat_file )
		return							false;
	
	if ( !m_impl->save_db (	data.fat_file, 
							data.db_file, 
							"", 
							no_duplicates, 
							fat_alignment,
							alloc,
							compressor,
							compress_smallest_rate,
							db_format, 
							inline_data,
							callback) )
	{
		return							false;
	}

	return								true;
}

bool   file_system::unpack (pcstr fat_dir, pcstr dest_dir, db_callback callback)
{
	return m_impl->unpack				(fat_dir, dest_dir, callback);
}

void   file_system::clear ()
{
	m_impl->clear						();
}

file_system::iterator   file_system::find (pcstr path)
{
	u32				hash;
	iterator		resources;

	resources.m_cur					=	m_impl->find_node(path, & hash);
	return								resources;
}

file_system::const_iterator   file_system::find (pcstr path) const
{
	return const_cast<file_system*>(this)->find(path); 
}

file_system::iterator   file_system::end () const
{
	iterator	resources;
	resources.m_cur					=	NULL;
	return								resources;
}

void   file_system::set_replication_dir (pcstr replication_dir)
{
	m_impl->set_replication_dir			(replication_dir);
}

void   file_system::get_disk_path (const iterator& it, path_string& out_path) const
{
	ASSERT								(it.m_cur);
	if ( it.m_cur )
		m_impl->get_disk_path			(it.m_cur, out_path);
	else
		out_path.clear					();
}

void   file_system::replicate_path (pcstr path2replicate, path_string& out_path) const
{
	m_impl->replicate_path				(path2replicate, out_path);
}

void   file_system::commit_replication (iterator it)
{
	m_impl->commit_replication			(it.m_cur);
}

bool   file_system::operator == (const file_system& f) const
{
	return								*m_impl == *f.m_impl;
}

bool   file_system::replicate_file (iterator it, pcbyte data)
{
	return	m_impl->replicate_file		(it.m_cur, data);
}

bool   file_system::equal_db (iterator const it1, iterator const it2) const
{
	ASSERT								(it1.m_cur && it2.m_cur);
	if ( !it1.m_cur || !it2.m_cur )
	{
		return							false;
	}

	return								m_impl->equal_db(it1.m_cur, it2.m_cur);
}

file_system::iterator   file_system::create_temp_disk_it (memory::base_allocator* const	alloc, 
														  pcstr const					disk_path)
{
	if ( !disk_path )
		return					end();
	
	file_size_type file_size		=	0;
	if ( !calculate_file_size(&file_size, disk_path) )
		return					end();

	pcstr const last_slash	=	strrchr(disk_path, '/');
	pcstr		name		=	last_slash ? last_slash + 1 : disk_path;

	iterator	result;
	fat_disk_file_node<> * file_node	=	m_impl->create_disk_file_node
								(0, name, strings::length(name), disk_path, strings::length(disk_path), (u32)file_size, alloc, false);

	result.m_cur			=	file_node->cast_node();
	return						result;
}

void   file_system::destroy_temp_disk_it (memory::base_allocator* const	alloc, iterator it)
{
	if ( it != end() )
		m_impl->destroy_temp_disk_node	(alloc, it.m_cur->cast_disk_file());
}

bool   file_system::mount_disk_node_by_logical_path (pcstr const logical_path, iterator * const out_iterator)
{
	return	m_impl->mount_disk_node	(logical_path, & out_iterator->m_cur);
}

bool   file_system::mount_disk_node_by_physical_path (pcstr const physical_path, iterator * const out_iterator)
{
	path_string		logical_path;
	if ( !m_impl->convert_physical_to_logical_path (& logical_path, physical_path, false) )
		return							false;

	return	m_impl->mount_disk_node		(logical_path.c_str(), & out_iterator->m_cur);
}

bool   file_system::unmount_disk_node (pcstr physical_path)
{
	path_string		logical_path;
	if ( !m_impl->convert_physical_to_logical_path(& logical_path, physical_path, false) )
		return							false;

	m_impl->unmount_disk	(logical_path.c_str(), physical_path, file_system_impl::respect_mount_history_false);
	return								true;
}

bool   file_system::update_file_size_in_fat	(pcstr physical_path)
{
	path_string		logical_path;
	if ( !m_impl->convert_physical_to_logical_path(& logical_path, physical_path, false) )
		return							false;

	return	m_impl->update_file_size_in_fat	(logical_path.c_str(), physical_path);
}

bool   file_system::rename_disk_node (pcstr old_physical_path, pcstr new_physical_path)
{
	path_string		logical_path;
	if ( !m_impl->convert_physical_to_logical_path(& logical_path, old_physical_path, false) )
		return							false;

	m_impl->rename_disk_node	(logical_path.c_str(), old_physical_path, new_physical_path);
	return								true;
}

signalling_bool   file_system::get_disk_path_to_store_file (pcstr const logical_path, buffer_string * const out_disk_path)
{
	return	m_impl->get_disk_path_to_store_file(logical_path, out_disk_path);
}

void   file_system::erase_disk_node (pcstr const physical_path)
{
	return	m_impl->erase_disk_node		(physical_path);
}

void   file_system::set_on_resource_leaked_callback (on_resource_leaked_callback callback)
{
	m_impl->set_on_resource_leaked_callback	(callback);
}

//-----------------------------------------------------------------------------------
// file_system::iterator
//-----------------------------------------------------------------------------------

pcstr   file_system::iterator::get_name () const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->m_name : NULL;
}

void   file_system::iterator::get_full_path (path_string & path) const
{
	if ( !m_cur )
		path						=	"<null fat_it>";
	else
		m_cur->get_full_path			(path);
}

path_string   file_system::iterator::get_full_path () const
{
	path_string		out_result;
	get_full_path						(out_result);
	return								out_result;
}

bool   file_system::iterator::is_folder() const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->is_folder() : false;
}

bool   file_system::iterator::is_disk() const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->is_disk() : false;
}

bool   file_system::iterator::is_db() const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->is_db() : false;
}

bool   file_system::iterator::is_replicated() const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->is_replicated() : false;
}

bool   file_system::iterator::is_compressed() const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->is_compressed() : false;
}

bool   file_system::iterator::is_inlined() const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->is_inlined() : false;
}

int   file_system::iterator::get_flags () const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->get_flags() : NULL;
}

void   file_system::iterator::set_flags (const u32 flags)
{
	ASSERT								(m_cur);
	if ( m_cur )
		m_cur->m_flags				=	(char)flags;
}

bool   file_system::iterator::get_hash (u32 * out_hash) const
{
	ASSERT								(m_cur);
	return								m_cur->get_hash(out_hash);
}

file_size_type   file_system::iterator::get_file_offs () const
{
	ASSERT								(m_cur);
	return								m_cur && !m_cur->is_folder() ? m_cur->get_file_offs() : NULL;
}

u32   file_system::iterator::get_raw_file_size () const
{
	ASSERT								(m_cur);
	return								(m_cur && !m_cur->is_folder()) ? m_cur->get_raw_file_size() : NULL;
} 

u32   file_system::iterator::get_compressed_file_size () const
{
	ASSERT								(m_cur);
	return								(m_cur && !m_cur->is_folder()) ? m_cur->get_compressed_file_size() : NULL;
} 

u32   file_system::iterator::get_file_size () const
{
	ASSERT								(m_cur);
	return								(m_cur && !m_cur->is_folder()) ? m_cur->get_file_size() : NULL;
}

u32   file_system::iterator::get_num_children () const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->get_num_children() : 0;
}

u32   file_system::iterator::get_num_nodes () const
{
	ASSERT								(m_cur);
	return								m_cur ? m_cur->get_num_nodes() : 0;
}

bool   file_system::iterator::operator == (const iterator& it) const
{
	return								m_cur == it.m_cur;
}

bool   file_system::iterator::operator != (const iterator& it) const
{
	return								!(*this == it);
}

bool   file_system::iterator::operator < (const iterator it) const
{
	return								(size_t)m_cur < (size_t)it.m_cur;
}

file_system::iterator   file_system::iterator::operator++ ()
{
	if ( m_cur )
	{
		m_cur						=	m_cur->get_next();
	}

	return *this;
}

file_system::iterator   file_system::iterator::operator++ (int)
{
	iterator saved						(*this);
	++*this;
	return								saved;
}

file_system::iterator   file_system::iterator::children_begin () const
{
	iterator it;
	it.m_cur						=	m_cur->get_first_child();
	return								it;
}

file_system::iterator   file_system::iterator::children_end () const
{
	iterator it;
	it.m_cur						=	NULL;
	return								it;
}

void   file_system::iterator::set_associated (resources::resource_base* const resource)
{
	R_ASSERT							(m_cur);
	if ( !m_cur )
		return;

	m_cur->set_associated				(resource);
}

resources::managed_resource_ptr   file_system::iterator::get_associated_managed_resource_ptr () const
{
	R_ASSERT							(m_cur);
	if ( !m_cur )
		return							NULL;

	return m_cur->get_associated_managed_resource_ptr();
}

resources::unmanaged_resource_ptr   file_system::iterator::get_associated_unmanaged_resource_ptr () const
{
	R_ASSERT							(m_cur);
	if ( !m_cur )
		return							NULL;

	return m_cur->get_associated_unmanaged_resource_ptr();
}

resources::query_result *   file_system::iterator::get_associated_query_result () const
{
	R_ASSERT							(m_cur);
	if ( !m_cur )
		return							NULL;

	return m_cur->get_associated_query_result();
}

bool   file_system::iterator::get_inline_data (const_buffer * out_buffer) const
{
	R_ASSERT							(m_cur);
	if ( !m_cur )
		return							false;

	return m_cur->get_inline_data(out_buffer);
}

bool   file_system::iterator::is_associated () const
{
	ASSERT								(m_cur);
	if ( !m_cur )
		return							NULL;

	return m_cur->is_associated			();
}

bool   file_system::iterator::is_associated_with (resources::resource_base * const resource_base) const
{
	ASSERT								(m_cur);
	if ( !m_cur )
		return							NULL;

	return m_cur->is_associated_with	(resource_base);
}

bool   file_system::iterator::try_clean_associated_if_zero_reference_resource () const
{
	ASSERT								(m_cur);
	if ( !m_cur )
		return							true;

	return m_cur->try_clean_associated_if_zero_reference_resource();
}

void   file_system::iterator::print (pcstr offs) const
{
	printf								("%s%s", offs, get_name());

	if ( is_disk() )
	{
		pcstr disk_path				=	NULL;
		if ( !is_folder() )
		{
			disk_path				=	m_cur->cast_disk_file()->m_disk_path;
			printf						("  (file: %s)\n", disk_path);
		}
		else
		{
			printf						("\r\n");
		}
	}
	else
	{
		printf							("\r\n");
	}

	fixed_string<64> child_offs		=	offs;
	child_offs						+=	"  ";

	iterator	cur_child			=	children_begin();
	iterator	end_child			=	children_end();

	while ( cur_child != end_child )
	{
		cur_child.print					(child_offs.c_str());
		++cur_child;
	}
}

void   file_system::iterator::set_is_locked (bool const lock)
{
	if ( !m_cur )
		return;
	set_fat_node_is_locked				(m_cur, lock);
}

//-----------------------------------------------------------------------------------
// file_system initialization
//-----------------------------------------------------------------------------------

bool   initialize_impl ();
void   finalize_impl ();

bool   initialize ()
{
	return	initialize_impl				();
}

void   finalize ()
{
	finalize_impl						();
}

} // namespace fs
} // namespace xray
