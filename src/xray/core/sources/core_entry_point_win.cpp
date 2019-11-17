////////////////////////////////////////////////////////////////////////////
//	Created 	: 27.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "core_entry_point.h"
#include <xray/os_include.h>
#include "platform_extensions_win.h"
#include <xray/fs_path.h>

namespace xray {
namespace core {

static string512	s_application		=	"";

void platform::preinitialize( )
{
}

void   set_application_name ( pcstr application_name )
{
	strings::copy							(s_application, application_name);
}

pcstr   application_name ( )
{
	if ( ! * s_application )
	{
		string512	path;
		if ( GetModuleFileName( 0, path, sizeof(path) ) )
		{
			pstr const last_back_slash	=	strrchr ( path, '\\' );
			if ( last_back_slash )
				strings::copy				(s_application, last_back_slash + 1);
			else
				strings::copy				(s_application, path);
		}
	}

	return									s_application;
}

pcstr   user_name ( )
{
	static string512	s_user			=	"";
	static bool			s_initialized	=	false;

	if ( ! s_initialized )
	{
		DWORD buffer_size				=	sizeof(s_user);
		GetUserName							( s_user, & buffer_size );
		R_ASSERT							( xray::platform::unload_delay_loaded_library("advapi32.dll") );
		s_initialized					=	true;
	}

	return									s_user;
}

pcstr   current_directory ( )
{
	static string512	s_current_directory	=	"";
	static bool			s_initialized	=	false;

	if ( ! * s_current_directory )
	{
		DWORD string_size = GetModuleFileName( 0, s_current_directory, sizeof(s_current_directory) );
		if ( string_size )
		{
			pstr backslash				=	strrchr(s_current_directory, '\\');
			if ( backslash )
				* ( backslash + 1 )		=	NULL;

			fs::convert_to_portable_in_place	( s_current_directory );
		}

		s_initialized					=	true;
	}

	return									s_current_directory;
}

pcstr user_data_directory	( )
{
	static string512	s_user_data_directory = "";
	static bool			s_initialized = false;

	if ( !*s_user_data_directory )
	{
		strings::join	( s_user_data_directory, current_directory(), "../../user_data/" );
		s_initialized	= true;
	}

	return				s_user_data_directory;
}

} // namespace core
} // namespace xray