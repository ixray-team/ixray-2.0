#include "pch.h"
#include "rms_generator_application.h"
#include "rms_generator_memory.h"
#include "xray/buffer_string.h"
#include <xray/core/core.h>
#include <xray/os_include.h>
#include "rms_evaluator.h"
#include <xray/core/simple_engine.h>

using namespace xray;

static xray::command_line::key				s_input_filename	("input_file_name",		"i", "RMS Generator", "set input file name");

namespace rms_generator
{

	xray::memory::doug_lea_allocator_type	g_rms_generator_allocator;
	static xray::core::simple_engine		s_core_engine;

	void application::initialize	( u32 argc, pstr const* const argv )
	{
		m_argv								=	argv;
		m_argc								=	argc;

		xray::core::preinitialize			(
			&s_core_engine,
			GetCommandLine(), 
			command_line::contains_application_true,
			"rms_generator",
			__DATE__
		);

		g_rms_generator_allocator.do_register			(64*1024*1024,	"rms_generator");
		g_rms_generator_allocator.user_current_thread_id();

		xray::memory::allocate_region		( );
		xray::core::initialize				( "rms_generator", core::create_log, core::perform_debug_initialization );
	}

	void   application::finalize	()
	{
		xray::core::finalize				( );
	}

	void   application::execute		()
	{
		m_exit_code							=	0;

		if(!s_input_filename.is_set())
		{
			printf							("Input file name is not typed! Please type input file name for process.");
			return;
		}
		char buffer[256];
		xray::buffer_string input_file_name(&buffer[0], array_size(buffer));

		s_input_filename.is_set_as_string	(&input_file_name);
		rms_generator::process_file			(input_file_name.c_str());

		m_exit_code							=	s_core_engine.get_exit_code();
	}

} // namespace rms_generator