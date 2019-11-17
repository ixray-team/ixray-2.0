////////////////////////////////////////////////////////////////////////////
//	Created		: 07.05.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "fs_file_system.h"
#include "fs_helper.h"
#include "fs_file_system_nodes_impl.h"
#include <xray/fs_utils.h>
#include <sys/paths.h>

namespace xray {
namespace fs   {

bool initialize_impl			( )
{
	pcstr const replication_dir		= SYS_DEV_HDD0 "/replication";
 	make_dir						( replication_dir );

	XRAY_CONSTRUCT_REFERENCE		( g_fat, file_system );
	g_fat->set_replication_dir		( replication_dir );

	return							( true );
}

void finalize_impl ( )
{
	XRAY_DESTROY_REFERENCE			( g_fat );
}

void flush_replications	( )
{
}

} // namespace fs
} // namespace xray