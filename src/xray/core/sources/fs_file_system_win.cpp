////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "fs_file_system.h"
#include "fs_helper.h"
#include "fs_file_system_nodes_impl.h"

namespace xray {
namespace fs   {

XRAY_CORE_API bool	make_dir(pcstr path);

bool initialize_impl			( )
{
	pcstr const replications_dir	= "c:/replication";
 	make_dir						( replications_dir );

	XRAY_CONSTRUCT_REFERENCE		( g_fat, file_system );
	g_fat->set_replication_dir		( replications_dir );
	
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