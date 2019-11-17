////////////////////////////////////////////////////////////////////////////
//	Created		: 09.06.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "test_simple_shader_op.h"
#include "cpu_timer.h"
#include "gpu_timer.h"
#include "benchmark.h"
#include "xray/render/core/effect_descriptor.h"
#include "time.h"

namespace xray {
namespace graphics_benchmark {


using namespace xray::render_dx10;
using namespace xray::math;

static std::string current_test_name;

class effect_simple_shader: public effect
{
public:
	effect_simple_shader () : effect() {}
	~effect_simple_shader() {}

	enum techniques{ solid };

	virtual void compile( effect_compiler& c, const effect_compilation_options& options)
	{
		XRAY_UNREFERENCED_PARAMETER	( options );

		shader_defines_list	defines;
		make_defines(defines);

		std::string name = current_test_name + "simple_shader";

		c.begin_technique( /*solid*/)
			.begin_pass	 ( name.c_str(), name.c_str(), defines)
			.end_pass	 ()
		.end_technique   ();
	}

private:
};

bool test_simple_shader_op::initialize_shader(benchmark& benchmark, char const*, char const*)
{
	char buffer[1024];
	sprintf(buffer,"shader_%d_",benchmark.get_current_test_index());
	current_test_name = buffer;

	xray::render_dx10::resource_manager::ref().add_shader_code((current_test_name + "simple_shader.vs").c_str(),m_vs_code.c_str(),true);
	xray::render_dx10::resource_manager::ref().add_shader_code((current_test_name + "simple_shader.ps").c_str(),m_ps_code.c_str(),true);

	effect_simple_shader b;
	m_sh = effect_manager::ref().create_effect( &b);

	return true;
}

void test_simple_shader_op::finalize(benchmark&)
{
	
}

void test_simple_shader_op::execute	(benchmark& benchmark, u32, u32, gpu_timer& gtimer)
{
	m_sh->apply			( effect_simple_shader::solid);
	benchmark.renderer	( ).render_fullscreen_quad( gtimer);
}

static std::string file_to_string( char const* file_name )
{
	std::string result;
	FILE* file = fopen(file_name,"r");
	fseek(file,0,SEEK_END);
	size_t file_size = ftell(file);
	fseek(file,0,SEEK_SET);
	result.resize(file_size);
	fread(result.begin(),1,file_size,file);
	fclose(file);
	return result;
}

bool test_simple_ps_shader_op::initialize	( benchmark& benchmark, char const* test_name, std::map<std::string,std::string>& parameters, std::string& out_comments)	
{ 
	std::string shader_template_file;
	std::string shader_template_data =
		"#include \"common.h\"								\n"
		"float4 x, y, z, w;									\n"
		"float4 main( v2p_TL i) : SV_TARGET					\n"
		"{													\n"
		"	float4 result = 1;								\n"
		"	float X=sin(i.Color);							\n"
		"	float Y=cos(i.Color);							\n"
		"	float Z=sqrt(i.Color);							\n"
		"	float W=rsqrt(i.Color);							\n"
		"	%s;												\n"
		"	return result;									\n"
		"}													\n";
	
	if (parameters.find("shader_template_file")!=parameters.end())
	{
		shader_template_file = parameters["shader_template_file"];
		shader_template_data = file_to_string(shader_template_file.c_str());
	}
	else
	{
		printf("\ntest_simple_ps_shader_op::initialize: field 'shader_template_file' not found in test '%s'!\n",test_name);
		//return false;
	}
	if (parameters.find("code")!=parameters.end())
	{
		std::string param = parameters["code"];
		out_comments	  = param;

#if 0
		char const* vs_code = 
			"#include \"common.h\"								\n"
			"float4 x, y, z, w;									\n"
			"v2p_TL main ( v_TL i)								\n"
			"{													\n"
			"	v2p_TL result;									\n"
			"	result.HPos = float4(i.P.xyz,1);				\n"
			"	result.Color = i.Color;							\n"
			"	result.Tex0 = i.Tex0;							\n"
			"	return result;									\n"
			"}													\n";
		char const* ps_code =
			"#include \"common.h\"								\n"
			"float4 x, y, z, w;									\n"
			"float4 main( v2p_TL i) : SV_TARGET					\n"
			"{													\n"
			"	float4 result = 1;								\n"
			"	float X=sin(i.Color);							\n"
			"	float Y=cos(i.Color);							\n"
			"	float Z=sqrt(i.Color);							\n"
			"	float W=rsqrt(i.Color);							\n"
			"	%s;												\n"
			"	return result;									\n"
			"}													\n";
#else
		char const* vs_code = 
			"#include \"common.h\"								\n"
			"float4 x, y, z, w;									\n"
			"v2p_TL main ( v_TL i)								\n"
			"{													\n"
			"	v2p_TL result;									\n"
			"	result.HPos = float4(i.P.xyz,1);				\n"
			"	result.Color = i.Color;							\n"
			"	result.Tex0 = i.Tex0;							\n"
			"	return result;									\n"
			"}													\n";
		char const* ps_code = shader_template_data.c_str();
#endif
		char new_ps_code[1024];
		::sprintf(new_ps_code,ps_code,param.c_str());

		m_vs_code = vs_code;
		m_ps_code = new_ps_code;

		initialize_shader(benchmark,test_name,param.c_str());
	}
	return true;
}

bool test_simple_vs_shader_op::initialize	( benchmark& benchmark, char const* test_name, std::map<std::string,std::string>& parameters, std::string& out_comments)	
{ 
	std::string shader_template_file;
	std::string shader_template_data = 
		"#include \"common.h\"								\n"
		"float4 x, y, z, w;									\n"
		"v2p_TL main ( v_TL i)								\n"
		"{													\n"
		"	v2p_TL result;									\n"
		"	%s												\n"
		"	return result;									\n"
		"}													\n";
	
	if (parameters.find("shader_template_file")!=parameters.end())
	{
		shader_template_file = parameters["shader_template_file"];
		shader_template_data = file_to_string(shader_template_file.c_str());
	}
	else
	{
		printf("\ntest_simple_vs_shader_op::initialize: field 'shader_template_file' not found in test '%s'!\n",test_name);
		//return false;
	}
	if (parameters.find("code")!=parameters.end())
	{
		std::string param = parameters["code"];
		out_comments	  = param;
		
#if 0
		char const* vs_code = 
			"#include \"common.h\"								\n"
			"float4 x, y, z, w;									\n"
			"v2p_TL main ( v_TL i)								\n"
			"{													\n"
			"	v2p_TL result;									\n"
			"	%s												\n"
			"	return result;									\n"
			"}													\n";
		char const* ps_code =
			"#include \"common.h\"								\n"
			"float4 x, y, z, w;									\n"
			"float4 main( v2p_TL i) : SV_TARGET					\n"
			"{													\n"
			"	return float4(1,1,1,1);							\n"
			"}													\n";
#else
		char const* vs_code = shader_template_data.c_str();
		
		char const* ps_code =
			"#include \"common.h\"								\n"
			"float4 x, y, z, w;									\n"
			"float4 main( v2p_TL i) : SV_TARGET					\n"
			"{													\n"
			"	return float4(1,1,1,1);							\n"
			"}													\n";
#endif
		
		char new_vs_code[1024];
		::sprintf(new_vs_code,vs_code,param.c_str());
		
		m_vs_code = new_vs_code;
		m_ps_code = ps_code;
		
		initialize_shader(benchmark,test_name,param.c_str());
	}
	return true;
}


} // namespace graphics_benchmark
} // namespace xray