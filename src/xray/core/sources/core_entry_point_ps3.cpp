////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "core_entry_point.h"
#include <sys/paths.h>

namespace xray {

namespace command_line {
	extern fixed_string4096	g_command_line;
} // namespace command_line

namespace core {

static string512 s_current_directory	= "";

void platform::preinitialize( )
{
}

pcstr   get_command_line ( )
{
	pcstr const result	= command_line::g_command_line.c_str( );
	return				result ? result : "";
}

pcstr current_directory		( )
{
	if ( ! * s_current_directory )
		strings::copy	( s_current_directory, SYS_APP_HOME "/" );

	return				s_current_directory;
}

} // namespace core
} // namespace xray