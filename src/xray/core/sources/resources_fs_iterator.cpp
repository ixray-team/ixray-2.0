////////////////////////////////////////////////////////////////////////////
//	Created		: 03.03.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_manager.h"
#include <xray/resources_fs.h>
#include <xray/os_preinclude.h>
#include <xray/os_include.h>
#include <xray/fs_utils.h>

namespace xray {
namespace resources {

using namespace fs;

void   log_fs_iterator_impl (fs_iterator it, buffer_string & prefix)
{
	if ( it.is_end() )
		return;

	fs::path_string	full_path;
	it.get_full_path					(full_path);
	pcstr const name				=	it.get_name();

	fs::path_string	file_info;
	if ( !it.is_folder() )
		file_info.assignf				("size (%d)", it.get_file_size());
	if ( it.is_compressed() )
		file_info.appendf				(" compressed (%d)", it.get_compressed_file_size());

	LOG_INFO							("%s %s (%s) %s", 
										 prefix.c_str(), 
										 name, 
										 full_path.c_str(), 
										 file_info.c_str());

	char const prefix_inc[]			=	"  ";
	u32 const prefix_inc_len		=	array_size(prefix_inc) - 1;
	prefix							+=	prefix_inc;
	for ( fs_iterator			jt	=	it.children_begin();
								jt	!=	it.children_end();
								++jt )
	{
		log_fs_iterator_impl			(jt, prefix);
	}
	
	prefix.set_length					(prefix.length() - prefix_inc_len);
}

void   log_fs_iterator (fs_iterator it)
{
	fixed_string<1024>	prefix;
	log_fs_iterator_impl					(it, prefix);
}

void   query_fs_iterator (pcstr							path, 
						  query_fs_iterator_callback	callback, 
						  memory::base_allocator*		allocator)
{
	ASSERT									(callback);

	g_resources_manager->inc_num_current_fs_ops();
	
	XRAY_MEMORY_BARRIER_FULL				();

	if ( !g_resources_manager->has_no_pending_mount_ops() )
	{
		g_resources_manager->dec_num_current_fs_ops();

		// ASYNC WAY
		threading::mutex & mount_transaction_mutex	
										=	g_resources_manager->get_mount_transaction_mutex();
		threading::mutex_raii raii			(mount_transaction_mutex);
		
		fs_task *	new_task			=	XRAY_NEW_IMPL(* allocator, fs_task)(fs_task::type_fs_iterator_task);
		new_task->fs_iterator_path		=	path;
		new_task->allocator				=	allocator;
		new_task->fs_iterator_callback	=	callback;

		g_resources_manager->add_fs_task		(new_task);
	}
	else
	{
		// SYNC WAY
		fs_iterator	it;
		it.m_fat_it						=	fat_it_to_wrapper( fs::g_fat->find(path) );

		callback							(it);

		g_resources_manager->dec_num_current_fs_ops();
	}
}

struct query_fs_iterator_and_wait_callback_proxy_pred
{
	query_fs_iterator_and_wait_callback_proxy_pred(query_fs_iterator_callback const callback) : callback_(callback), receieved_callback_(false) {}
	void callback (fs_iterator result)
	{
		callback_				(result);
		receieved_callback_	=	true;
	}

