////////////////////////////////////////////////////////////////////////////
//	Created		: 17.05.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

//state_desc.set_stencil( true, 0x00/*ref_value*/, 0xFF, 0xFF, D3D_COMPARISON_LESS_EQUAL, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP );

#include "pch.h"
#include "../system_renderer.h"
#include "../effect_system_colored.h"
#include "../effect_system_ui.h"
#include "../effect_system_line.h"
#include "xray/render/engine/vertex_formats.h"
#include "xray/render/core/effect_manager.h"
#include "xray/render/core/backend.h"
#include "xray/render/core/resource_manager.h"
//#include "xray/render/common/sources/hw_wrapper.h">

namespace xray {
namespace render_dx10 {


	//TODO: not here
	const D3D_INPUT_ELEMENT_DESC F_L_sl[] = 
	{
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	0, 12, 	D3D_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16,	D3D_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	1, DXGI_FORMAT_R32_FLOAT,		0, 28,	D3D_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	2, DXGI_FORMAT_R32_UINT,		0, 32,	D3D_INPUT_PER_VERTEX_DATA, 0},
	};
	struct vertex_colored_sl
	{
		math::float3	position;
		u32				color;
		math::float3	begin_line_position;
		float			shift;
		u32				pattern;
	}; 


system_renderer::system_renderer( const render_dx10::scene_render& scene, res_buffer * quad_ib):
m_scene(scene),
m_grid_mode (false),
m_color_write ( true)
{
	m_vertex_stream.create( 1024*1024);
	m_index_stream.create( 512*1024);

	m_colored_geom = resource_manager::ref().create_geometry( vertex_formats::F_L, sizeof(vertex_formats::L), m_vertex_stream.buffer(), m_index_stream.buffer());

	m_colored_geom_sl = resource_manager::ref().create_geometry( F_L_sl, sizeof(vertex_colored_sl), m_vertex_stream.buffer(), m_index_stream.buffer());
	
	//state_descriptor state_desc;

	//state_desc.color_write_enable( D3D_COLOR_WRITE_ENABLE_NONE);
	//m_rotation_mode_states[0] = resource_manager::ref().create_state( state_desc);

	//state_desc.color_write_enable( D3D_COLOR_WRITE_ENABLE_ALL);
	//m_rotation_mode_states[1] = resource_manager::ref().create_state( state_desc);

	//m_rotation_mode_states[0] = resource_manager::ref().create_state( state_desc);

	//state_desc.set_depth( true, true);
	//state_desc.color_write_enable(D3D_COLOR_WRITE_ENABLE_NONE);

	//m_rotation_mode_states[1] = resource_manager::ref().create_state( state_desc);

	//state_desc.set_stencil( true, 0x00/*ref_value*/, 0xFF, 0xFF, D3D_COMPARISON_ALWAYS, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP );
	//state_desc.color_write_enable( D3D_COLOR_WRITE_ENABLE_NONE);

	//m_rotation_mode_states[2] = resource_manager::ref().create_state( state_desc);
	//m_rotation_mode_states[3] = m_rotation_mode_states[2];

	//state_desc.set_stencil( true, 0x00/*ref_value*/, 0xFF, 0xFF, D3D_COMPARISON_ALWAYS, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_DECR_SAT, D3D_STENCIL_OP_KEEP );
	//state_desc.color_write_enable( D3D_COLOR_WRITE_ENABLE_NONE);

	//m_rotation_mode_states[4] = resource_manager::ref().create_state( state_desc);

	//state_desc.set_stencil( true, 0x00/*ref_value*/, 0xFF, 0xFF, D3D_COMPARISON_EQUAL, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_KEEP );

	//state_desc.color_write_enable( D3D_COLOR_WRITE_ENABLE_ALL);
	//state_desc.set_depth(false, true);
	//m_rotation_mode_states[5] = resource_manager::ref().create_state( state_desc);

	effect_system_colored b;
	m_sh_vcolor = effect_manager::ref().create_effect( &b);

	effect_system_line sl;
	m_sh_sl = effect_manager::ref().create_effect( &sl);

	m_grid_density_constant = backend::ref().register_constant_host( "grid_density");

	m_ui_geom = resource_manager::ref().create_geometry( vertex_formats::F_TL, sizeof(vertex_formats::TL), m_vertex_stream.buffer(), quad_ib);

	effect_system_ui desc_ui;
	m_sh_ui = effect_manager::ref().create_effect( &desc_ui, NULL, "ui/ui_font_arial_21_1024,ui/ui_skull");

	m_WVP_sl = backend::ref().register_constant_host( "m_WVP_sl");
}

system_renderer::~system_renderer()
{

}

void system_renderer::draw_lines		( colored_vertices_type const& vertices, colored_indices_type  const& indices)
{
	// Prepare vertex buffer.
	render::vertex_colored* vbuffer;
	u32 voffset;
	m_vertex_stream.lock( vertices.size(), &vbuffer, voffset);
	memory::copy( vbuffer, vertices.size()*sizeof(render::vertex_colored), &vertices[0], vertices.size()*sizeof(render::vertex_colored));
	m_vertex_stream.unlock();

	// Prepare index buffer
	u32 ioffset;
	u16* ibuffer = (u16*) m_index_stream.lock( indices.size(), ioffset);
	memory::copy( ibuffer, indices.size()*sizeof(u16), &indices[0], indices.size()*sizeof(u16));
	m_index_stream.unlock();

	m_colored_geom->apply();
	m_sh_vcolor->apply();

	backend::ref().render_indexed( D3D_PRIMITIVE_TOPOLOGY_LINELIST, indices.size(), ioffset /* ? */, voffset );
}

static xray::math::float2 clip_2_screen( xray::math::float3 const& world_pixel, math::float4x4 const& wvpMatrix, u32 screen_width, u32 screen_height)
{
	xray::math::float4 result = wvpMatrix.transform(float4(world_pixel,1.0f));
	
	result = result * (1.0f / result.elements[3]);

	result.elements[0] *= 0.5f;
	result.elements[1] *= -0.5f;

	result = result + float4(0.5f,0.5f,0.0f,0.0f);

	result.elements[0] *= screen_width;
	result.elements[1] *= screen_height;

	return xray::math::float2( result.elements[0], result.elements[1]);
}

static u8 pattern_length = 8;

static float frac( float f)
{
	return f - (u32)f;
}

static u8 calc_pattern(xray::math::float2 const& begin, xray::math::float2 const& end)
{
	return static_cast_checked<u8>(frac( xray::math::max(abs(end.elements[0]-begin.elements[0]), abs(end.elements[1]-begin.elements[1])) / (float)pattern_length ) * ((float)pattern_length));
}

void system_renderer::draw_screen_lines	( xray::math::float3 const* points, u32 count, color clr, float width, u32 pattern, bool use_depth)
{
	(void)&width;

	// Vertices already in world space.
	math::float4x4 wvpMatrix = math::mul4x4( m_scene.get_v(), m_scene.get_p());

	u32 screen_width = xray::render_dx10::device::ref().get_width();
	u32 screen_height = xray::render_dx10::device::ref().get_height();
	
	PIX_EVENT( draw_screen_lines);
	u32 u32_color = clr.get_d3dcolor();
	u32 vertex_count = count * 2 - 2;

	// Prepare index buffer
	u32 ioffset;
	u16* ibuffer = (u16*) m_index_stream.lock( vertex_count, ioffset);

	// Prepare vertex buffer
	vertex_colored_sl* vbuffer;
	u32 voffset;
	m_vertex_stream.lock( vertex_count, &vbuffer, voffset);

	float shift = 0.0f;

	for( u32 i = 1; i < count;  ++i)
	{
		const xray::math::float3& start_point	= points[ i-1 ],
								  current_point = points[ i   ];
		
		vertex_colored_sl&		  line_begin	= *vbuffer++;
		vertex_colored_sl&		  line_end		= *vbuffer++;
		
		line_begin.position = start_point;
		line_begin.color = u32_color;
		line_begin.begin_line_position = start_point;

		line_begin.shift = line_end.shift = shift;
		line_begin.pattern = line_end.pattern = pattern;

		xray::math::float2 screen_start = clip_2_screen( start_point, wvpMatrix, screen_width, screen_height);
		xray::math::float2 screen_end	= clip_2_screen( current_point, wvpMatrix, screen_width, screen_height);

		shift = shift + calc_pattern( screen_start, screen_end);

		if ( (u32)shift >= pattern_length)
			shift = shift - pattern_length;

		line_end.position = current_point;
		line_end.color = u32_color;
		line_end.begin_line_position = start_point;

		*ibuffer++ = static_cast_checked<u16>(i*2-2);
		*ibuffer++ = static_cast_checked<u16>(i*2-1);
	}

	m_vertex_stream.unlock();
	m_index_stream.unlock();

	m_colored_geom_sl->apply();

	if( use_depth)
		m_sh_sl->apply		( effect_system_line::z_enabled);
	else
		m_sh_sl->apply		( effect_system_line::z_disabled);

	backend::ref().set_constant( m_WVP_sl, wvpMatrix);

	backend::ref().render_indexed( D3D_PRIMITIVE_TOPOLOGY_LINELIST, vertex_count, ioffset , voffset );
}

void system_renderer::draw_3D_point	( xray::math::float3 const& position, float width, color const color)
{
	u32 screen_width  = xray::render_dx10::device::ref().get_width(),
		screen_height = xray::render_dx10::device::ref().get_height();
	
	float4x4 view_matrix = m_scene.get_v(),
			 proj_matrix = m_scene.get_p();

	float4x4 inv_view_proj_matrix = math::mul4x4( view_matrix, proj_matrix);

	inv_view_proj_matrix.try_invert( inv_view_proj_matrix);

	float4x4 inv_view_matrix = view_matrix;
	inv_view_matrix.try_invert(inv_view_matrix);

	// Distance to view pos.
	float dist = ( float3( inv_view_matrix.e30, inv_view_matrix.e31, inv_view_matrix.e32) - position).magnitude();

	// Calc quad vertex offsets.
	float3 offset_by_x = inv_view_proj_matrix.transform_direction( float3( 1, 0, 0 ) ).normalize() * 1.0f/(float)screen_width  * width*0.5 * dist,
		   offset_by_y = inv_view_proj_matrix.transform_direction( float3( 0,-1, 0 ) ).normalize() * 1.0f/(float)screen_height * width*0.5 * dist;
	
	float3 quad_position[4] = { 
		position - offset_by_x - offset_by_y,
		position - offset_by_x + offset_by_y, 
		position + offset_by_x + offset_by_y,
		position + offset_by_x - offset_by_y
	};
	
	colored_vertices_type vertices;
	colored_indices_type indices;

	// Setup vertices.
	for ( u32 i = 0; i < 4; ++i)
	{
		xray::render::vertex_colored vtx;
		vtx.color = color.get_d3dcolor();
		vtx.position = quad_position[i];
		vertices.push_back( vtx);
	}

	// Setup indices.
	indices.push_back(2); indices.push_back(1); indices.push_back(0);
	indices.push_back(3); indices.push_back(2); indices.push_back(0);

	draw_triangles(vertices, indices);
}

void system_renderer::draw_triangles	( colored_vertices_type const& vertices, colored_indices_type  const& indices)
{
	// Prepare ill vertex buffer.
	u32 voffset;
	render::vertex_colored* vbuffer = (render::vertex_colored*) m_vertex_stream.lock( vertices.size(), sizeof(render::vertex_colored), voffset);
	memory::copy( vbuffer, vertices.size()*sizeof(render::vertex_colored), &vertices[0], vertices.size()*sizeof(render::vertex_colored));
	m_vertex_stream.unlock();

	// Prepare index buffer
	u32 ioffset;
	u16* ibuffer = (u16*) m_index_stream.lock( indices.size(), ioffset);
	memory::copy( ibuffer, indices.size()*sizeof(u16), &indices[0], indices.size()*sizeof(u16));
	m_index_stream.unlock();

	m_colored_geom->apply	();

	if (!m_color_write)
		m_sh_vcolor->apply		( effect_system_colored::voided);
	else 
	if( !m_grid_mode)
		m_sh_vcolor->apply		( effect_system_colored::solid);
	else
		m_sh_vcolor->apply		( effect_system_colored::stenciled);


	backend::ref().set_constant ( m_grid_density_constant, m_grid_density);

	backend::ref().render_indexed( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indices.size(), ioffset /* ? */, voffset );
}

void system_renderer::setup_grid_render_mode	( u32 grid_density )
{
	m_grid_mode = true;
	m_grid_density = grid_density/100.f;
}

void system_renderer::remove_grid_render_mode( )
{
	m_grid_mode = false;
}

void system_renderer::setup_rotation_control_modes ( bool color_write)
{
	m_color_write = color_write;
}

void system_renderer::draw_ui_vertices( vertex_formats::TL const * vertices, u32 const & count, int prim_type, int point_type )
{
	u32 v_offset;
	vertex_formats::TL* vbuffer;
	m_vertex_stream.lock( count, &vbuffer, v_offset);
	memory::copy( vbuffer, count*sizeof(vertex_formats::TL), vertices, count*sizeof(vertex_formats::TL));
	m_vertex_stream.unlock();

	m_ui_geom->apply();

	if( prim_type == 0)
	{
		if( point_type == 0)
			m_sh_ui->apply( effect_system_ui::ui_font);

		else if( point_type == 1)
			m_sh_ui->apply( effect_system_ui::ui);

		else if( point_type == 2)
			m_sh_ui->apply( effect_system_ui::ui_fill);

		else
			UNREACHABLE_CODE();

		backend::ref().render_indexed( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 3*count/2, 0, v_offset);

	}
	else if( prim_type == 1)
	{
		m_sh_ui->apply( effect_system_ui::ui_line_strip);

		backend::ref().render( D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, count, v_offset);
	}
	else
	{
		UNREACHABLE_CODE();
	}

}

} // namespace render_dx10
} // namespace xray


