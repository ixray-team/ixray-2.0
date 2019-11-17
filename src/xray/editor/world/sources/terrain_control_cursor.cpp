////////////////////////////////////////////////////////////////////////////
//	Created		: 08.02.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_control_cursor.h"
#include "terrain_modifier_control.h"
#include "level_editor.h"
#include "terrain_object.h"
#include "tool_terrain.h"

#include "picker.h"
#include <xray/editor/base/managed_delegate.h>
#include <xray/editor/world/engine.h>
#include "collision_object_dynamic.h"
#include <xray/geometry_utils.h>
#include <xray/ogf.h>
#include <d3d9types.h>

namespace xray {
namespace editor {


terrain_control_cursor::terrain_control_cursor(terrain_modifier_control_base^ owner)
:m_owner(owner),
m_visual_inner_ring_id(10000),
m_visual_outer_ring_id(10001),
m_visual_center_point_id(10002),
m_visual_strength_id(10003),
m_shown(false),
m_pinned(false)
{
	m_inner_width		= 0.2f;
	m_outer_width		= 0.2f;
	m_center_width		= 0.2f;
	m_inner_radius		= 1.0f;
	m_outer_radius		= 1.0f;
	m_center_radius		= 0.0f;
	m_inner_segments	= 256; //60;
	m_outer_segments	= 256; //60;
	m_center_segments	= 8;
	m_inner_vertices	= NEW (vector<vert_struct>)();
	m_outer_vertices	= NEW (vector<vert_struct>)();
	m_center_vertices	= NEW (vector<vert_struct>)();
	m_strength_vertices	= NEW (vector<vert_struct>)();

	m_visual_inner_ring	= NEW(render::visual_ptr)();
	m_visual_outer_ring	= NEW(render::visual_ptr)();
	m_visual_center_point= NEW(render::visual_ptr)();
	m_visual_strength	= NEW(render::visual_ptr)();
	m_last_picked_position = NEW(float3)(0,0,0);
	generate_geometry				();
}

terrain_control_cursor::~terrain_control_cursor()
{
	if(m_shown)
		show(false);

	DELETE (m_visual_inner_ring);
	DELETE (m_visual_outer_ring);
	DELETE (m_visual_center_point);
	DELETE (m_visual_strength);
	DELETE (m_inner_vertices);
	DELETE (m_outer_vertices);
	DELETE (m_center_vertices);
	DELETE (m_strength_vertices);
	DELETE (m_last_picked_position);
}

void terrain_control_cursor::on_visual_ready(xray::resources::queries_result& data)
{
	R_ASSERT(!data.is_failed());

	*m_visual_inner_ring	= static_cast_checked<render::visual*>(data[0].get_unmanaged_resource().c_ptr());
	ASSERT(*m_visual_inner_ring);
	*m_visual_outer_ring	= static_cast_checked<render::visual*>(data[1].get_unmanaged_resource().c_ptr());
	ASSERT(*m_visual_outer_ring);
	*m_visual_center_point	= static_cast_checked<render::visual*>(data[2].get_unmanaged_resource().c_ptr());
	ASSERT(*m_visual_center_point);
	*m_visual_strength		= static_cast_checked<render::visual*>(data[3].get_unmanaged_resource().c_ptr());
	ASSERT(*m_visual_strength);

	for(u32 data_idx=0; data_idx<data.size(); ++data_idx)
	{
		const_buffer user_data_to_create	= data[data_idx].creation_data_from_user();
		pbyte data							= (pbyte)user_data_to_create.c_ptr();
		DELETE (data);
	}

	if(m_shown)
		show(true);
}

void terrain_control_cursor::show(bool show)
{
	m_shown				= show;
	xray::render::editor::renderer& r = m_owner->m_level_editor->get_editor_renderer();

	if(m_shown)
	{
		if(m_visual_inner_ring->c_ptr())
		{
			float4x4 m		= float4x4().identity();
			r.add_visual	( m_visual_inner_ring_id,	 *m_visual_inner_ring, m, false, true);
			r.add_visual	( m_visual_outer_ring_id,	*m_visual_outer_ring,	m, false, true);
			r.add_visual	( m_visual_center_point_id, *m_visual_center_point, m, false, true);
			r.add_visual	( m_visual_strength_id,		*m_visual_strength,		m, false, true);
		}
	}else
	{
		r.remove_visual		(m_visual_inner_ring_id);
		r.remove_visual		(m_visual_outer_ring_id);
		r.remove_visual		(m_visual_center_point_id);
		r.remove_visual		(m_visual_strength_id);
	}
}

void terrain_control_cursor::set_inner_raduis(float radius)
{
	m_inner_radius = radius;
}

void terrain_control_cursor::set_outer_raduis(float radius)
{
	m_outer_radius = radius;
}

void terrain_control_cursor::update()
{
	level_editor^ le		= m_owner->m_level_editor;

	System::Drawing::Point	mouse_pos;
	le->ide()->get_mouse_position(mouse_pos);
	if(m_last_mouse_pos==mouse_pos && !m_pinned)
		return;

	terrain_node^ terrain		= nullptr;
	collision_object const* picked_collision = NULL;
	
	if(le->get_picker( ).ray_query( m_owner->acceptable_collision_type(), &picked_collision, true, m_last_picked_position ) )
	{
		if(!m_shown)
			show		(true);

		ASSERT( picked_collision->get_type() & m_owner->acceptable_collision_type() );
	
		object_base^ o					= (static_cast_checked<collision_object_dynamic const*>(picked_collision))->get_owner();
		terrain							= safe_cast<terrain_node^>(o);
		update							(terrain->get_terrain_core(), *m_last_picked_position);
	}else
	{
		if(m_shown)
			show		(false);
	}

	if(terrain)
	{
	System::String^ props	= System::String::Format("Terrain[{0}:{1}]: {2:f2} {3:f2} {4:f2}", terrain->m_tmp_key.x, terrain->m_tmp_key.z,
							m_last_picked_position->x, m_last_picked_position->y, m_last_picked_position->z);

	le->ide()->set_status_label		(2, props);
	}

	m_last_mouse_pos				= mouse_pos;
}

void terrain_control_cursor::update(terrain_core^ terrain_core, float3 const& picked_position_global)
{
	if(!(*m_visual_inner_ring).c_ptr()) // not not loaded still
		return;

	xray::render::editor::renderer& r = m_owner->m_level_editor->get_editor_renderer();
	
	// tmp, for debug
	m_outer_radius					= m_owner->radius;
	m_inner_radius					= m_owner->radius * m_owner->hardness;


	typedef vectora<xray::render::buffer_fragment>	buffer_fragments;
	buffer_fragments										fragments(g_allocator);
	fragments.resize				(1);
	xray::render::buffer_fragment& fragment = fragments[0];
	fragment.start					= 0;

	// update inner
	fragment.size					= m_inner_vertices->size() * sizeof(vert_struct);
	fragment.buffer					= &((*m_inner_vertices)[0]);
	
	update_ring(terrain_core, picked_position_global, m_inner_radius, m_inner_width, m_inner_segments, *m_inner_vertices);
	r.update_visual_vertex_buffer	(*m_visual_inner_ring, fragments);

	// update outer
	fragment.size					= m_outer_vertices->size() * sizeof(vert_struct);
	fragment.buffer					= &((*m_outer_vertices)[0]);
	
	update_ring(terrain_core, picked_position_global, m_outer_radius, m_outer_width, m_outer_segments, *m_outer_vertices);
	r.update_visual_vertex_buffer	(*m_visual_outer_ring, fragments);

	// update center
	fragment.size					= m_center_vertices->size() * sizeof(vert_struct);
	fragment.buffer					= &((*m_center_vertices)[0]);
	
	update_ring(terrain_core, picked_position_global, m_center_radius, m_center_width, m_center_segments, *m_center_vertices);
	r.update_visual_vertex_buffer	(*m_visual_center_point, fragments);

	
	float3 p					= picked_position_global;
	float h						= terrain_core->get_height	(p);
	p.y							= h;

	float4x4 m					= create_scale(float3(0.5f, m_owner->strength_value(), 0.5f)) * create_translation(p);
	r.update_visual				(m_visual_strength_id, m, false);
}

void terrain_control_cursor::update_ring(	terrain_core^ terrain, 
											float3 const& picked_position_global,  
											float radius, 
											float width, 
											u16 segments_count, 
											vector<vert_struct>& vertices)
{
	float3 cam_p, cam_d;
	float3 dir_to_camera;
	m_owner->m_level_editor->get_camera_props(cam_p, cam_d);

	float height					= 0.0f;
	vector<vert_struct>::iterator	vit;
	vit								= vertices.begin();
	float segment_ang				= math::pi_x2/segments_count;

	float3 p;
	for(u32 i=0; i<segments_count; ++i)
	{
		math::sine_cosine			sincos(segment_ang*i);

		p.set						(radius*sincos.cosine, 0.0f, -radius*sincos.sine);
		p							+= picked_position_global;
		
		height						= terrain->get_height(p);
		p.y							= height;
		
		dir_to_camera				= cam_p - p;
		dir_to_camera.normalize		();
		dir_to_camera				*= 0.1f;

		(*vit).position				= p + dir_to_camera;
		++vit;

		p.set						((radius+width)*sincos.cosine, 0.0f, -(radius+width)*sincos.sine);
		p							+= picked_position_global;
		height						= terrain->get_height(p);
		p.y							= height;

		(*vit).position				= p + dir_to_camera;
		++vit;
	}
}

void export_ring(vector<vert_struct>& dest_vertices, pbyte& dest_buffer, u32& size, float radius, float width, u16 segments, pcstr tex_name, pcstr shader_name)
{
	xray::memory::writer	writer(g_allocator);
	writer.external_data	= true;
	ogf_header				hdr;

	// fill hdr here !!!
	hdr.format_version		= 4;
	hdr.bb.min				= float3(-1, -1, -1);
	hdr.bb.max				= float3(1, 1, 1);
	hdr.type				= mt_normal;

	geometry_utils::geom_vertices_type	vertices(g_allocator);
	geometry_utils::geom_indices_type	indices(g_allocator);
	geometry_utils::geom_vertices_type::const_iterator	it, it_e;
	vector<vert_struct>::iterator	vit;

	geometry_utils::create_ring		(vertices, indices, radius, width, segments );
	writer.open_chunk		(ogf_chunk_header);
	writer.w				(&hdr, sizeof(hdr));
	writer.close_chunk		();

	writer.open_chunk		(ogf_texture);
    writer.w_stringZ		(tex_name);
    writer.w_stringZ		(shader_name);
	writer.close_chunk		();

	writer.open_chunk		(ogf_chunk_vertices);

////vertices
	// Create vertex declaration
	D3DVERTEXELEMENT9 vertex_declaration[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },	// pos+uv
		{ 0, 12, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_NORMAL,	0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	0 },
		D3DDECL_END()
	};	
	writer.w			(&vertex_declaration[0], sizeof(vertex_declaration));