	bool received_callback		() const { return receieved_callback_; }

private:
	bool						receieved_callback_;
	query_fs_iterator_callback	callback_;
};

void   query_fs_iterator_and_wait	(pcstr							path, 							 		 query_fs_iterator_callback		callback, 
							 		 memory::base_allocator*		allocator)
{
	query_fs_iterator_and_wait_callback_proxy_pred		callback_proxy	(callback);
	query_fs_iterator				(path, boost::bind(& query_fs_iterator_and_wait_callback_proxy_pred::callback, & callback_proxy, _1), allocator);

	while ( !callback_proxy.received_callback() )
	{
		if ( threading::g_debug_single_thread )
			resources::tick			();

		dispatch_callbacks			();
	}
}

fs_iterator	  fs_iterator::find_child (pcstr relative_path)
{
	path_string					full_path;
	get_full_path				(full_path);
	full_path				+=	'/';
	full_path				+=	relative_path;
	fs_iterator					out_result;
	out_result.m_fat_it		=	fat_it_to_wrapper(g_fat->find(full_path.c_str()));
	return						out_result;
}

pcstr   fs_iterator::get_name () const{
	return						wrapper_to_fat_it(m_fat_it).get_name();
}

void   fs_iterator::get_full_path (fs::path_string& path) const
{
	wrapper_to_fat_it(m_fat_it).get_full_path(path);
}

void   fs_iterator::get_disk_path (fs::path_string& path) const
{
	g_fat->get_disk_path		(wrapper_to_fat_it(m_fat_it), path);
}

u32   fs_iterator::get_num_children () const
{
	return						wrapper_to_fat_it(m_fat_it).get_num_children();
}

u32   fs_iterator::get_num_nodes () const
{
	return						wrapper_to_fat_it(m_fat_it).get_num_nodes();
}

bool   fs_iterator::is_folder () const
{
	return						wrapper_to_fat_it(m_fat_it).is_folder();
}

bool   fs_iterator::is_disk () const
{
	return						wrapper_to_fat_it(m_fat_it).is_disk();
}

bool   fs_iterator::is_db () const
{
	return						wrapper_to_fat_it(m_fat_it).is_db();
}

bool   fs_iterator::is_replicated () const
{
	return						wrapper_to_fat_it(m_fat_it).is_replicated();
}

bool   fs_iterator::is_compressed () const
{
	return						wrapper_to_fat_it(m_fat_it).is_compressed();
}

u32   fs_iterator::get_file_size () const
{
	return						wrapper_to_fat_it(m_fat_it).get_file_size();
}

u32   fs_iterator::get_compressed_file_size () const
{
	return						wrapper_to_fat_it(m_fat_it).get_compressed_file_size();
}

u32   fs_iterator::get_raw_file_size () const
{
	return						wrapper_to_fat_it(m_fat_it).get_raw_file_size();
}

file_size_type	 fs_iterator::get_file_offs () const
{
	return						wrapper_to_fat_it(m_fat_it).get_file_offs();
}

fs_iterator   fs_iterator::children_begin () const
{
	fs_iterator it;
	it.m_fat_it				=	m_fat_it ? 
								fat_it_to_wrapper(wrapper_to_fat_it(m_fat_it).children_begin()) : NULL;

	return						it;
}

fs_iterator   fs_iterator::children_end	() const
{
	fs_iterator it;
	it.m_fat_it				=	m_fat_it ? 
								fat_it_to_wrapper(wrapper_to_fat_it(m_fat_it).children_end()) : NULL;
	return						it;
}

fs_iterator   fs_iterator::operator ++ ()
{
	fat_iterator	it		=	wrapper_to_fat_it(m_fat_it);
	++it;
	m_fat_it				=	fat_it_to_wrapper(it);
	return						*this;
}

fs_iterator   fs_iterator::operator ++	(int)
{
	fs_iterator	prev_value	=	*this;
	++*this;
	return						prev_value;
}

bool   fs_iterator::operator == (const fs_iterator& it) const
{
	return						wrapper_to_fat_it(m_fat_it) == wrapper_to_fat_it(it.m_fat_it);
}

bool   fs_iterator::operator !=	(const fs_iterator& it) const
{
	return						!(*this == it);
}

bool   fs_iterator::operator <	(const fs_iterator it) const
{
	return						wrapper_to_fat_it(m_fat_it) < wrapper_to_fat_it(it.m_fat_it);
}

using namespace fs;

struct get_path_info_predicate
{
public:
	get_path_info_predicate	(path_info::type_enum *	out_result, 
							 path_info *			out_info)
	: m_out_info(out_info), m_out_result(out_result) {}

	void callback (fs_iterator it)
	{
		path_string	disk_path;
		if ( it.is_end() )
		{
			* m_out_result	=	fs::path_info::type_nothing;
			if ( m_out_info )
				m_out_info->type	=	fs::path_info::type_nothing;
		}
		else
		{
			it.get_disk_path		(disk_path);
			* m_out_result		=	get_path_info(m_out_info, disk_path.c_str());
		}
	}

	path_info *				m_out_info;
	path_info::type_enum *	m_out_result;
};

path_info::type_enum   get_path_info_by_logical_path (path_info * out_path_info, pcstr logical_path, memory::base_allocator * allocator)
{
	path_info::type_enum   out_result = path_info::type_nothing;
	get_path_info_predicate		predicate(& out_result, out_path_info);

	query_fs_iterator_and_wait	(logical_path, 
								 boost::bind(& get_path_info_predicate::callback, & predicate, _1),
								 allocator);

	return						out_result;
}

} // namespace resources
} // namespace xray
