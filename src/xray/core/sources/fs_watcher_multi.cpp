////////////////////////////////////////////////////////////////////////////
//	Created		: 15.12.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "fs_watcher.h"

namespace xray {
namespace fs {

bool   watcher_enabled ()
{
	return								false;
}

threading::mutex &   mutex_folder_watcher ()
{
	static threading::mutex	s_dummy;
	return								s_dummy;
}

bool   watcher_is_initialized ()
{
	return								false;
}

void   initialize_watcher ()
{
}

void   finalize_watcher ()
{
}

void   watch_physical_directory (pcstr const physical_path)
{
	XRAY_UNREFERENCED_PARAMETER	(physical_path);
}

void   quit_watching_physical_directory (pcstr const physical_path)
{
	XRAY_UNREFERENCED_PARAMETER	(physical_path);
}

void   wakeup_watcher_thread () 
{ 
}

u32   watcher_thread_id	() 
{
	return									(u32)-1;
}

} // namespace fs
} // namespace xray