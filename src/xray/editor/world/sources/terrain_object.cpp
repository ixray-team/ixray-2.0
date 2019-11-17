////////////////////////////////////////////////////////////////////////////
//	Created		: 08.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_object.h"
#include "project.h"
#include "project_items.h"
#include "level_editor.h"
#include "editor_world.h"
#include "tool_terrain.h"
#include "collision_object.h"
#include <xray/editor/base/managed_delegate.h>
#include <xray/editor/world/engine.h>

namespace xray {
namespace editor {

terrain_node::terrain_node(tool_base^ tool, terrain_core^ core)
:super(tool),
m_core(core),
m_b_load_complete(true)
{
	cell_size				= 1.0f;
	size					= 64.0f;
	image_key				= "curve";
	draw_wireframe			= false;
	m_visual				= NEW (xray::render::visual_ptr)();
}

terrain_node::~terrain_node()
{
	DELETE					(m_visual);
}

void terrain_node::load_defaults()
{
	super::load_defaults	();
	cell_size				= 1.0f;
	size					= 64.0f;
	generate_mesh			();
}

void terrain_node::set_transform_internal(float4x4 const& transform)
{
	super::set_transform	(transform);
}

void terrain_node::set_visible(bool bvisible)
{
	ASSERT					(bvisible!=get_visible());

	super::set_visible		(bvisible);
	// add or remove from render chain
//.	get_editor_renderer().terrain_set_cell_visible( m_id, bvisible);
}

void terrain_node::set_visual(xray::render::visual_ptr v)
{
	ASSERT				(v);
	*m_visual			= v;
	get_editor_renderer().terrain_add_cell			(v);
}

void terrain_node::generate_mesh()
{
	ASSERT				(m_vertices.Count==0);

 	m_dimension			= int(size / cell_size);
	// gen vertices
	float def_y			= 0.0f;
	
	// vertex index [0] is left-top corner
	for(int cy=0; cy<m_dimension+1; ++cy)
		for(int cx=0; cx<m_dimension+1; ++cx)
		{
			terrain_vertex		v;
			v.height			= def_y;
			v.t0_infl			= 255;
			v.t1_infl			= 0;//85;
			v.t2_infl			= 0;//85;
			v.t0_index			= 0;
			v.t1_index			= 1;
			v.t2_index			= 2;

			v.vertex_color		= xray::math::color_argb(255, 128, 128, 128);

			m_vertices.Add		(v);
		}

	//map_uv						(0, cell_size/10.0f); // 2048x2048 - 10m
	//map_uv						(1, cell_size/10.0f); // 1024x1024 - 10m
	//map_uv						(2, cell_size/10.0f);  // 1024x1024 - 10m
}

void terrain_node::map_uv(u32 /*layer_id*/, float /*k*/)
{
	//ASSERT				(m_vertices);
	//ASSERT				(layer_id>=0 && layer_id<3);

	//for(int cy=0; cy<m_dimension.Height+1; ++cy)
	//	for(int cx=0; cx<m_dimension.Width+1; ++cx)
	//	{
	//		u32 idx				= (m_dimension.Width+1)*cy + cx;
	//		terrain_vertex& v	= m_vertices[idx];
	//		v.uvset[layer_id].set(cx*k, cy*k);
	//	}
}

void terrain_node::add_used_texture(System::String^ texture)
{
	ASSERT(!m_used_textures.Contains(texture));

	m_used_textures.Add(texture);
	render::texture_string	t(unmanaged_string(texture).c_str());
	get_editor_renderer().terrain_add_cell_texture(*m_visual, t, m_used_textures.IndexOf(texture));
}

void terrain_node::initialize_collision()
{
	if( m_collision.initialized() )
		m_collision.destroy	();
	
	ASSERT(!m_collision.initialized());

	vector<float3>	vertices;
	u32				vertex_count;
	vectora<u32>	indices(g_allocator);
	u32				index_count;
	vector<u32>		triangle_data;
	u32				triangle_data_count;

//vertices
	vertex_count	= m_vertices.Count;
	for(u32 idx=0; idx<vertex_count; ++idx)
	{
		float3						p = position((u16)idx);
		vertices.push_back			(p);
	}
//indices & data

	terrain_quad						quad;
	u16 quad_count						= u16(m_dimension * m_dimension);
	index_count							= quad_count * 2 * 3; //2 triangle per quad, tri list
	indices.resize						(index_count);

	u32 iindex							= 0;
	for(u16 quad_index=0; quad_index<quad_count; ++quad_index)
	{
		triangle_data.push_back			(quad_index);
		triangle_data.push_back			(quad_index);

		bool res = get_quad				(quad, quad_index);
		ASSERT_U						(res);
		quad.export_tris				(indices, iindex, true);
	}

//--
	triangle_data_count			= quad_count*2;

	m_collision.create_mesh		(	this, 
									&vertices[0], 
									vertex_count, 
									&indices[0], 
									index_count, 
									&triangle_data[0], 
									triangle_data_count,
									collision_type_dynamic | collision_type_terrain);
	m_collision.insert			();
	m_collision.set_matrix		(m_transform);
}

void terrain_node::load(xray::configs::lua_config_value const& t)
{
	super::load			(t);
	cell_size			= t["cell_size"];
	size				= t["size"];
	generate_mesh		();

	if(!t["textures"].empty())
	{
		m_used_textures.Clear	();
		xray::configs::lua_config_value::iterator it	= t["textures"].begin();
		xray::configs::lua_config_value::iterator it_e	= t["textures"].end();

		for(; it!=it_e; ++it)
			m_used_textures.Add( gcnew System::String(*it));
	}

	if(t.value_exists("filename"))
	{
		m_b_load_complete		= false;
		System::String^ filename = gcnew System::String(t["filename"]);

		System::String^ fn		= "resources/projects/";
		fn						+= m_owner_tool->get_level_editor()->get_project()->project_name();
		fn						+= "/";
		fn						+= filename;

		query_result_delegate* q = NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &terrain_node::on_vertices_loaded));

