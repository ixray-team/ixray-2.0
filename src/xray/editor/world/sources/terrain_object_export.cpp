////////////////////////////////////////////////////////////////////////////
//	Created		: 29.01.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_object.h"
#include "terrain_object.h"
#include <d3d9types.h>

namespace xray {
namespace editor {

u32	color_argb	(u8 a, u8 r, u8 g, u8 b);

/*
void terrain_node::export_ogf(pbyte& dest_buffer, u32& size)
{
	xray::memory::writer			writer;
	writer.external_data	= true;
	export_ogf_impl			(writer);
	size					= writer.size();
	dest_buffer				= writer.pointer();
}

void terrain_node::export_ogf(System::String^ fname)
{
	xray::memory::writer		writer;
	export_ogf_impl		(writer);
	writer.save_to		(unmanaged_string(fname).c_str());
}


void terrain_node::export_ogf_impl(xray::memory::writer& writer)
{
	ogf_header		hdr;
	// fill hdr here !!!
	hdr.format_version = 4;
	hdr.bb.min = aabb().min;
	hdr.bb.max = aabb().max;
	hdr.type	= mt_normal;

	writer.open_chunk					(ogf_chunk_header);
	writer.w							(&hdr, sizeof(hdr));
	writer.close_chunk					();

	writer.open_chunk					(ogf_texture);
	unmanaged_string s0(m_textures[0]->ToString() + "," + m_textures[1]->ToString() + "," + m_textures[2]->ToString());
    writer.w_stringZ					(s0.c_str());
	
	writer.w_stringZ					(unmanaged_string(m_shader_name).c_str());
	writer.close_chunk					();

	writer.open_chunk					(ogf_chunk_vertices);

	u32									vertex_count;
	vectora<u16>						indices_16(g_allocator);
	vectora<u32>						indices_32(g_allocator);
	u32									index_count;

//vertices
	D3DVERTEXELEMENT9 vertex_declaration[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },	// pos+uv
		{ 0, 12, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_NORMAL,	0 },
		{ 0, 24, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_COLOR,		0 },
		{ 0, 28, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_COLOR,		1 },
		{ 0, 32, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	0 },
		{ 0, 40, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	1 },
		{ 0, 48, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	2 },
		D3DDECL_END()
	};	

	writer.w			(&vertex_declaration[0], sizeof(vertex_declaration));

	vertex_count		= m_vertices.Count;

	writer.w_u32		(vertex_count);

//	writer.w			(&m_vertices[0], sizeof(terrain_vertex)*vertex_count);
	float k = cell_size/10.0f; // for uv-mapping
	for(u16 vidx=0; vidx<vertex_count; ++vidx)
	{
		terrain_vertex% v		= m_vertices[vidx];

		float3 p				= position(vidx);
		writer.w_fvector3		(p);// position

		p.set					(0,1,0);
		writer.w_fvector3		(p);//normal
		
		u32 clr					= color_argb(0, v.t0_infl, v.t1_infl, v.t2_infl);
		writer.w_u32			(clr);
	
		clr						= v.vertex_color;
		writer.w_u32			(clr);

		int _x, _z;
		vertex_xz				(vidx, _x, _z);
		float2 uv				(_x*k, _z*k);

		writer.w_fvector2		(uv);
		writer.w_fvector2		(uv);
		writer.w_fvector2		(uv);
	}

	writer.close_chunk					();

//indices & data
	writer.open_chunk					(ogf_chunk_indices);

	terrain_quad						quad;
	u16 quad_count						= u16(m_dimension * m_dimension);
	index_count							= quad_count * 2 * 3; //2 triangle per quad, tri list
	indices_16.resize					(index_count);
	indices_32.resize					(index_count);

	u32 iindex_16						= 0;
	u32 iindex_32						= 0;
	for(u16 quad_index=0; quad_index<quad_count; ++quad_index)
	{
		bool res = get_quad				(quad, quad_index);
		ASSERT							(res);
		quad.export_tris				(indices_16, iindex_16, true);
		quad.export_tris				(indices_32, iindex_32, true);
	}
	writer.w_u32						(indices_16.size());
	writer.w							(&indices_16.front(), indices_16.size()*sizeof(indices_16[0]));
	writer.close_chunk					();
}
*/

void terrain_node::export_vertices(u16 start_idx, u16 count, vectora<render::terrain_data>& dest_buffer)
{
	dest_buffer.resize			(count);

	float k						= cell_size/10.0f; // for uv-mapping
	for(u16 vidx=start_idx; vidx<start_idx+count; ++vidx)
	{
		terrain_vertex% v		= m_vertices[vidx];
		
		render::terrain_data& data = dest_buffer[vidx-start_idx];

		data.height				= v.height;
		data.tex_id0			= v.t0_index;
		data.tex_id1			= v.t1_index;
		data.tex_id2			= v.t2_index;

		data.alpha0				= v.t0_infl;
		data.alpha1				= v.t1_infl;
		data.alpha2				= v.t2_infl;
		data.color				= v.vertex_color;

		int _x, _z;
		vertex_xz				(vidx, _x, _z);

		float2 uv				(_x*k, _z*k);
		data.tex_coord0			= uv;
		data.tex_coord1			= uv;
		data.tex_coord2			= uv;
	}
}

} // namespace editor
} // namespace xray