////////////////////////////////////////////////////////////////////////////
//	Created 	: 06.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "debug.h"
#include "logging.h"
#include "fs_file_system.h"
#include "fs_watcher.h"
#include "resources_manager.h"
#include "game_resources_manager.h"
#include "strings_shared_manager.h"
#include "build_extensions.h"
#include "core_entry_point.h"
#include "timing.h"
#include <xray/memory_base_allocator.h>
#include "memory.h"
#include "testing_impl.h"
#include <locale.h>			// for setlocale
#include <xray/compressor_ppmd.h>
#include <xray/core_test_suite.h>
#include <xray/core/engine.h>
#include <xray/fs_utils.h>

using xray::core::debug::error_mode;
using xray::core::debug::bugtrap_usage;

static bool	s_initialized				= false;
static xray::core::engine * s_engine	=	NULL;

xray::command_line::key	s_log_verbosity ("log_verbosity", "", "logging", "one of: [trace|debug|info|warning|error|silent]");

namespace xray {
namespace core {
namespace configs {
	void initialize		( pcstr, pcstr );
	void finalize		();
} // namespace configs
} // namespace core

namespace threading {
	void preinitialize	();
	void finalize		();
	void initialize		();
} // namespace threading

} // namespace xray

void xray::core::preinitialize		( core::engine *								engine, 
									  pcstr const									command_line, 
									  command_line::contains_application_bool const	command_line_contains_application, 
									  pcstr	const									application,
									  pcstr	const									build_date
									)
{
	s_engine				= engine;
	R_ASSERT				( !s_initialized, "you cannot preinitialize core when it has been initialized already" );

	using namespace			debug;
	command_line::initialize( engine, command_line, command_line_contains_application );

	debug::initialize		( s_engine );

	set_application_name	( application );

	platform::preinitialize	( );
	threading::preinitialize( );
	memory::preinitialize	( );
	logging::preinitialize	( );
	build::preinitialize	( build_date );
}

bool xray::core::initialized ( )
{
	return					s_initialized;
}

static void   initialize_logging ( xray::core::log_file_usage const log_file_usage )
{
	using namespace xray;

	logging::verbosity	verbosity	=	logging::trace;
	fixed_string512		verbosity_string;
	bool const log_verbosity_key_is_set	=	s_log_verbosity.is_set_as_string(& verbosity_string);
	if ( log_verbosity_key_is_set )
		verbosity			= logging::string_to_verbosity(verbosity_string.c_str());
	else if ( testing::run_tests_command_line() && !xray::debug::is_debugger_present() )
		verbosity			= logging::warning;

	logging::initialize		( log_file_usage, verbosity );

	logging::verbosity const verbosity_for_resources	=	log_verbosity_key_is_set ? verbosity : logging::warning;
 	//logging::push_rule		( "core:resources:allocator", verbosity_for_resources );
 	logging::push_rule		( "core:resources:test", verbosity_for_resources );
    logging::push_rule		( "core:resources:device_manager", verbosity_for_resources );
    logging::push_rule		( "core:resources:detail", verbosity_for_resources );
}

void xray::core::initialize			(
		pcstr const debug_thread_id,
		log_file_usage const log_file_usage,
		debug_initialization const debug_initialization
	)
{
	R_ASSERT				( !s_initialized, "you cannot initialize core when it has been initialized already" );

	if ( debug_initialization == perform_debug_initialization)
		debug::postinitialize	( );
	
	// for language-dependent strings
	setlocale				( LC_CTYPE, "" );

	threading::set_thread_name	( debug_thread_id, debug_thread_id );
	
	threading::initialize	( );
	
	initialize_logging		( log_file_usage );

	LOG_INFO				( "working directory: '%s'", current_directory() );
	LOG_INFO				( "resources directory: '%s'", s_engine->get_resource_path() );

#ifdef XRAY_STATIC_LIBRARIES
	command_line::check_keys( );
	command_line::handle_help_key( );
#endif // #ifndef XRAY_STATIC_LIBRARIES

	timing::initialize		( );
	build::initialize		( s_engine );
	
	strings::initialize		( );
	fs::initialize			( );
	core::configs::initialize ( s_engine->get_resource_path(), s_engine->get_underscore_G_path() );

	tasks::initialize		(	2 * threading::core_count(),	// tasks thread count
								64,								// user thread count
								1,								// minimum active task thread count
								tasks::execute_while_wait_for_children_true, 
								tasks::do_logging_false
							);
	threading::set_current_thread_affinity	( 0 );
	tasks::register_current_thread_as_core_user	( );

	resources::initialize	( );

	core_test_suite::singleton()->set_resource_path	( s_engine->get_resource_path() );

	fs::initialize_watcher	( ); // must be initialized after resources

	resources::mount_mounts_path	( s_engine->get_mounts_path( ) );

//	resources::initialize_game_resources_manager	( );

	testing::initialize		( s_engine );

	debug::notify_xbox_debugger ( "hello from core::initialize" );

	core_test_suite::singleton()->set_resource_path	( s_engine->get_resource_path() );
	core_test_suite::run_tests	( );

	s_initialized			= true;
}

void xray::core::finalize			( )
{
	R_ASSERT				( s_initialized, "core library hasn't been initialized" );

	tasks::finalize			( );
	testing::finalize		( );

//	resources::finilize_game_resources_manager	( );

	fs::finalize_watcher	( ); // must be finalized before resources
	resources::finalize		( );
	core::configs::finalize	( );
	fs::finalize			( );
	strings::finalize		( );
//	build::finalize			( );
//	timing::finalize		( );
	logging::finalize		( );
	threading::finalize		( );
	memory::finalize		( );
	debug::finalize			( );

	s_initialized			= false;

	if ( testing::run_tests_command_line( ) )
	{
		fixed_string512		message;
		message.assignf		("program exit code: %d", s_engine->get_exit_code());
		debug::notify_xbox_debugger	(message.c_str());
	}
}