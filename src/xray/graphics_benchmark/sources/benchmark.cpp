////////////////////////////////////////////////////////////////////////////
//	Created		: 09.06.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "benchmark.h"
#include "gpu_timer.h"
#include "test_simple_shader_op.h"
#include <conio.h>


#define REGISTER_TEST_CLASS(type) \
	this->register_test_class(type::creator, #type);

namespace xray { 
namespace graphics_benchmark {


static pcstr s_window_id					= XRAY_ENGINE_ID " DX11 Renderer Window";
static pcstr s_window_class_id				= XRAY_ENGINE_ID " DX11 Renderer Window Class ID";
static WNDCLASSEX s_window_class;

static std::string const& base_test_name = "BASE";

static LRESULT APIENTRY message_processor	( HWND window_handle, UINT message_id, WPARAM w_param, LPARAM l_param )
{
	switch ( message_id ) {
	case WM_DESTROY: {
		PostQuitMessage	( 0 );

		return			( 0 );
					 }
	case WM_ACTIVATE: {

		while (	ShowCursor( TRUE ) < 0 );
		break;
					  }
	}

	return					( DefWindowProc( window_handle, message_id, w_param, l_param ) );
}

static void create_window			( HWND& result, u32 window_size_x, u32 window_size_y )
{
	WNDCLASSEX const temp	=
	{
		sizeof( WNDCLASSEX ),
		CS_CLASSDC,
		&message_processor,
		0L,
		0L, 
		GetModuleHandle( 0 ),
		NULL,
		NULL,
		NULL,
		NULL,
		s_window_class_id,
		NULL
	};

	s_window_class			= temp;
	RegisterClassEx			( &s_window_class );

	u32 const screen_size_x	= GetSystemMetrics( SM_CXSCREEN );
	u32 const screen_size_y	= GetSystemMetrics( SM_CYSCREEN );

	DWORD const	window_style = WS_POPUP;//WS_OVERLAPPEDWINDOW;//WS_CAPTION;

	RECT		window_size = { 0, 0, window_size_x, window_size_y };
	AdjustWindowRect		(&window_size, window_style, false);

	result					= 
		CreateWindow (
		s_window_class_id,
		s_window_id,
		window_style,
		( screen_size_x - window_size.right ) / 2,
		( screen_size_y - window_size.bottom ) / 2,
		window_size.right - window_size.left,
		window_size.bottom - window_size.top,
		GetDesktopWindow( ),
		0,
		s_window_class.hInstance,
		0
		);
}

HWND benchmark::get_new_window(u32 window_size_x, u32 window_size_y)
{
	HWND h_wnd;

	create_window(h_wnd,window_size_x,window_size_y);
	::ShowWindow(h_wnd,SW_SHOW);
	
	m_hwnd = h_wnd;

	return h_wnd;
}

bool benchmark::initialize_d3d()
{
	return true;
}

void benchmark::finalize_d3d()
{
	//need to delete device, how?
	//m_device;
}

ID3DDevice* benchmark::d3d_device() const
{
	return m_device.d3d_device();
}

static void shader_compile_error_handler( xray::render_dx10::enum_shader_type /*shader_type*/, char const* /*name*/, char const* error_string)
{
	printf("\n%s\n",error_string);
}

benchmark::benchmark(benchmark_target_enum bt, char const* benchmark_file):
	m_benchmark_loaded  ( execute_benchmark_file(benchmark_file)),
	m_device			( 0, get_new_window(m_window_size_x,m_window_size_y), !m_is_full_screen),
	m_counters_valid	( initialize(bt)),
	m_backend			( m_device.d3d_device())
{
	
	post_initialize(bt);
	
	xray::render_dx10::resource_manager::ref().set_compile_error_handler(shader_compile_error_handler);
	
	first_class				= NULL;
	
	m_current_test_index = 0;
}

benchmark::~benchmark()
{
	::ShowWindow(m_hwnd,SW_HIDE);
	
	pre_finalize();
	finalize_d3d();
	finalize();
	
	benchmark_class* next = NULL;
	for ( benchmark_class* it=first_class; it!=NULL; it = next)
	{
		next = it->next;
		XRAY_DELETE_IMPL(g_allocator,it);
	}
	
	::ShowWindow(m_hwnd,SW_HIDE);
	::ShowWindow(m_hwnd,SW_HIDE);
}

bool benchmark::initialize(benchmark_target_enum bt)
{
	m_benchmark_target = bt;

	if (bt==target_ati)
		return initialize_ati();

	if (bt==target_nvidia)
		return initialize_nvidia();

	if (bt==target_xbox)
		return initialize_xbox();

	if (bt==target_ps3)
		return initialize_ps3();

	return false;
}

bool benchmark::post_initialize(benchmark_target_enum bt)
{
	if (bt==target_ati)
	{
		//GPA_OpenContext( m_device.d3d_device() );
		//GPA_EnableAllCounters();
	}
	return true;
}

void benchmark::finalize()
{
	if (m_benchmark_target==target_ati)
		finalize_ati();

	if (m_benchmark_target==target_nvidia)
		finalize_nvidia();

	if (m_benchmark_target==target_xbox)
		finalize_xbox();

	if (m_benchmark_target==target_ps3)
		finalize_ps3();
}


bool benchmark::initialize_ati()
{
	//GPA_Initialize();
	return true;
}
/*
int MyEnumFunc(UINT unCounterIndex, char *pcCounterName)
{
	(void)&unCounterIndex;
	(void)&pcCounterName;

	char zString[200], zLine[400];
	unsigned int unLen;
	float fValue;
	(void)&fValue;

	unLen = 200;
	if(NVPMGetCounterDescription(unCounterIndex, zString, &unLen) == NVPM_OK) {
		sprintf(zLine, "Counter %d [%s] : ", unCounterIndex, zString);

		//unLen = 200;
		if(NVPMGetCounterName(unCounterIndex, zString, &unLen) == NVPM_OK)
			strcat(zLine, zString); // Append the short name
		else
			strcat(zLine, "Error retrieving short name");

		NVPMGetCounterClockRate(zString, &fValue);
		sprintf(zString, " %.2f\n", fValue);
		strcat(zLine, zString);
		printf(zLine);
		//OutputDebugStringA(zLine);
	}

	return(NVPM_OK);
}
*/
bool benchmark::initialize_nvidia()
{
	/*NVPMRESULT res = NVPMInit();
	res = NVPMAddCounterByName("D3D driver time");
	res = NVPMAddCounterByName("gpu_idle");
	
	printf("\n%d\n",res);
	NVPMEnumCounters(MyEnumFunc);
	*/
	return true;
}

void benchmark::finalize_nvidia()
{
	//NVPMShutdown();
}

bool benchmark::initialize_xbox()
{
	return true;
}

bool benchmark::initialize_ps3()
{
	return true;
}

// call before d3d device closed
void benchmark::pre_finalize()
{
	//GPA_CloseContext();
}

void benchmark::finalize_ati()
{
	//GPA_Destroy();
}


void benchmark::finalize_xbox()
{

}

void benchmark::finalize_ps3()
{

}

void benchmark::register_test_class( creator_func_type creator_func, char const* class_name)
{
	for ( benchmark_class* it=first_class; it!=NULL; it = it->next)
	{
		if (it->name!=class_name)
			continue;

		it->creator = creator_func;
		return;
	}
	benchmark_class* new_class	= NEW(benchmark_class)();
	new_class->next				= first_class;
	first_class					= new_class;

	new_class->creator			= creator_func;
	new_class->name				= class_name;
}

std::string benchmark::target_to_string(benchmark_target_enum t)
{
	if (t==target_ati)		return "ATI";
	if (t==target_nvidia)	return "NVIDIA";
	if (t==target_xbox)		return "XBOX";
	if (t==target_ps3)		return "PS3";
	
	return "none";
}

void benchmark::begin_test(u32& num_passes)
{
	num_passes = 1;
	m_start_gpu_time = 0;

	if ( m_benchmark_target==target_nvidia)
	{
		//s32 i_num_passes;
		//NVPMBeginExperiment(&i_num_passes);

		//num_passes = (u32)i_num_passes;
		
		m_start_gpu_time = 0;//TODO: read current time from "D3D timer" counter
	}
	if ( m_benchmark_target==target_ati)
	{
		num_passes = 0;
		m_start_gpu_time = 0;
	}
	if ( m_benchmark_target==target_xbox)
	{
		num_passes = 0;
		m_start_gpu_time = 0;
	}
	if ( m_benchmark_target==target_ps3)
	{
		num_passes = 0;
		m_start_gpu_time = 0;
	}
}

void benchmark::end_test(counters_result& out_results)
{
	(void)&out_results;

	// use m_start_gpu_time to get elapsed time;
	
	if ( m_benchmark_target==target_nvidia) {}
		// fill out_results
	if ( m_benchmark_target==target_ati) {}
		// fill out_results
	if ( m_benchmark_target==target_xbox) {}
		// fill out_results
	if ( m_benchmark_target==target_ps3) {}
		// fill out_results
}

void benchmark::get_empty_frame_elapsed_time(double& out_sec, u64& out_ticks)
{
	gpu_timer gtimer;

	gtimer.start();

	xray::render_dx10::device::ref().begin_frame();
	//xray::render_dx10::backend::ref().clear_render_targets( math::color( 0.1f, 0.2f, 0.3f));
	// NONE
	xray::render_dx10::device::ref().end_frame();

	gtimer.stop();

	gtimer.get_elapsed_info(out_sec, out_ticks);
}

void benchmark::execute( group_to_execute& group, test_to_execute& test, gpu_timer& gtimer2)
{
	if ( !test.object )
		return;
	
	char const*						   test_name	= test.test_name.c_str();
	std::map<std::string,std::string>& parameters	= test.parameters;
	u32								   num_passes	= test.num_passes;
	
	test.object->initialize(*this,test_name,parameters,test.test_comments);
	
	benchmark_test* new_test = test.object;
	
	//xray::render_dx10::backend::ref().clear_render_targets( math::color( 1.0f, 0.125f, 0.3f));
	xray::render_dx10::backend::ref().clear_depth_stencil( D3D_CLEAR_DEPTH | D3D_CLEAR_STENCIL, 1, 0);
	
	double avrg		  = 0.0;
	u64	   avrg_ticks = 0;

	const u32 num_tests		 = group.num_circles;
	const u32 num_skip_tests = group.num_skipped_circles;

#if 1
	
	u64		elapsed_ticks;
	double	elapsed_time;
	

	xray::render_dx10::device::ref().begin_frame();
		new_test->execute(*this,1,1, gtimer2);
	xray::render_dx10::device::ref().end_frame();

	xray::render_dx10::device::ref().begin_frame();
	
	for ( u32 ps=0; ps < num_tests + num_skip_tests; ps++)
	{
		gtimer2.start();
		for (u32 pass_index=0; pass_index < num_passes; pass_index++) 
			new_test->execute(*this,pass_index+1,num_passes, gtimer2);
		gtimer2.stop();
		
		gtimer2.get_elapsed_info(elapsed_time,elapsed_ticks);
		
		if (ps<num_skip_tests)
			continue;
		
		avrg	   += elapsed_time;
		avrg_ticks += elapsed_ticks;
	}
	xray::render_dx10::device::ref().end_frame();
#else
	gpu_timer gtimer2;

	//int nNumPasses;
	//NVPMBeginExperiment(&nNumPasses);

	u64 value, cycle = 0;
	
	NVPMRESULT res;
	//while (!cycle)
	//{
		NVPMGetCounterValueByName("gpu_idle", 0, &value, &cycle);
	//}

	printf("\n%d",res);

	for (u32 pass_index=0; pass_index < num_passes; pass_index++) 
		new_test->execute(*this,pass_index+1,num_passes, gtimer2);

	//int nNumPasses;
	
	//for ( u32 ps=0; ps<nNumPasses; ps++)
	//{
	//	NVPMBeginPass(ps);

		for (u32 pass_index=0; pass_index < num_passes; pass_index++) 
			new_test->execute(*this,pass_index+1,num_passes, gtimer2);
		
		// Flush GPU.
		gtimer2.start(); gtimer2.stop(); gtimer2.get_elapsed_time();
		
	//	NVPMEndPass(ps);
	//}
	//NVPMEndExperiment();

	//u64 value2, cycle2;
	//NVPMGetCounterValueByName("D3D driver time", 0, &value2, &cycle2);

#endif

	avrg		/= num_tests?num_tests:1;
	avrg_ticks	/= num_tests?num_tests:1;
	
	elapsed_time	= avrg;
	elapsed_ticks	= avrg_ticks;
	
	if (num_passes && elapsed_time)
	{
		test.result.elapsed_time	= (elapsed_time*1000.0*1000.0*1000.0)/(double)(num_passes);
		test.result.fps				= 1.0f / ((float)elapsed_time/(float)(num_passes));
		test.result.elapsed_ticks	= elapsed_ticks/num_passes;
		test.result.pixels_per_tick = ( (double)elapsed_ticks / (double)num_passes ) / (double)( m_window_size_x * m_window_size_y );
	}
	else
	{
		test.result.elapsed_time	= 0;
		test.result.fps				= 0;
		test.result.elapsed_ticks	= 0;
		test.result.pixels_per_tick = 0;
	}

	test.result.triangles_count = 0; // get from nv/ati counters
	test.result.batch_count		= 0; // get from nv/ati counters

	test.object->finalize(*this);
	
	m_current_test_index++;
	
	m_results.insert(std::pair<std::string, counters_result>(test_name, test.result));
	
	//::system("cls");
}

void benchmark::on_load_benchmark_file		( resources::queries_result & in_result )
{
	m_groups.clear();
	
	if ( in_result.is_failed() )
	{
		printf("failed to load benchmark file!");
		return;
	}
	
	configs::lua_config_ptr config_ptr = static_cast_checked<configs::lua_config *>(in_result[0].get_unmanaged_resource().c_ptr());
	configs::lua_config_value const & config	=	config_ptr->get_root();
	
	configs::lua_config_value const & settings_config	=	config_ptr->get_root()["SETTINGS"];
	
	m_window_size_x = 800;
	m_window_size_y = 800;
	m_is_full_screen = false;

	std::string log_file_name = "benchmark.log";
	
	// Read settings.
	for ( configs::lua_config::const_iterator it_s = settings_config.begin(); it_s != settings_config.end(); ++it_s )
	{
		configs::lua_config_value const & next_table_value	=	*it_s;
		fixed_string<260> next_table_name					= it_s.key();

		if (next_table_value.get_type()==configs::t_integer)
		{
			if (next_table_name=="resolution_x")
				m_window_size_x = (u32)next_table_value;
			else
			if (next_table_name=="resolution_y")
				m_window_size_y = (u32)next_table_value;
		}
		else
		if (next_table_value.get_type()==configs::t_string && next_table_name=="log_file_name")
			log_file_name = (pcstr)next_table_value;
		else
		if (next_table_value.get_type()==configs::t_boolean && next_table_name=="windowed")
			m_is_full_screen = !(bool)next_table_value;
		
	}
	
	m_log_file_name = log_file_name;
	
	// TODO: How to change resolution?
	//xray::render_dx10::device::ref().(100,100,!m_is_full_screen,m_hwnd);
	
	// Groups.
	for ( configs::lua_config::const_iterator it_g = config.begin(); it_g != config.end(); ++it_g )
	{
		configs::lua_config_value const & next_table_group	=	*it_g;
		fixed_string<260> next_table_name_group = it_g.key();
		
		if (next_table_name_group=="SETTINGS")
			continue;
		
		group_to_execute group;
		
		group.group_name		 = next_table_name_group.c_str();
		group.num_circles		 = 10;
		group.num_skipped_circles = 2;
		
		std::string							default_test_class;
		u32									default_num_passes = 1;
		std::map<std::string, std::string>	default_parameters;
		
		
		// Get default values.
		for ( configs::lua_config_value::const_iterator	it_t = next_table_group.begin(); it_t != next_table_group.end(); ++it_t )
		{
			configs::lua_config_value const & group_field_value	=	*it_t;
			fixed_string<260> next_group_field_name = it_t.key();
			
			if ( group_field_value.get_type()==configs::t_integer)
			{
				if ( next_group_field_name=="num_passes")
					default_num_passes = (u32)group_field_value;

				else if ( next_group_field_name=="num_circles")
					group.num_circles = (u32)group_field_value;

				else if ( next_group_field_name=="num_skipped_circles")
					group.num_skipped_circles = (u32)group_field_value;
			}
			
			else if ( group_field_value.get_type()==configs::t_string && next_group_field_name=="class")
				default_test_class = (pcstr)group_field_value;
			
			else if ( group_field_value.get_type()==configs::t_string)
				default_parameters[next_group_field_name.c_str()] = (pcstr)(pcstr)group_field_value;
		}

		// Tests, group fields.
		for ( configs::lua_config_value::const_iterator	it_t = next_table_group.begin(); it_t != next_table_group.end(); ++it_t )
		{
			configs::lua_config_value const & next_group_field	=	*it_t;
			fixed_string<260> next_group_field_name = it_t.key();
			
			configs::lua_config_value const & group_field_value	=	*it_t;

			if ( group_field_value.get_type()!=configs::t_table_named)
				continue;

			std::string test_class_name					  = default_test_class;
			u32			test_num_passes					  = default_num_passes;
			std::map<std::string, std::string> parameters = default_parameters;

			// Test fields.
			for ( configs::lua_config_value::const_iterator	it_f = next_group_field.begin(); it_f !=	next_group_field.end(); ++it_f )
			{
				fixed_string<260> next_table_name_test = it_f.key();

				configs::lua_config_value const & field_value	=	*it_f;

				if ( field_value.get_type()==configs::t_integer && next_table_name_test=="num_passes")
				{
					test_num_passes = (u32)field_value;
					continue;
				}
				
				if ( field_value.get_type()==configs::t_string)
				{
					if ( next_table_name_test=="class")
						test_class_name							 = (pcstr)field_value;
					else 
						parameters[next_table_name_test.c_str()] = (pcstr)field_value;
				}
			}
			
			test_to_execute test;
			test.class_name		= test_class_name;
			test.test_name		= next_group_field_name.c_str();
			test.parameters		= parameters;
			test.num_passes		= test_num_passes;
			test.object			= NULL;
			
			group.tests.push_back( test);
		}
		m_groups.push_back( group);
	}
	
	
	// Sort groups and tests by name.
	std::sort(m_groups.begin(),m_groups.end());
	for (u32 i=0; i<m_groups.size(); i++)
		std::sort(m_groups[i].tests.begin(),m_groups[i].tests.end());
}

void benchmark::start_execution()
{
	FILE* output = fopen(m_log_file_name.c_str(),"a+");
	
	char buffer[2048];
	
	sprintf(buffer,"\n-------------------------------------------------------------------------------------\n");
	printf(buffer);
	fwrite(buffer,1,strlen(buffer),output);
	
	sprintf(buffer,"[%s, %s] Results for %S: (%dx%d)\n", 
		__DATE__, 
		__TIME__, 
		//target_to_string(m_benchmark_target).c_str(), 
		xray::render_dx10::device::ref().adapter_desc().Description, 
		m_window_size_x, 
		m_window_size_y
	);
	
	printf(buffer);
	fwrite(buffer,1,strlen(buffer),output);
	fflush(output);
	
	u32 max_name_len = 0;
	for ( u32 group_index=0; group_index<m_groups.size(); group_index++)
		for ( u32 test_index=0; test_index<m_groups[group_index].tests.size(); test_index++)
			if (max_name_len<m_groups[group_index].tests[test_index].test_name.length())
				max_name_len = m_groups[group_index].tests[test_index].test_name.length();
	
	// Global GPU timer.
	gpu_timer gtimer;
	
	for ( u32 group_index=0; group_index<m_groups.size(); group_index++)
	{
		m_groups[group_index].has_base_result = false;
		for ( u32 test_index=0; test_index<m_groups[group_index].tests.size(); test_index++)
		{
			test_to_execute& test = m_groups[group_index].tests[test_index];
			benchmark_class* bmk_class = get_benchmark_class(test.class_name.c_str());
			if (bmk_class) 
				test.object = bmk_class->creator();
		}
	}
	
	// Execute groups.
	for ( u32 group_index=0; group_index<m_groups.size(); group_index++)
	{
		std::string space_string;
		space_string.assign(max_name_len,' ');
		
		sprintf(buffer,"\n  %s (circles:%d, skipped circles:%d):\n\n          %sticks/pixel     ticks\n\n", 
			m_groups[group_index].group_name.c_str(),
			m_groups[group_index].num_circles,
			m_groups[group_index].num_skipped_circles,
			space_string.c_str()
		);
		printf(buffer);
		fwrite(buffer,1,strlen(buffer),output);
		
		
		fflush(output);
		
		//for ( u32 group_pass_index=0; group_pass_index<m_groups[group_index].num_passes; group_pass_index++)
			for ( u32 test_index=0; test_index<m_groups[group_index].tests.size(); test_index++)
			{
				test_to_execute& test = m_groups[group_index].tests[test_index];
				
				test.group_pass_index = 0;//group_pass_index;
				
				// Execute the test.
				execute(m_groups[group_index],test,gtimer);

				//_getch();

				std::string space_string;
				space_string.assign(max_name_len-test.test_name.length(),' ');
				
				counters_result res;
				res = test.result;
				
				if ( test.test_name==base_test_name)
					m_groups[group_index].base_result = test.result;
				
				sprintf(buffer,"    %d. %s: %s%s // %s\n", 
					test_index+1, 
					test.test_name.c_str(), 
					space_string.c_str(), 
					res.as_text(m_groups[group_index].base_result, false).c_str(),
					test.test_comments.c_str()
				);
				printf(buffer);
				fwrite(buffer,1,strlen(buffer),output);
				fflush(output);

			}
	}

	// Print compared results with base test.
	show_compared_results( );


	for ( u32 group_index=0; group_index<m_groups.size(); group_index++)
		for ( u32 test_index=0; test_index<m_groups[group_index].tests.size(); test_index++)
			if (  m_groups[group_index].tests[test_index].object )
				XRAY_DELETE_IMPL(g_allocator, m_groups[group_index].tests[test_index].object);

	fclose(output);
}

void benchmark::show_compared_results( )
{
	FILE* output = fopen(m_log_file_name.c_str(),"a+");
	
	char buffer[2048];
	
	u32 max_name_len = 0;
	for ( u32 group_index=0; group_index<m_groups.size(); group_index++)
		for ( u32 test_index=0; test_index<m_groups[group_index].tests.size(); test_index++)
			if (max_name_len<m_groups[group_index].tests[test_index].test_name.length())
				max_name_len = m_groups[group_index].tests[test_index].test_name.length();
	
	sprintf(buffer,"\n\n Compared with base:\n");
	printf(buffer);
	fwrite(buffer,1,strlen(buffer),output);
	fflush(output);
	
	for ( u32 group_index=0; group_index<m_groups.size(); group_index++)
	{
		sprintf(buffer,"\n  %s:\n\n", m_groups[group_index].group_name.c_str());
		printf(buffer);
		fwrite(buffer,1,strlen(buffer),output);
		fflush(output);
		
		for ( u32 test_index=0; test_index<m_groups[group_index].tests.size(); test_index++)
		{
			test_to_execute& test = m_groups[group_index].tests[test_index];
			
			if ( test.test_name==base_test_name)
				continue;
			
			std::string space_string;
			space_string.assign(max_name_len-test.test_name.length(),' ');
			
			counters_result res;
			res = test.result;
			
			sprintf(buffer,"  %3d. %s: %s%s // %s\n", 
				test_index+1, 
				test.test_name.c_str(), 
				space_string.c_str(), 
				res.as_text(m_groups[group_index].base_result, true).c_str(),
				test.test_comments.c_str()
				);
			
			printf(buffer);
			fwrite(buffer,1,strlen(buffer),output);
			fflush(output);
		}
	}

	fclose(output);
}

bool benchmark::execute_benchmark_file		( char const* file_name)
{
	//m_log_file_name = "benchmark.txt";
	
	std::string new_file_name = "@";
	new_file_name += file_name;
	
	for (u32 i=0; i<new_file_name.length(); i++)
	{
		if (new_file_name[i]=='\\')
			new_file_name[i]='/';
	}
	xray::resources::query_resource_and_wait(
		new_file_name.c_str(),
		xray::resources::config_lua_class,
		boost::bind( &benchmark::on_load_benchmark_file, this, _1 ),
		&g_allocator);
	xray::resources::wait_and_dispatch_callbacks(true);

	return true;
}


benchmark_class* 
benchmark::get_benchmark_class( char const* class_name) const
{
	benchmark_class* bmk_class = NULL;
	for ( benchmark_class* it=first_class; it!=NULL; it = it->next)
	{
		if (!it->creator)
		{
			printf("benchmark::execute(): one of class creators equal to 0!");
			continue;
		}
		if (it->name==class_name)
		{
			bmk_class = it;
			break;
		}
	}
	return bmk_class;
}

} // namespace graphics_benchmark
} // namespace xray