////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "fs_file_system.h"
#include "fs_helper.h"
#include <xray/fs_utils.h>
#include "xray/os_include.h"
#include <xbox.h>

namespace xray {
namespace fs   {

XRAY_CORE_API bool	make_dir(pcstr path);

bool initialize_impl			( )
{
	long const result				= XMountUtilityDrive( false, 64*1024, 64*1024 );
	XRAY_UNREFERENCED_PARAMETER		( result );
	R_ASSERT						( result == ERROR_SUCCESS );

	pcstr const replications_dir	= "cache:/replication";
 	make_dir						( replications_dir );

	XRAY_CONSTRUCT_REFERENCE		( g_fat, file_system );
	g_fat->set_replication_dir		( replications_dir );
	
	return							true;
}

void finalize_impl ( )
{
	XRAY_DESTROY_REFERENCE			( g_fat );
}

void flush_replications	( )
{
	XFlushUtilityDrive				( );
}

} // namespace fs
} // namespace xray