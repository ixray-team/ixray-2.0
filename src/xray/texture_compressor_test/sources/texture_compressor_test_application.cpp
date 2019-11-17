////////////////////////////////////////////////////////////////////////////
//	Created		: 19.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "texture_compressor_test_application.h"
#include "texture_compressor_test_memory.h"
#include <xray/core/core.h>
#include <xray/core/simple_engine.h>
#include <xray/os_include.h>		// for GetCommandLine
#include <io.h>

using xray::texture_compressor_test::application;

xray::memory::doug_lea_allocator_type	xray::texture_compressor_test::g_allocator;

typedef xray::core::simple_engine							core_engine_type;
static xray::uninitialized_reference< core_engine_type >	s_core_engine;

void application::initialize( )
{
	m_exit_code				= 0;

	XRAY_CONSTRUCT_REFERENCE( s_core_engine, core_engine_type );

	core::preinitialize		(
		&*s_core_engine,
		GetCommandLine(), 
		command_line::contains_application_true,
		"texture_compressor_test",
		__DATE__
	);

	g_allocator.do_register	( 16*1024*1024, "texture_compressor_test" );

	memory::allocate_region	( );

	core::initialize		( "texture_compressor_test", core::create_log, core::perform_debug_initialization );
}

void application::finalize	( )
{
	core::finalize			( );

	XRAY_DESTROY_REFERENCE	( s_core_engine );
}

#include <amd/compress/texture_compressor/api.h>

static xray::command_line::key
	s_target_format(
		"target_format",
		"tf",
		"texture compressor",
		""
	);

static xray::command_line::key
	s_input_file(
		"input_file",
		"i",
		"texture compressor",
		""
	);

static xray::command_line::key
	s_output_file(
		"output_file",
		"o",
		"texture compressor",
		""
	);

static xray::command_line::key
	s_compression_speed(
		"compression_speed",
		"cs",
		"texture compressor",
		""
	);

static xray::command_line::key
	s_enable_multithreading(
		"enable_multithreading",
		"em",
		"texture compressor",
		""
	);

pvoid texture_compressor_allocate	( u32 size, bool temp_data )
{
	XRAY_UNREFERENCED_PARAMETER		( temp_data );
	return							MALLOC( size, "texture_compressor" );
}

void texture_compressor_deallocate	( pvoid buffer )
{
	FREE							( buffer );
}

void application::execute	( )
{
	fixed_string<512>				format_string;
	if ( !s_target_format.is_set_as_string(&format_string) ) {
		LOG_ERROR					( "please specify texture format" );
		m_exit_code					= u32(-1);
		return;
	}

	ATI_TC_FORMAT const target_format	= texture_compressor::parse_format( format_string.c_str() );
	if ( target_format == ATI_TC_FORMAT_Unknown ) {
		LOG_ERROR					( "unknown texture format" );
		m_exit_code					= u32(-2);
		return;
	}

	fixed_string<512>				input_file;
	if ( !s_input_file.is_set_as_string(&input_file) ) {
		LOG_ERROR					( "please specify input file" );
		m_exit_code					= u32(-3);
		return;
	}

	fixed_string<512>				output_file;
	if ( !s_output_file.is_set_as_string(&output_file) ) {
		LOG_ERROR					( "please specify output file" );
		m_exit_code					= u32(-4);
		return;
	}

	float compression_speed			= -1.f;
	if ( !s_compression_speed.is_set_as_number(&compression_speed) ) {
		LOG_ERROR					( "please specify compression speed" );
		m_exit_code					= u32(-5);
		return;
	}

	FILE* file;
	errno_t error					= fopen_s( &file, input_file.c_str(), "rb" );
	if ( error ) {
		m_exit_code					= u32(-6);
		return;
	}

	u32 const buffer_size			= _filelength( _fileno(file) );
	u8* buffer						= (u8*)MALLOC(buffer_size,"tga_source");
	u32 const read_bytes			= (u32)fread_s( buffer, buffer_size, 1, buffer_size, file );
	if ( read_bytes != buffer_size ) {
		m_exit_code					= u32(-7);
		return;
	}

	fclose							( file );

	u32 new_buffer_size				= 0;
	u8* destination_buffer			=
		create_texture(
			new_buffer_size,
			&texture_compressor_allocate,
			&texture_compressor_deallocate,
			buffer,
			target_format,
			(texture_compressor::compression_speed)math::floor(compression_speed),
			s_enable_multithreading.is_set()
		);

	FREE							( buffer );

	error							= fopen_s( &file, output_file.c_str(), "wb" );
	if ( error ) {
		m_exit_code					= u32(-8);
		return;
	}

	fwrite							( destination_buffer, 1, new_buffer_size, file );
	fclose							( file );

	FREE							( destination_buffer );

	m_exit_code						= s_core_engine->get_exit_code();
}