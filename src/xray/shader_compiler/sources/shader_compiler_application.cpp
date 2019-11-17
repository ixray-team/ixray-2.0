////////////////////////////////////////////////////////////////////////////
//	Created		: 18.05.2010
//	Author		: Armen Abroyan, Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "shader_compiler_application.h"
#include "shader_compiler_memory.h"
#include <xray/core/core.h>
#include <xray/core/simple_engine.h>
#include <xray/os_include.h>		// for GetCommandLine
#include <xray/configs_lua_config.h>

#include "shader_compiler.h"

#include "define_manager.h"
#include "resource_parser.h"
#include "file_batcher.h"
#include "utility_functions.h"

#include <xray/configs.h>
#include <string>


namespace xray {
namespace shader_compiler {

namespace colorize {
	//12	- red
	//14	- yellow
	//10	- green
	//7, 8  - gray
	//15	- white
	void set_text_color(unsigned short color)
	{
		HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hCon,color);
	}
	void set_base_color		()	{ set_text_color(7);  }
	void set_error_color	()	{ set_text_color(12); }
	void set_warning_color	()	{ set_text_color(14); }
	void set_sucesses_color	()	{ set_text_color(10); }

} // namespace colorize

using xray::shader_compiler::application;

xray::memory::doug_lea_allocator_type	xray::shader_compiler::g_allocator;

typedef xray::core::simple_engine							core_engine_type;
static xray::uninitialized_reference< core_engine_type >	s_core_engine;

using xray::configs::lua_config_ptr;
using xray::configs::lua_config_value;
using xray::configs::lua_config_iterator;

void application::initialize( )
{
	m_exit_code				= 0;

	XRAY_CONSTRUCT_REFERENCE( s_core_engine, core_engine_type );

	core::preinitialize		(
		&*s_core_engine,
		GetCommandLine(), 
		command_line::contains_application_true,
		"shader_compiler",
		__DATE__
	);

	g_allocator.do_register	( 4*1024*1024, "shader_compiler" );

	memory::allocate_region	( );

	core::initialize		( "shader_compiler", core::create_log, core::perform_debug_initialization );
}

void application::finalize	( )
{
	core::finalize			( );

	XRAY_DESTROY_REFERENCE	( s_core_engine );
}


void on_load( resources::queries_result & in_result);

void application::execute	( )
{
	//static bool switcher = true;
	//while( switcher)
	//{
	//	Sleep(200);
	//}

	std::string command_line = GetCommandLine();
	std::string entry_point = "main";
	
	files_batcher	f_batcher		(	 "ps,vs,gs,hs,cs"	);
	define_manager  define_manager(f_batcher.get_general_path().c_str());
	
	if ( !define_manager.get_valid())
	{
		return;
	}

	float	total_compile_time		= 0.0f;
	u32		total_failed_shaders	= 0;
	u32		total_with_warnings		= 0;
	u32		total_success_shaders	= 0;
	float	total_shaders_size		= 0.0f;
	u32		compiled_shader_index	= 0;
	bool show_define_name = strstr(GetCommandLine(),"-show_define_names") != 0;

	while ( f_batcher.next_file() )
	{
		bool legacy = false;
		
		std::string new_shader_path = f_batcher.get_file_name();
		std::string file_data = f_batcher.get_file_data();
		
		char* buffer	= file_data.begin();
		u32 file_size	= file_data.length();		
		
		bool is_debug = strstr(command_line.c_str(),"-debug_mode") != 0;
		
		printf("\n\n\n%d. Processing shader '%s'%s", compiled_shader_index+1, get_file_name(new_shader_path.c_str()).c_str(),is_debug?"  (debug)":"");
		
		u32 define_set_index = 0;

		while ( define_manager.next_defines_set() )
		{
			//printf("\n   -----------------------------------------------------------");

			u32 shader_size			= 0;
			u32 instruction_number	= 0;

			compiler comp(
				new_shader_path,
				buffer,
				file_size,
				"main",
				define_manager.get_defines_set()
				);
			
			std::vector<std::string> errors, 
									 warnings;
			
			printf("\n   %d.%d.( ",compiled_shader_index+1,define_manager.get_define_set_index());

			DWORD	start_time		= timeGetTime	( );
			bool	result			= comp.compile	( legacy, shader_size, instruction_number, errors, warnings );
			DWORD	compile_time	= timeGetTime	( ) - start_time;
			

			

			printf("   "); define_manager.print_set				( show_define_name); printf("  "); 
			printf("%s): ...",show_define_name?"   ":" ");
			
			if (result)
			{
				colorize::set_sucesses_color		( );
				printf("OK!\n");
			}
			else
			{
				colorize::set_error_color			( );
				printf								( "Failed\n" );
			}
			colorize::set_base_color				( );
			
			define_set_index++;
			
			total_compile_time+=compile_time/1000.0f;
			
			if ( warnings.size() || errors.size() )
				printf("\n");
			
			if ( warnings.size() )
			{
				colorize::set_warning_color();
				for (u32 index=0; index<warnings.size(); index++)
					printf("      %s",warnings[index].c_str());
			}

			if ( errors.size() )
			{
				colorize::set_error_color();
				for (u32 index=0; index<errors.size(); index++)
					printf("      %s",errors[index].c_str());
			}
			
			if ( warnings.size() )
				total_with_warnings++;
			
			if ( warnings.size() || errors.size() )
				printf("\n");

			colorize::set_base_color();
			
			total_shaders_size += (float)shader_size/1024.0f;

			bool show_size  = strstr(command_line.c_str(),"-size")!=0;
			bool show_time  = strstr(command_line.c_str(),"-time")!=0;
			bool show_ni	= strstr(command_line.c_str(),"-instruction_number")!=0;
			
			if ( show_size )
				printf("   shader size: %.2f KB", (float)shader_size/1024.0f);
			
			if ( show_time )
				printf("   time: %.3f sec.",(float)compile_time/1000.0f);
			
			if ( show_ni )
				printf("   instructions: %d ",instruction_number);
			
			if (show_size || show_time || show_ni)
				printf("\n");

			//printf("   -----------------------------------------------------------\n");
			
			colorize::set_base_color();
			
			total_failed_shaders+=(result?0:1);
			total_success_shaders+=(result?1:0);
		}
		printf("\n-----------------------------------------------------------");
		compiled_shader_index++;
	}

	printf("\n\n> Total configurations: %d, Success: %d, With errors: %d, With warnings: %d, Total shaders size: %.2f %s, Total time: %.2f %s",
		total_success_shaders+total_failed_shaders,
		total_success_shaders,
		total_failed_shaders,
		total_with_warnings,
		(total_shaders_size>=1024.0f) ? total_shaders_size/1024.0f : total_shaders_size,
		(total_shaders_size>=1024.0f) ? "MB" : "KB",
		(total_compile_time>=60.0f) ? ((total_compile_time>=3600.0f) ? total_compile_time/3600.0f : total_compile_time/60.0f) : total_compile_time,
		(total_compile_time>=60.0f) ? ((total_compile_time>=3600.0f) ? "hours" : "min.") : "sec."
		);
	
	
	_getch();
	
	m_exit_code				= s_core_engine->get_exit_code();
}

} // namespace shader_compiler
} // namespace xray