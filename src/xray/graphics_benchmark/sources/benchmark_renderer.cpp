////////////////////////////////////////////////////////////////////////////
//	Created		: 14.06.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "benchmark_renderer.h"
#include "gpu_timer.h"
#include "sphere_primitive.h"

namespace xray { 
namespace graphics_benchmark {

using namespace xray::render_dx10;
using namespace xray::math;

const D3D_INPUT_ELEMENT_DESC F_L[] = 
{
	{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	0, 12, 	D3D_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 16,	D3D_INPUT_PER_VERTEX_DATA, 0},
};

benchmark_renderer::benchmark_renderer()
{
	m_geom = resource_manager::ref().create_geometry( F_L, sizeof(colored_vertex), xray::render_dx10::backend::ref().vertex.buffer(), xray::render_dx10::backend::ref().index.buffer());
	
	colored_vertex v;
	
	colored_indices_type				q_indices;
	colored_vertices_type				q_vertices;
	
	v.position = float3(  -1.0f,  1.0f,  0.5f  ); v.color = color(1.0f,0.0f,0.0f,1.0f).get_d3dcolor(); v.uv = float2(0.f,0.f); q_vertices.push_back(v);
	v.position = float3(   1.0f, 1.0f,  0.5f   ); v.color = color(0.0f,1.0f,0.3f,1.0f).get_d3dcolor(); v.uv = float2(1.f,0.f); q_vertices.push_back(v);
	v.position = float3(  1.0f,  -1.0f,  0.5f  ); v.color = color(0.0f,0.0f,1.0f,1.0f).get_d3dcolor(); v.uv = float2(1.f,1.f); q_vertices.push_back(v);
	v.position = float3(  -1.0f,  -1.0f,  0.5f ); v.color = color(1.0f,1.0f,0.0f,1.0f).get_d3dcolor(); v.uv = float2(0.f,1.f); q_vertices.push_back(v);
	
	q_indices.push_back(0); q_indices.push_back(1); q_indices.push_back(2);
	q_indices.push_back(0); q_indices.push_back(2); q_indices.push_back(3);
	
	m_quad_mesh_idx = cook_mesh( q_vertices, q_indices );
	
	m_WVP = backend::ref().register_constant_host( "m_WVP_sl");
	
	colored_vertices_type s_vertices;
	colored_indices_type  s_indices;
	
	for (u32 i=0; i<SPHERE_NUMVERTEX; i++)
	{
		colored_vertex v;
			v.position  = sphere_vertices[i];
			v.color		= color(v.position.x,v.position.y,v.position.z,1.0f).get_d3dcolor();
			v.uv		= float2(v.position.x,v.position.y);
		s_vertices.push_back( v );
	}
	
	for (u32 i=0; i<SPHERE_NUMFACES*3; i++)
		s_indices.push_back( sphere_faces[i]);
	
	m_3d_sphere_mesh_idx = cook_mesh( s_vertices, s_indices );
}

benchmark_renderer::~benchmark_renderer()
{

}

u32 benchmark_renderer::cook_mesh(colored_vertices_type const& vertices, colored_indices_type  const& indices)
{
	mesh msh;
	
	// Prepare ill vertex buffer.
	colored_vertex* vbuffer = (colored_vertex*) xray::render_dx10::backend::ref().vertex.lock( vertices.size(), sizeof(colored_vertex), msh.voffset);
	memory::copy( vbuffer, vertices.size()*sizeof(colored_vertex), &vertices[0], vertices.size()*sizeof(colored_vertex));
	xray::render_dx10::backend::ref().vertex.unlock();
	
	// Prepare index buffer
	u16* ibuffer = (u16*) xray::render_dx10::backend::ref().index.lock( indices.size(), msh.ioffset);
	memory::copy( ibuffer, indices.size()*sizeof(u16), &indices[0], indices.size()*sizeof(u16));
	xray::render_dx10::backend::ref().index.unlock();
	
	msh.index_count = indices.size();
	
	m_meshes.push_back(msh);
	
	return m_meshes.size()-1;
}

void benchmark_renderer::render_cooked_mesh(u32 mesh_index, xray::math::float4x4 const& wvp_matrix)
{
	if (mesh_index >= m_meshes.size())
		return;
	
	m_geom->apply();
	backend::ref().set_constant( m_WVP, wvp_matrix);
	backend::ref().render_indexed( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_meshes[mesh_index].index_count, m_meshes[mesh_index].ioffset,m_meshes[mesh_index].voffset );
}


void benchmark_renderer::prepare_quad()
{
	//m_geom->apply();
	//backend::ref().set_constant( m_WVP, m_meshes[0].wvp_matrix);
	//backend::ref().flush();
	//backend::ref().update_input_layout();
	//device::ref().d3d_context()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	
}

void benchmark_renderer::render_fullscreen_quad(gpu_timer&)
{
	render_cooked_mesh( m_quad_mesh_idx );
}

void benchmark_renderer::render_3d_sphere(gpu_timer&, xray::math::float4x4 const& wvp_matrix)
{
	render_cooked_mesh( m_3d_sphere_mesh_idx, wvp_matrix );
}


} // namespace graphics_benchmark
} // namespace xray
