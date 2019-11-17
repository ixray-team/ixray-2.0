////////////////////////////////////////////////////////////////////////////
//	Created		: 09.06.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "graphics_benchmark_application.h"
#include "graphics_benchmark_memory.h"
#include <xray/core/core.h>
#include <xray/core/simple_engine.h>
#include <xray/os_include.h>		// for GetCommandLine

#include <conio.h>

#include "benchmark.h"
#include "test_simple_shader_op.h"
#include "test_clear_render_target_depth_stencil.h"
#include "test_texture_shader_reading.h"
#include "test_3d_shader_op.h"

using namespace xray::graphics_benchmark;

using xray::graphics_benchmark::application;

xray::memory::doug_lea_allocator_type	xray::graphics_benchmark::g_allocator;
xray::render::render_allocator_type*	xray::render::g_allocator = &xray::graphics_benchmark::g_allocator;

class graphics_benchmark_core_engine : public xray::core::simple_engine
{
public:
	virtual	pcstr	get_mounts_path		( ) const	{ return "../../mounts"; }
};

static xray::uninitialized_reference< graphics_benchmark_core_engine >	s_core_engine;

namespace xray {
namespace graphics_benchmark {

static void on_mounted_disk				( bool const result )
{
	XRAY_UNREFERENCED_PARAMETER			( result );
	R_ASSERT							( result );
}

void application::initialize( )
{
	m_exit_code				= 0;

	XRAY_CONSTRUCT_REFERENCE( s_core_engine, graphics_benchmark_core_engine );

	core::preinitialize		(
		&*s_core_engine,
		GetCommandLine(), 
		command_line::contains_application_true,
		"graphics_benchmark",
		__DATE__ 
	);

	g_allocator.do_register	( 4*1024*1024, "graphics_benchmark" );

	memory::allocate_region	( );

	core::initialize		( "graphics_benchmark", core::create_log, core::perform_debug_initialization );

	resources::query_mount	( "resources", on_mounted_disk, &g_allocator );
}

void application::finalize	( )
{
	core::finalize			( );

	XRAY_DESTROY_REFERENCE	( s_core_engine );
}


void application::execute	( )
{
/*	std::string command_line = GetCommandLine(),
				benchmark_file_name,
				benchmark_log_file_name;

	// skip app name
	s32 q_pos  = command_line.find("\"");
		q_pos  = command_line.find("\"",q_pos+1);
		
		q_pos  = command_line.find("\"",q_pos+1);
	s32 q_pos2 = command_line.find("\"",q_pos+1);
	
	for ( s32 i=q_pos+1; i<q_pos2; i++)
		benchmark_file_name+=command_line[i];

		q_pos  = command_line.find("\"",q_pos2+1);
		q_pos2 = command_line.find("\"",q_pos+1);

	for ( s32 i=q_pos+1; i<q_pos2; i++)
		benchmark_log_file_name+=command_line[i];
*/
	{
		benchmark benchmark(target_nvidia, "D:/benchmark.txt");
		
		benchmark.register_test_class ( test_simple_vs_shader_op::creator, "test_simple_vs_shader_op" );
		benchmark.register_test_class ( test_simple_ps_shader_op::creator, "test_simple_ps_shader_op" );
		benchmark.register_test_class ( test_clear_render_target::creator, "test_clear_render_target" );
		benchmark.register_test_class ( test_clear_depth_stencil::creator, "test_clear_depth_stencil" );
		benchmark.register_test_class ( test_texture_shader_reading::creator, "test_texture_shader_reading" );
		benchmark.register_test_class ( test_3d_shader_op::creator, "test_3d_shader_op" );
		
		benchmark.start_execution();
	}
	_getch();
	
	m_exit_code				= s_core_engine->get_exit_code();
}

} // namespace graphics_benchmark
} // namespace xray