		resources::query_resource	(
			unmanaged_string(fn).c_str(),
			xray::resources::raw_data_class,
			boost::bind(&query_result_delegate::callback, q, _1),
			g_allocator
			);
	}
}

void terrain_node::on_vertices_loaded( xray::resources::queries_result& data )
{
	R_ASSERT(data.is_successful());

	resources::pinned_ptr_const<u8> pdata	(data[ 0 ].get_managed_resource( ));

	memory::reader				F( pdata.c_ptr(), pdata.size() );
	u32 vcount					= m_vertices.Count;

	u32 row_size				= F.r_u32();
	float phys_size				= F.r_float();
	u32 tex_count				= F.r_u32();
	XRAY_UNREFERENCED_PARAMETERS(row_size,phys_size,tex_count);
	ASSERT(m_used_textures.Count==0);
	m_used_textures.Clear		();
	for(u32 tex_idx=0; tex_idx<tex_count; ++tex_idx)
		m_used_textures.Add( gcnew System::String(F.r_string()));

	float4x4		tr;
	tr.identity		();
	tr.i.xyz()		= F.r_float3	();
	tr.j.xyz()		= F.r_float3	();
	tr.k.xyz()		= F.r_float3	();
	tr.c.xyz()		= F.r_float3	();

	xray::render::terrain_data d;
	for(u32 vidx=0; vidx<vcount; ++vidx)
	{
		ASSERT				(!F.eof());
		F.r					(&d, sizeof(d), sizeof(d));

		terrain_vertex		v;

		v.height			= d.height;
		v.t0_infl			= d.alpha0;
		v.t1_infl			= d.alpha1;
		v.t2_infl			= d.alpha2;
		v.t0_index			= d.tex_id0;
		v.t1_index			= d.tex_id1;
		v.t2_index			= d.tex_id2;
		v.vertex_color		= d.color;
	
		ASSERT(v.t0_index<m_used_textures.Count);
		ASSERT(v.t1_index<m_used_textures.Count);
		ASSERT(v.t2_index<m_used_textures.Count);

		m_vertices[vidx]	= v;
	}
	m_b_load_complete		= true;
}

void terrain_node::save(xray::configs::lua_config_value t)
{
	super::save			(t);
	t["cell_size"]		= cell_size;
	t["size"]			= size;

	if(m_owner_tool->get_level_editor()->get_project()->m_tmp_is_save_to_file)
	{
		System::String^	filename	= System::String::Format("terrain/{0}_{1}.terr", m_tmp_key.x, m_tmp_key.z);
		t["filename"]				= unmanaged_string(filename).c_str();

		xray::memory::writer		binary_writer(g_allocator);
		// row size
		binary_writer.w_u32			(m_dimension+1);
		
		// physical size
		binary_writer.w_float		(size);

		// used textures
		binary_writer.w_u32			(m_used_textures.Count);

		for each (System::String^ t in m_used_textures)
			binary_writer.w_stringZ (unmanaged_string(t).c_str());
		// used textures

		// transform
		float4x4	tr				= get_transform();
		binary_writer.w_fvector3	(tr.i.xyz());
		binary_writer.w_fvector3	(tr.j.xyz());
		binary_writer.w_fvector3	(tr.k.xyz());
		binary_writer.w_fvector3	(tr.c.xyz());
		// transform

		{
			vectora<render::terrain_data>	vvertices(g_allocator);
			export_vertices				(0, (u16)m_vertices.Count, vvertices);
			R_ASSERT					((u32)m_vertices.Count == vvertices.size());

			binary_writer.w				(&vvertices[0], vvertices.size()* sizeof(vvertices[0]));
		}
		fs::path_string resource_path	= m_owner_tool->get_level_editor()->get_editor_world().engine().get_resource_path();
		System::String^ path			= gcnew System::String(resource_path.c_str());
		path							+= "/projects/";
		path							+= m_owner_tool->get_level_editor()->get_project()->project_name();
		path							+= "/";
		path							+= filename;

		System::String^ file_name	= System::IO::Path::GetFullPath(path);
		file_name					= file_name->Replace("\\", "/");
		binary_writer.save_to		(unmanaged_string(file_name).c_str());
	}
}
	
} // namespaxe editor
} // namespaxe xray
