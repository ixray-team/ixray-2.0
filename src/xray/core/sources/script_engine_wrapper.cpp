////////////////////////////////////////////////////////////////////////////
//	Created		: 06.03.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "script_engine_wrapper.h"
#include <xray/fs_utils.h>

using xray::core::script_engine_wrapper;
using cs::script::file_handle;
using cs::script::file_buffer;

script_engine_wrapper::script_engine_wrapper			( pcstr resource_path, pcstr underscore_G_path )
{
	strings::join			( m_resource_path, resource_path, "/" );
	strings::join			( m_underscore_G_path, underscore_G_path, "/" );
}

void script_engine_wrapper::log							( cs::message_initiator const &message_initiator, cs::core::message_type const &message_type, const char *string )
{
	XRAY_UNREFERENCED_PARAMETER	(message_initiator);

	logging::verbosity verbosity	= logging::trace;
	switch ( message_type ) {
		case cs::core::message_type_error : {
			verbosity		= logging::error;
			break;
		}
		case cs::core::message_type_warning : {
			verbosity		= logging::warning;
			break;
		}
		case cs::core::message_type_information : {
			verbosity		= logging::info;
			break;
		}
		case cs::core::message_type_output : {
			verbosity		= logging::debug;
			break;
		}
		default : NODEFAULT();
	}

	LOGI					( "script", verbosity, "%s", string );
}

pcstr script_engine_wrapper::get_file_name	( int const file_type, pcstr const file_name, pstr const result, u32 const result_size, bool add_extension )
{
	pcstr					extension = "";
	pcstr					folder = "";
	switch (file_type) {
		case (cs::script::engine::file_type_script) : {
			if ( add_extension )
				extension	= ".lua";

			if ( (file_name[0] == '_') && (file_name[1] == 'G') && (file_name[2] == 0) )
				folder		= m_underscore_G_path;
			else
				folder		= m_resource_path;
			break;
		}
		case (cs::script::engine::file_type_script_chunk) : {
			if ( add_extension )
				extension	= ".lua_chunk";

			folder			= m_resource_path;
			break;
		}
		case (cs::script::engine::file_type_config) : {
			UNREACHABLE_CODE();
			break;
		}
		default :			return(false);
	}

	xray::buffer_string dest(result, result_size);
	dest					= folder;
	dest					+= file_name;
	dest					+= extension;

	return					(result);
}

bool script_engine_wrapper::file_exists					( int file_type, const char *file_name )
{
	string_path				real_file_name;
	FILE*					file;
	if ( !fs::open_file( &file, fs::open_file_read, get_file_name( file_type, file_name, real_file_name, sizeof(real_file_name), false), false) ) {
		if ( !fs::open_file( &file, fs::open_file_read, get_file_name( file_type, file_name, real_file_name, sizeof(real_file_name), true), false) )
			return			false;
	}

	fclose					( file );
	return					true;
}

file_handle script_engine_wrapper::open_file_buffer		( int file_type, const char *file_name, cs::script::file_buffer &file_buffer, u32 &buffer_size )
{
	string_path				real_file_name;
	FILE*					file;
	if ( !fs::open_file( &file, fs::open_file_read, get_file_name( file_type, file_name, real_file_name, sizeof(real_file_name), false ), false ) ) {
		if ( !fs::open_file( &file, fs::open_file_read, get_file_name( file_type, file_name, real_file_name, sizeof(real_file_name), true ), false ) ) {
			FATAL			( "cannot open file %s", real_file_name );
			return			0;
		}
	}
	
	buffer_size				= (u32)fseek64(file,0,SEEK_END);
	fseek64					( file, 0, SEEK_SET);
	pvoid const buffer		= MT_MALLOC( (u32)buffer_size, "script" );
	fread					( buffer, 1, buffer_size, file );
	fclose					( file );

	file_buffer				= buffer;
	return					( (cs::script::file_handle)file_buffer );
}

void script_engine_wrapper::close_file_buffer			( cs::script::file_handle file_handle )
{
	ASSERT					( file_handle );

	pvoid buffer			= (void*)file_handle;
	MT_FREE					( buffer );
}

bool script_engine_wrapper::create_file					( int file_type, const char *file_name, const cs::script::file_buffer &file_buffer, const u32 &buffer_size )
{
	XRAY_UNREFERENCED_PARAMETERS(file_type, file_name, file_buffer, buffer_size);
	UNREACHABLE_CODE		(return false);
}

void script_engine_wrapper::lua_studio_backend_file_path( int file_type, const char *file_name, char *path, const u32 &max_size )
{
	FILE*					file;
	if ( !fs::open_file( &file, fs::open_file_read, get_file_name( file_type, file_name, path, max_size, false), false) ) {
		if ( !fs::open_file( &file, fs::open_file_read, get_file_name( file_type, file_name, path, max_size, true), false) )
			UNREACHABLE_CODE( );
	}
	fclose					( file );
}