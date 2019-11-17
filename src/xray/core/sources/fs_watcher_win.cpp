////////////////////////////////////////////////////////////////////////////
//	Created		: 15.12.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "fs_watcher.h"
#include "fs_macros.h"
#include "fs_folder_watcher_win.h"
#include "fs_file_system.h"
#include "fs_file_system_impl.h"
#include <xray/intrusive_double_linked_list.h>
#include <xray/resources.h>

namespace xray {
namespace fs {

class watcher
{
public:
			watcher								(file_change_callback callback);

	void	thread_proc							();

	void	watch_physical_directory			(pcstr physical_directory);
	void	quit_watching_physical_directory	(pcstr physical_directory);

	threading::mutex &	mutex_folder_watcher	() { return m_mutex_folder_watcher; }
	void	wakeup_watcher_thread				() { m_event_folder_recieved_changes.set(1); }
	u32		watcher_thread_id					() const { return m_watcher_thread_id; }

			~watcher							();

private:
	typedef intrusive_double_linked_list<	folder_watcher, 
											folder_watcher, 
											& folder_watcher::m_prev_watcher, 
											& folder_watcher::m_next_watcher, 
											threading::single_threading_policy	>	folder_watchers;

	threading::mutex							m_mutex_folder_watcher;
	folder_watchers								m_folder_watchers;
	file_change_callback						m_callback;

	threading::event							m_event_folder_recieved_changes;
	threading::atomic32_type					m_destroying;
	threading::atomic32_type					m_thread_proc_ended;
	u32											m_watcher_thread_id;
};

watcher::watcher (file_change_callback callback) : 
			m_destroying(false), m_thread_proc_ended(false), m_watcher_thread_id(0), m_callback(callback)
{
	m_watcher_thread_id			=	threading::spawn (
											boost::bind(&watcher::thread_proc, this), 
											"file system monitor", 
											"FS-monitor", 
											0,
											3 % threading::core_count(),
											threading::tasks_unaware
										);
}

watcher::~watcher ()
{
	threading::interlocked_exchange		(m_destroying, true);
	m_event_folder_recieved_changes.set	(true);
	while ( !m_thread_proc_ended )		{ ; }

	struct delete_predicate
	{
		void operator ()(folder_watcher * folder_watcher)
		{
			FS_DELETE					(folder_watcher);
		}
	};

	delete_predicate					delete_predicate;
	m_folder_watchers.for_each			(delete_predicate);
}

void   watcher::thread_proc ()
{
	struct try_recieve_changes_predicate
	{
		void operator () (folder_watcher * const folder_watcher)
		{
			folder_watcher->try_receive_changes	();
		}
	};

	for ( ;; )
	{
		m_event_folder_recieved_changes.wait	(threading::event::wait_time_infinite);
		if ( m_destroying )
			break;

	#pragma message (XRAY_TODO("fix it"))
	// resources::dispatch_callbacks			();

		threading::mutex_raii	raii			(m_mutex_folder_watcher);
		try_recieve_changes_predicate			try_recieve_changes_predicate;
		m_folder_watchers.for_each				(try_recieve_changes_predicate);
	}	

	resources::finalize_thread_usage	(false);

	threading::interlocked_exchange		(m_thread_proc_ended, true);
}

struct find_watcher_predicate
{
	enum find_enum	{ find_exact, find_as_subfolder };

	find_watcher_predicate	(find_enum find, pcstr physical_path) 
				: find(find), physical_path(physical_path), found_watcher(NULL) {}

	void operator () (folder_watcher * const folder_watcher)
	{
		if ( find == find_exact )
		{
			if ( physical_path == folder_watcher->path() )
				found_watcher		=	folder_watcher;
		}
		else
		{
			fs::path_string watcher_path_with_slash	=	folder_watcher->path();
			if ( watcher_path_with_slash.length() )
				watcher_path_with_slash				+=	'/';
						
			if ( physical_path.find(watcher_path_with_slash.c_str()) == 0 ||
				 physical_path == folder_watcher->path() )
			{
				found_watcher		=	folder_watcher;
			}
		}
	}

	find_enum							find;
	folder_watcher *					found_watcher;
	fixed_string512						physical_path;
}; // find_watcher_predicate

void   watcher::watch_physical_directory (pcstr physical_path)
{
	threading::mutex_raii	raii			(m_mutex_folder_watcher);
	find_watcher_predicate					find_watcher(find_watcher_predicate::find_as_subfolder, physical_path);
	m_folder_watchers.for_each				(find_watcher);

	if ( find_watcher.found_watcher )
		return; // already watching

	folder_watcher * const new_watcher	=	FS_NEW(folder_watcher)
											(physical_path, 
											 m_callback, 
											 & m_event_folder_recieved_changes);
	
	m_folder_watchers.push_back				(new_watcher);

	m_event_folder_recieved_changes.set		(true);
}

void   watcher::quit_watching_physical_directory (pcstr physical_path)
{
	threading::mutex_raii	raii			(m_mutex_folder_watcher);
	find_watcher_predicate					find_watcher(find_watcher_predicate::find_exact, physical_path);
	m_folder_watchers.for_each				(find_watcher);

	CURE_ASSERT								(find_watcher.found_watcher, return, "not watching directory: %s", physical_path);
	m_folder_watchers.erase					(find_watcher.found_watcher);

	FS_DELETE								(find_watcher.found_watcher);	
}

static uninitialized_reference<watcher>		s_watcher;

static command_line::key					s_no_fs_watch("no_fs_watch", "", "file system", "disables file system changes watching");

bool   watcher_enabled ()
{
	return									!s_no_fs_watch;
}

threading::mutex &   mutex_folder_watcher ()
{
	return									s_watcher->mutex_folder_watcher();
}

bool   watcher_is_initialized ()
{
	return									s_watcher.initialized();
}

void   initialize_watcher ()
{
	if ( !watcher_enabled() )
		return;

	XRAY_CONSTRUCT_REFERENCE				(s_watcher, watcher)
											(file_change_callback(g_fat->get_impl(), & file_system_impl::on_disk_file_change));
}

void   finalize_watcher ()
{
	if ( !watcher_enabled() )
		return;

	XRAY_DESTROY_REFERENCE					(s_watcher);
}

void   watch_physical_directory (pcstr const physical_path)
{
	s_watcher->watch_physical_directory		(physical_path);
}

void   quit_watching_physical_directory (pcstr const physical_path)
{
	s_watcher->quit_watching_physical_directory	(physical_path);
}

void   wakeup_watcher_thread () 
{ 
	s_watcher->wakeup_watcher_thread		();
}

u32   watcher_thread_id	() 
{
	return									s_watcher->watcher_thread_id(); 
}

} // namespace fs
} // namespace xray


