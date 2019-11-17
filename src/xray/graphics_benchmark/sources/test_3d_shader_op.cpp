////////////////////////////////////////////////////////////////////////////
//	Created		: 17.06.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "test_3d_shader_op.h"
#include "benchmark.h"
#include "benchmark_renderer.h"
//#include "xray/render/engine/visual.h"
//#include "xray/render/engine/model_manager.h"

namespace xray {
namespace graphics_benchmark {


using namespace xray::render_dx10;
using namespace xray::math;


static std::string current_test_name;

class effect_3d_shader: public effect
{
public:
	effect_3d_shader () : effect() {}
	~effect_3d_shader() {}

	enum techniques{ solid };

	virtual void compile( effect_compiler& c, const effect_compilation_options& options)
	{
		XRAY_UNREFERENCED_PARAMETER	( options );

		shader_defines_list	defines;
		make_defines(defines);

		std::string name = current_test_name + "3d_shader";

		c.begin_technique( /*solid*/)
			.begin_pass	 ( name.c_str(), name.c_str(), defines)
			.end_pass	 ()
		.end_technique   ();
	}

private:
};

void test_3d_shader_op::on_test_visual_loaded	( xray::resources::queries_result& data)
{
	//ASSERT							( !data.empty());

	//resources::pinned_ptr_const<u8>	ogf_data	( data[ 0 ].get_managed_resource());

	//memory::chunk_reader		chunk_reader( ogf_data.c_ptr(), ogf_data.size(), memory::chunk_reader::chunk_type_sequential);

	////u16 type					= model_manager::get_visual_type( chunk_reader);
	//simple_visual*			s_visual;
	//s_visual				= NEW(simple_visual)();////xray::render::engine::model_manager::create_instance( type);
	//s_visual->load			( chunk_reader);

	//effect_3d_shader tb;
	//s_visual->m_effect = effect_manager::ref().create_effect( &tb);
}



bool test_3d_shader_op::initialize_shader(benchmark& benchmark, char const*, char const* options)
{
	char const* vs_code = 
		"#include \"common.h\"								\n"
		"float4 x, y, z, w;									\n"
		"float4x4 m_WVP_sl;	\n"
		"v2p_TL main ( v_TL i)								\n"
		"{													\n"
		"	v2p_TL result;									\n"
		"	result.HPos = mul(m_WVP_sl, float4(i.P.xyz,1));	\n"
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
		"	%s;												\n"
		"	return result;									\n"
		"}													\n";

	char new_ps_code[2048];
	::sprintf(new_ps_code,ps_code,options);

	m_vs_code = vs_code;
	m_ps_code = new_ps_code;

	char buffer[1024];
	sprintf(buffer,"shader_%d_",benchmark.get_current_test_index());
	current_test_name = buffer;

	xray::render_dx10::resource_manager::ref().add_shader_code((current_test_name + "3d_shader.vs").c_str(),m_vs_code.c_str(),true);
	xray::render_dx10::resource_manager::ref().add_shader_code((current_test_name + "3d_shader.ps").c_str(),m_ps_code.c_str(),true);

	effect_3d_shader b;
	m_sh = effect_manager::ref().create_effect( &b);

	return true;
}

bool test_3d_shader_op::initialize	( benchmark& benchmark, char const* test_name, std::map<std::string,std::string>& parameters, std::string&)	
{ 
	m_mesh_index = 0;

	if (parameters.find("code")!=parameters.end())
	{
		std::string param = parameters["code"];
		initialize_shader(benchmark,test_name,param.c_str());
	}
	
	if (parameters.find("mesh")!=parameters.end())
	{
		std::string mesh_file_name = parameters["mesh"];
		//TODO: load vertices, cook mesh, set wvp matrix
		
		//resources::request	resources[]	= {
		//	{ "@D:/mesh.ogf", xray::resources::raw_data_class },
		//};
		//
		//xray::resources::query_resources	( 
		//	resources,
		//	array_size( resources),
		//	boost::bind(  &test_3d_shader_op::on_test_visual_loaded, this, _1),
		//	render::g_allocator
		//	);

		//xray::math::float4x4 wvp_matrix = xray::math::float4x4().identity();
		
		//benchmark_renderer::colored_vertices_type vertices;
		//benchmark_renderer::colored_indices_type  indices;
		//
		//m_mesh_index = benchmark.renderer().cook_mesh(vertices, indices, wvp_matrix);
	}

	u32 wnd_size_x, wnd_size_y;

	benchmark.get_window_resolution(wnd_size_x, wnd_size_y);

	m_sphere_matrix = xray::math::mul4x3(
		create_camera_at(float3(2.0f,2.0f,2),float3(0.0f,0.0f,0.0f),float3(0.0f,1.0f,0.0f)),
		create_projection(math::pi_d4,(float)wnd_size_y/(float)wnd_size_x,0.01f,100.0f)
	);

	return true;
}

void test_3d_shader_op::finalize(benchmark&)
{
	
}

void test_3d_shader_op::execute	(benchmark& benchmark, u32, u32, gpu_timer& gtimer)
{
	m_sh->apply			( effect_3d_shader::solid);
	
	benchmark.renderer().render_3d_sphere( gtimer, m_sphere_matrix );
}


} // namespace graphics_benchmark
} // namespace xray