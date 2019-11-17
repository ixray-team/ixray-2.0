////////////////////////////////////////////////////////////////////////////
//	Created 	: 09.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "memory_monitor.h"

#if XRAY_USE_MEMORY_MONITOR

#include "core_entry_point.h"	// for xray::core::current_directory
#include <time.h>				// for _time64, _localtime64_s, strftime
#include <direct.h>				// for _mkdir
#include <xray/fs_utils.h>

namespace xray {
namespace memory {
namespace monitor {

using threading::mutex;

// constants
static bool						s_initialized = false;
static u32 const				s_buffer_size = 512*1024;
static char						s_buffer[ s_buffer_size ];
static FILE*					s_file = 0;
static pcstr					s_output_extension = ".bin";

static xray::uninitialized_reference<mutex>	s_mutex;

static command_line::key		s_memory_monitor_key("memory_monitor", "", "memory", "turns on monitoring all the memory oprations and dumping them to file");

union _allocation_size {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4201)
	struct {
		u32	allocation			: 1;
		u32 size				: 31;
	};
#pragma warning(pop)
#endif // #if defined(_MSC_VER)
	u32		allocation_size;
};

static inline bool enabled		( )
{
#if XRAY_USE_MEMORY_MONITOR
	static bool const result	= s_memory_monitor_key.is_set( );
	return						result;
#else // #if XRAY_USE_MEMORY_MONITOR
	return						false;
#endif // #if XRAY_USE_MEMORY_MONITOR
}

void initialize	( )
{
	ASSERT						( !s_initialized );

	if ( !enabled() )
		return;

	string_path					output_folder;
	strings::copy					( output_folder, xray::core::current_directory() );
	strcat_s					( output_folder, "memory_monitor\\");

	_mkdir						( output_folder );

	__time64_t					long_time;
	_time64						(&long_time);

	tm							new_time;
	errno_t error				= _localtime64_s( &new_time, &long_time);
	R_ASSERT					( !error );

	string_path					file_name;
	strftime					( file_name, sizeof( file_name ), "%Y.%m.%d.%H.%M.%S", &new_time );

	string_path					file;
	strings::join				( file, output_folder, file_name, s_output_extension );

	ASSERT						( !s_file );
	if ( !fs::open_file ( &s_file, fs::open_file_create | fs::open_file_truncate | fs::open_file_write, file ) )
		NODEFAULT				( );

	ASSERT						( s_file );

	setvbuf						( s_file, s_buffer, _IOFBF, s_buffer_size );

	XRAY_CONSTRUCT_REFERENCE	( s_mutex, mutex );

	s_initialized				= true;
}

void finalize		( )
{
	if ( !enabled() )
		return;

	ASSERT						( s_initialized );

	XRAY_DESTROY_REFERENCE		( s_mutex );

	fclose						( s_file );
	s_file						= 0;

	s_initialized				= false;
}

void on_alloc	( pvoid& allocation_address, size_t& allocation_size, size_t const previous_size, pcstr const allocation_description )
{
	XRAY_UNREFERENCED_PARAMETER	( previous_size );

	if ( !enabled() )
		return;

	s_mutex->lock				( );

	_allocation_size			temp;
	temp.allocation				= 1;
	temp.size					= allocation_size;
	fwrite						( &temp, sizeof( temp ), 1, s_file );
	fwrite						( &allocation_address, sizeof( allocation_address ), 1, s_file );
	fwrite						( allocation_description, ( xray::strings::length( allocation_description ) + 1 )*sizeof( char ), 1, s_file );

	static bool flush = false;
	if (flush)
		fflush					( s_file );

	s_mutex->unlock				( );
}

void on_free	( pvoid& deallocation_address, bool const can_clear )
{
	XRAY_UNREFERENCED_PARAMETER	( can_clear );

	if ( !enabled() )
		return;

	s_mutex->lock				( );

	if ( deallocation_address ) {
		_allocation_size		temp;
		temp.allocation_size	= 0;
		fwrite					( &temp, sizeof( temp ), 1, s_file );
		fwrite					( &deallocation_address, sizeof( deallocation_address ), 1, s_file );
	}

	s_mutex->unlock				( );
}

} // namespace monitor
} // namespace memory
} // namespace xray

#endif // #if XRAY_USE_MEMORY_MONITOR