////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "core_entry_point.h"
#include <xray/os_include.h>

namespace xray {
namespace core {

static string512 s_current_directory	= "";

void platform::preinitialize( )
{
#ifdef DEBUG
	DmMapDevkitDrive	( );
#endif // #ifdef DEBUG
}

pcstr current_directory		( )
{
	if ( ! * s_current_directory )
		strings::copy	( s_current_directory, "game:\\" );

	return				s_current_directory;
}

} // namespace core
} // namespace xray