	dest_vertices.resize	(vertices.size());
	writer.w_u32			(vertices.size());

	it						= vertices.begin();
	it_e					= vertices.end();
	vit						= dest_vertices.begin();
	bool b_inner			= true;
	for(; it!=it_e; ++it,++vit)
	{
		vert_struct& v		= *vit;
		v.position			= (*it);
		v.normal			= float3(0, 1, 0);
		v.uv				= (b_inner) ? float2(0, 1) : float2(0, 0);

		b_inner				= !b_inner;
	}
	writer.w				(&dest_vertices.front(), dest_vertices.size()* sizeof(vert_struct));
	writer.close_chunk		();


	writer.open_chunk		(ogf_chunk_indices);
	writer.w_u32			(indices.size());
	writer.w				(&indices[0], indices.size()*sizeof(indices[0]));
	writer.close_chunk		();

	size					= writer.size();
	dest_buffer				= writer.pointer();
}

void export_cylinder(vector<vert_struct>& dest_vertices, pbyte& dest_buffer, u32& size, float3 dim, pcstr tex_name, pcstr shader_name)
{
	xray::memory::writer	writer(g_allocator);
	writer.external_data	= true;
	ogf_header				hdr;

	// fill hdr here !!!
	hdr.format_version		= 4;
	hdr.bb.min				= float3(-1, -1, -1);
	hdr.bb.max				= float3(1, 1, 1);
	hdr.type				= mt_normal;

	geometry_utils::geom_vertices_type	vertices(g_allocator);
	geometry_utils::geom_indices_type	indices(g_allocator);
	geometry_utils::geom_vertices_type::const_iterator	it, it_e;
	vector<vert_struct>::iterator	vit;

	geometry_utils::create_cylinder	(vertices, indices, float4x4().identity(), dim );

	writer.open_chunk		(ogf_chunk_header);
	writer.w				(&hdr, sizeof(hdr));
	writer.close_chunk		();

	writer.open_chunk		(ogf_texture);
    writer.w_stringZ		(tex_name);
    writer.w_stringZ		(shader_name);
	writer.close_chunk		();

	writer.open_chunk		(ogf_chunk_vertices);

////vertices
	// Create vertex declaration
	D3DVERTEXELEMENT9 vertex_declaration[] =
	{
		{ 0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },	// pos+uv
		{ 0, 12, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_NORMAL,	0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	0 },
		D3DDECL_END()
	};	
	writer.w			(&vertex_declaration[0], sizeof(vertex_declaration));


	dest_vertices.resize	(vertices.size());
	writer.w_u32			(vertices.size());

	it						= vertices.begin();
	it_e					= vertices.end();
	vit						= dest_vertices.begin();
	
	float3 position_offset	(0.0f, 1.0f, 0.0f);

	for(; it!=it_e; ++it,++vit)
	{
		vert_struct& v		= *vit;
		v.position			= (*it) + position_offset;
		
		v.normal			= float3(0, 1, 0);
		v.uv				= (v.position.y>0) ? float2(0, 1) : float2(0, 0);
	}
	writer.w				(&dest_vertices.front(), dest_vertices.size()* sizeof(vert_struct));
	writer.close_chunk		();


	writer.open_chunk		(ogf_chunk_indices);
	writer.w_u32			(indices.size());
	writer.w				(&indices[0], indices.size()*sizeof(indices[0]));
	writer.close_chunk		();

	size					= writer.size();
	dest_buffer				= writer.pointer();
}

void terrain_control_cursor::generate_geometry()
{
	pbyte inner_p;
	pbyte outer_p;
	pbyte center_p;
	pbyte strength_p;
	u32 inner_p_size;
	u32 outer_p_size;
	u32 center_p_size;
	u32 strength_p_size;

	export_ring(	*m_inner_vertices,
					inner_p, inner_p_size, 
					m_inner_radius, m_inner_width, m_inner_segments,
					"ui\\ui_cursor_border", "#forward_system");

	export_ring(	*m_outer_vertices,
					outer_p, outer_p_size, 
					m_outer_radius, m_outer_width, m_outer_segments,
					"ui\\ui_cursor_border", "#forward_system");

	export_ring(	*m_center_vertices,
					center_p, center_p_size, 
					m_center_radius, m_center_width, m_center_segments,
					"ui\\ui_cursor_border", "#forward_system");

	export_cylinder(*m_strength_vertices,
					strength_p, strength_p_size, 
					float3(0.07f, 1.0f, 0.07f), // same as center point
					"ui\\ui_cursor_border", "#forward_system");

	query_result_delegate* q		= NEW(query_result_delegate)(gcnew query_result_delegate::Delegate(this, &terrain_control_cursor::on_visual_ready));

	resources::creation_request requests[] =
	{
		{"", xray::memory::buffer(inner_p, inner_p_size), xray::resources::visual_class},
		{"", xray::memory::buffer(outer_p, outer_p_size), xray::resources::visual_class},
		{"", xray::memory::buffer(center_p, center_p_size), xray::resources::visual_class},
		{"", xray::memory::buffer(strength_p, strength_p_size), xray::resources::visual_class},
	};
	resources::query_create_resources(	requests,
										boost::bind(&query_result_delegate::callback, q, _1),
										g_allocator);
}

} // namespace editor
} // namespace xray