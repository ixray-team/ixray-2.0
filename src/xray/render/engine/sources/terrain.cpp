////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/terrain.h>
#include <xray/render/core/resource_manager.h>
#include <xray/render/core/effect_manager.h>
#include <xray/render/engine/model_manager.h>
#include <xray/render/engine/effect_terrain_NEW.h>
#include <xray/render/engine/visual.h>
#include <xray/fs_utils.h>


namespace xray {
namespace render_dx10 {

static void	fill_indexes	( uint2 size, u16* buffer);
static u32	fill_vertices	( void* vertex_buffer, u32 vertex_conut, u32 offset, u32 vertex_row_size, float phisical_size, float4x4 transform, render::terrain_data const* t_data, terrain_visual::Textures const& textures);

//struct terrain_cell_less_equal_pred 
//{
//	bool operator () ( terrain_cell const& first, u32 id)
//	{
// 		return first.user_id < id;
//	}
//};
//
//struct terrain_cell_equal_pred 
//{
//	terrain_cell_equal_pred	( int id): m_id(id){}
//
//	bool operator	() ( terrain_cell const& first)
//	{
//		return first.user_id == m_id;
//	}
//
//private:
//	u32		m_id;
//};


// D3DVERTEXELEMENT9	terrain_vertex_declaration[] =
// {
// 	{ 0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },	// pos+uv
// 	{ 0, 12, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_NORMAL,	0 },
// 	{ 0, 24, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_COLOR,		0 },
// 	{ 0, 28, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_COLOR,		1 },
// 	{ 0, 32, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	0 },
// 	{ 0, 40, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	1 },
// 	{ 0, 48, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	2 },
// 	{ 0, 56, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD/**/,	3 },
// 	D3DDECL_END()
// };	

const u32 terrain_vertex_size = 60;
D3D_INPUT_ELEMENT_DESC	terrain_vertex_declaration[] =
{
	{"POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12,	D3D_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR",	0, DXGI_FORMAT_R8G8B8A8_UNORM,	0, 24,	D3D_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR",	1, DXGI_FORMAT_R8G8B8A8_UNORM,	0, 28,	D3D_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",0, DXGI_FORMAT_R32G32_FLOAT,	0, 32, 	D3D_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",1, DXGI_FORMAT_R32G32_FLOAT,	0, 40, 	D3D_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",2, DXGI_FORMAT_R32G32_FLOAT,	0, 48, 	D3D_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",3, DXGI_FORMAT_R8G8B8A8_UNORM,	0, 56, 	D3D_INPUT_PER_VERTEX_DATA, 0}
};	



terrain::terrain():
m_texture_pool	( 1024, 4)
{
	m_intermediate_vertex_stream.create( 1024*1024 );
	
	m_effect	= NEW( effect_terrain_NEW);
}

terrain::~terrain()
{
	DELETE		( m_effect );
}

void terrain::add_cell( render::visual_ptr v, bool beditor )
{
	cells& working_set			= (beditor) ? m_editor_cells : m_game_cells;
	terrain_cell				cell;
	cell.visual					= v;

	cells::iterator it			= std::find( working_set.begin(), working_set.end(), v );
	ASSERT_U					( it == working_set.end() );
	working_set.push_back		( cell);
}

void terrain::remove_cell(render::visual_ptr v, bool beditor )
{
	cells& working_set			= (beditor) ? m_editor_cells : m_game_cells;
	cells::iterator it			= std::find( working_set.begin(), working_set.end(), v);
	working_set.erase				( it );
}

void terrain::update_cell_buffer( render::visual_ptr visual, xray::vectora<render::buffer_fragment_NEW> const& fragments, float4x4 const& transform)
{
	terrain_visual* v	= static_cast_checked<terrain_visual*>(visual.c_ptr());
	u8* pLockData		= 0;

	u32 vertex_size = terrain_vertex_size;//D3DXGetDeclVertexSize(terrain_vertex_declaration, 0);

	u32 lock_start	= fragments[0].start*vertex_size;
	u32 lock_size	= (fragments[fragments.size()-1].start+fragments[fragments.size()-1].size)*vertex_size;
		lock_size	-= lock_start;
	ASSERT( lock_size > 0 );

	// --Porting to DX10_
	//IDirect3DVertexBuffer9*		vb = model_manager::ref().get_vbuffer_by_id( v->vertex_buffer_id);
	//R_CHK				( vb->Lock( lock_start, lock_size, (void**)&pLockData, 0 ) );
	


	u32 voffset;
	void* vbuffer;

	vectora<render::buffer_fragment_NEW>::const_iterator	it_frag	= fragments.begin();
	vectora<render::buffer_fragment_NEW>::const_iterator	en_frag	= fragments.end();
		
	
	
	pLockData = (u8*)v->vb->map( D3D_MAP_READ);
	v->vb->unmap ();

	for( ; it_frag != en_frag; ++it_frag)
	{
		vbuffer = m_intermediate_vertex_stream.lock( it_frag->size, vertex_size, voffset);	
		fill_vertices( vbuffer, it_frag->size, it_frag->start, v->vertex_row_size, v->phisical_size, transform, &(it_frag->buffer[0]), v->textures);
		m_intermediate_vertex_stream.unlock();

		resource_manager::ref().copy(v->vb, u32(pLockData + it_frag->start*vertex_size), m_intermediate_vertex_stream.buffer(), voffset*vertex_size, it_frag->size*vertex_size);
	}
		

	//m_intermediate_vertex_stream
	/*
	pLockData = (u8*)v->vb->map( D3D_MAP_WRITE);

	vectora<render::buffer_fragment_NEW>::const_iterator	it_frag	= fragments.begin();
	vectora<render::buffer_fragment_NEW>::const_iterator	en_frag	= fragments.end();

	for( ; it_frag != en_frag; ++it_frag)
		fill_vertices( pLockData + it_frag->start*vertex_size, it_frag->size, it_frag->start, v->vertex_row_size, v->phisical_size, transform, &(it_frag->buffer[0]), v->textures);

	v->vb->unmap ();*/
}

void terrain::add_cell_texture 	( render::visual_ptr visual, render::texture_string const & texture, u32 tex_user_id)
{
	terrain_visual* v	= static_cast_checked<terrain_visual*>(visual.c_ptr());
	int tex_pool_id		= m_texture_pool.add_texture( texture);
	ASSERT				( tex_pool_id>=0);
	ASSERT				( std::find( v->textures.begin(), v->textures.end(), tex_pool_id) == v->textures.end());

	v->textures.resize( math::max( tex_user_id+1, v->textures.size()) , -1 );
	v->textures[tex_user_id] = tex_pool_id;
}

void terrain::remove_cell_texture( render::visual_ptr visual, u32 tex_user_id)
{
	terrain_visual* v	= static_cast_checked<terrain_visual*>(visual.c_ptr());

	int tex_pool_id		= -1;
	ASSERT( v->textures.size()> tex_user_id);
	tex_pool_id = v->textures[tex_user_id]; 

	// Mark as unused 
	ASSERT( tex_pool_id >=0);
	v->textures[tex_user_id] = -1;

	// Remove void tail
	int i = int(v->textures.size()-1);
	for( ; i >= 0; --i)
		if( v->textures[i] != -1)
			break;

	v->textures.resize(i+1);

	ASSERT( tex_pool_id>=0);
	if( tex_pool_id <0)
		return;

	// Remove texture from the pool if it isn't used any more by other cells.
	cells::iterator			it = m_editor_cells.begin();
	cells::const_iterator	en = m_editor_cells.end();

	for( ; it!= en; ++it)
		 if( v->textures.end() != std::find( v->textures.begin(), v->textures.end(), tex_pool_id))
			return;

	m_texture_pool.remove_texture( tex_pool_id);
}

void terrain::exchange_texture	( render::texture_string const & old_texture, render::texture_string const & new_texture)
{
	m_texture_pool.exchange_texture( old_texture, new_texture);
}

render::visual_ptr terrain::get_cell( pcstr cell_id)
{
	cells::const_iterator it		= m_editor_cells.begin();
	cells::const_iterator it_e		= m_editor_cells.end();
	
	// need optimization later
	for( ; it!= it_e; ++it)
	{
		terrain_visual* const v	= static_cast_checked<terrain_visual* const>(it->visual.c_ptr());
		if(v->name == cell_id)
			return it->visual;
	}
	
//.-------
	m_game_cells.begin();
	m_game_cells.end();
	
	// need optimization later
	for( ; it!= it_e; ++it)
	{
		terrain_visual* const v	= static_cast_checked<terrain_visual* const>(it->visual.c_ptr());
		if(v->name == cell_id)
			return it->visual;
	}
//.-------

	return 0;
}

bool terrain::has_cell( pcstr cell_id) const
{
	cells::const_iterator it		= m_editor_cells.begin();
	cells::const_iterator it_e		= m_editor_cells.end();
	
	// need optimization later
	for( ; it!= it_e; ++it)
	{
		terrain_visual* const v	= static_cast_checked<terrain_visual* const>(it->visual.c_ptr());
		if(v->name == cell_id)
			return true;
	}

//.-------
	it			= m_game_cells.begin();
	it_e		= m_game_cells.end();
	
	// need optimization later
	for( ; it!= it_e; ++it)
	{
		terrain_visual* const v	= static_cast_checked<terrain_visual* const>(it->visual.c_ptr());
		if(v->name == cell_id)
			return true;
	}
//.---------

	return false;
}

static u32 fill_vertices( void* vertex_buffer, u32 vertex_count, u32 offset, u32 vertex_row_size, float phisical_size, float4x4 transform, render::terrain_data const* t_data, terrain_visual::Textures const& textures)
{
	ASSERT( vertex_count <= vertex_row_size*vertex_row_size);

	float inv_row_size = 1.f/vertex_row_size;
// 	{ 0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },	// pos+uv
// 	{ 0, 12, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_NORMAL,	0 },
// 	{ 0, 24, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_COLOR,		0 },
// 	{ 0, 28, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_COLOR,		1 },
// 	{ 0, 32, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	0 },
// 	{ 0, 40, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	1 },
// 	{ 0, 48, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	2 },
// 	{ 0, 56, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD/**/,	3 },

	render::terrain_data const* src_ptr = t_data;
	char*				dst_ptr = (char*)vertex_buffer;
	for( u32 i = 0; i < vertex_count; ++i, ++src_ptr)
	{
		float z = (float)math::floor((i+offset)*inv_row_size);
		float x = (i+offset)-z*vertex_row_size;

		x*= phisical_size/(vertex_row_size-1);
		z*= -phisical_size/(vertex_row_size-1);

		// writing position 
		*(float3*) dst_ptr	= transform.transform_position( float3( x, src_ptr->height, z));
		dst_ptr+= sizeof(float3);

		// writing normal 
		*(float3*) dst_ptr	= float3( 0, 1, 0);
		dst_ptr+= sizeof(float3);

		
		u32 c0 = math::color_argb( 0, src_ptr->alpha0, src_ptr->alpha1, src_ptr->alpha2) ;
		*(u32*) dst_ptr = c0;
		dst_ptr+= sizeof(u32);

		u32			color_aaa	= src_ptr->color;
//		u8 const	tex_id2		= u8(textures[src_ptr->tex_id2]);

		// writing diffuse color
		//color_aaa = (color_aaa&0x00FFFFFF)|(tex_id2<<24);
		*(u32*) dst_ptr		= color_aaa;
		dst_ptr+= sizeof(u32);

		// Tex0 coordinates
		*(float2*) dst_ptr  = src_ptr->tex_coord0;
		dst_ptr+= sizeof(float2);

		// Tex1 coordinates
		*(float2*) dst_ptr  = src_ptr->tex_coord1;
		dst_ptr+= sizeof(float2);

		// Tex2 coordinates
		*(float2*) dst_ptr  = src_ptr->tex_coord2;
		dst_ptr+= sizeof(float2);

		// Writing texture indices
		u32 tex_indices = math::color_argb( 0, u8(textures[src_ptr->tex_id0]), u8(textures[src_ptr->tex_id1]), u8(textures[src_ptr->tex_id2])) ;
		*(u32*) dst_ptr		= tex_indices;
		dst_ptr+= sizeof(u32);
	}

	return vertex_count;
}

static void fill_indexes( uint2 size, u16* buffer)
{
	u32 ptr = 0;
	for( u32 i = 0; i< size.y-1; ++i)
	for( u32 j = 0; j< size.x-1; ++j)
	{
		buffer[ptr] = u16(i*size.x+j);
		ptr++;
		buffer[ptr] = u16(i*size.x+j+1);
		ptr++;
		buffer[ptr] = u16((i+1)*size.x+j);
		ptr++;

		buffer[ptr] = u16(i*size.x+j+1);
		ptr++;
		buffer[ptr] = u16((i+1)*size.x+j+1);
		ptr++;
		buffer[ptr] = u16((i+1)*size.x+j);
		ptr++;
	}
}


terrain_cell_cook::terrain_cell_cook( )
:super( resources::terrain_cell_class, reuse_true, threading::current_thread_id(), threading::current_thread_id() )
{}

void   terrain_cell_cook::create_resource	(resources::query_result_for_cook &	in_out_query, 
											 const_buffer				raw_file_data, 
											 mutable_buffer				in_out_unmanaged_resource_buffer)
{
	XRAY_UNREFERENCED_PARAMETER		(in_out_unmanaged_resource_buffer);

	fs::path_string				cell_name;
	pcstr req_path				= in_out_query.get_requested_path();
	xray::fs::file_name_with_no_extension_from_path	(&cell_name, req_path);

	memory::reader				F((pbyte)raw_file_data.c_ptr(), raw_file_data.size());
	render_dx10::terrain_visual* visual 		= static_cast_checked<terrain_visual*>(model_manager::ref().create_instance( mt_terrain_cell ));

	u32 vertex_row_size			= F.r_u32();
	float phisical_size			= F.r_float();
	u32 tex_count				= F.r_u32();

	visual->textures.resize		(tex_count);
	for(u32 tidx = 0; tidx<tex_count; ++tidx)  
	{
		pcstr texture_name		= F.r_string();
		int id_tex				= terrain::ref().m_texture_pool.add_texture(texture_name);
		visual->textures[tidx]	= id_tex;
	} 

	float4x4					transform;
	transform.identity			();
	transform.i.xyz()			= F.r_float3();
	transform.j.xyz()			= F.r_float3();
	transform.k.xyz()			= F.r_float3();
	transform.c.xyz()			= F.r_float3();

	u32	vertex_size				= terrain_vertex_size;// D3DXGetDeclVertexSize( terrain_vertex_declaration,0);

	visual->mesh()->vertex_count	= math::sqr(vertex_row_size);

	visual->mesh()->primitive_count	= 2*math::sqr(vertex_row_size-1);
	visual->mesh()->index_count		= visual->mesh()->primitive_count*3;

	void* tmp_buffer			= NEW_ARRAY( char, math::max( vertex_size*visual->mesh()->vertex_count, u32(visual->mesh()->index_count*sizeof(u16)) ));

	// Filling vertices here 
	fill_vertices( tmp_buffer, visual->mesh()->vertex_count, 0, vertex_row_size, phisical_size, transform, (render::terrain_data*)F.pointer(), visual->textures );

	// --porting to DX10
	// 	u32 buffer_usage			= D3DUSAGE_WRITEONLY;
	// 	visual->vertex_buffer_id	= model_manager::ref().create_vb	( visual->vertex_count*vertex_size, buffer_usage, tmp_buffer);

	ref_buffer vb				= resource_manager::ref().create_buffer ( visual->mesh()->vertex_count*vertex_size, tmp_buffer, enum_buffer_type_vertex, true);

	visual->vb					= &*vb;
	visual->mesh()->vertex_base	= 0;


	//Filling indices here
	fill_indexes( uint2(vertex_row_size,vertex_row_size), (u16*)tmp_buffer);

	ref_buffer ib				= resource_manager::ref().create_buffer ( visual->mesh()->index_count*sizeof(u16), tmp_buffer, enum_buffer_type_index, false);
	visual->mesh()->index_base	= 0;

	DELETE_ARRAY				(tmp_buffer);

	ref_declaration decl_ptr	= resource_manager::ref().create_declaration	( terrain_vertex_declaration);
	visual->mesh()->geom		= resource_manager::ref().create_geometry		( &*decl_ptr, terrain_vertex_size, &*vb, &*ib);
	visual->m_effect			= effect_manager::ref().create_effect			( terrain::ref().m_effect, 0, terrain::ref().m_texture_pool.get_pool_texture_name());

	visual->vertex_row_size		= vertex_row_size;
	visual->phisical_size		= phisical_size;
	visual->name				= cell_name;

	in_out_query.set_unmanaged_resource	(visual, resources::memory_type_non_cacheable_resource, sizeof(render_dx10::terrain_visual));
	in_out_query.finish_query	(result_success);
}

void terrain_cell_cook::destroy_resource (resources::unmanaged_resource* const res)
{
	render_visual * const visual		= dynamic_cast<render_visual*>(res);
	R_ASSERT							(visual);
	model_manager::destroy_instance		(visual);
}


void   terrain_cell_cook::create_resource_if_no_file(resources::query_result_for_cook &	in_out_query, 
													 mutable_buffer				in_out_unmanaged_resource_buffer)
{
	XRAY_UNREFERENCED_PARAMETER		(in_out_unmanaged_resource_buffer);

	pcstr cell_name					= in_out_query.get_requested_path();
	fs::path_string req_path;

	resources::user_data_variant* v	= in_out_query.user_data();
	if ( !v )
	{
		in_out_query.finish_query	(result_error);
		return;
	}

	pcstr project_name				= NULL;
	if ( !v->try_get(project_name) )
	{
		in_out_query.finish_query	(result_error);
		return;
	}

	req_path						= "resources/projects/";
	req_path						+= project_name;
	req_path						+= "/";
	req_path						+= cell_name;

	in_out_query.set_request_path	( req_path.c_str() );
	in_out_query.finish_query		(result_requery);
}

} // namespace render_dx10
} // namespace xray
