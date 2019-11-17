////////////////////////////////////////////////////////////////////////////
//	Created		: 04.02.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/stage_screen_image.h>
#include <xray/render/engine/effect_combine.h>
#include <xray/render/engine/environment.h>
#include <xray/render/core/effect_manager.h>
#include <xray/render/core/effect_compiler.h>
#include <xray/render/core/resource_manager.h>

namespace xray {
namespace render_dx10 {

#pragma pack(push,4)
struct screen_image_vertex
{
	inline screen_image_vertex( float  x, float  y, float tcu, float tcv ) : position ( x, y), text_coords ( tcu, tcv) {}

	float2  position;
	float2  text_coords;
};
#pragma pack(pop)

class scr_quad_effect: public effect
{
public:
	void compile( effect_compiler& compiler, const effect_compilation_options& options)
	{
		XRAY_UNREFERENCED_PARAMETER	( options );

		shader_defines_list	defines;
		make_defines( defines);

		compiler.begin_technique( /*LEVEL*/);
		compiler.begin_pass( "sa_quad", "image", defines)
					.set_depth			( false, false, D3D_COMPARISON_ALWAYS)
					.set_alpha_blend	( false)
					.set_stencil		( false)
					//.def_sampler( "s_base", r2_rt_albedo)
				.end_pass()
			.end_technique();
	}

private:
};

stage_screen_image::stage_screen_image( scene_context* context):stage( context)
{
// 	D3D_INPUT_ELEMENT_DESC	dcl[MAX_FVF_DECL_SIZE];
// 	R_CHK( D3DXDeclaratorFromFVF( D3DFVF_XYZRHW | D3DFVF_TEX1, dcl));
	D3D_INPUT_ELEMENT_DESC dcl[] = 
	{
		{"POSITIONT",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 0,	D3D_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 8, 	D3D_INPUT_PER_VERTEX_DATA, 0}
	};
	m_decl_ptr	= resource_manager::ref().create_declaration( dcl);

	scr_quad_effect b;
	m_sh		= effect_manager::ref().create_effect( &b);

	m_textures.resize(1, NULL);
}

void stage_screen_image::execute( ref_texture t_image)
{
	PIX_EVENT( stage_screen_image );

	backend::ref().reset_render_targets();
	backend::ref().reset_depth_stencil_target();

	backend::ref().clear_render_targets( math::color( 0.f, 0.f, 0.f, 0.f));

	m_sh->apply();

	ASSERT( m_textures.size() == 1);
	m_textures[0] = t_image;
//	backend::ref().set_ps_textures( NULL);      // To refuse caching of texture list.
	backend::ref().set_ps_textures( &m_textures);
	//t_image->apply(0);

	backend::ref().set_declaration( &*m_decl_ptr);

	u32 base_offset;
	screen_image_vertex* vertices = (screen_image_vertex*)backend::ref().vertex.lock( 4, sizeof(screen_image_vertex), base_offset);
	
// 	vertices[1] = screen_image_vertex( x1-orig_x, y1-orig_y, 0, 0 );
// 	vertices[0] = screen_image_vertex( x2-orig_x, y1-orig_y, 1, 0 );
// 	vertices[3] = screen_image_vertex( x1-orig_x, y2-orig_y, 0, 1 );
// 	vertices[2] = screen_image_vertex( x2-orig_x, y2-orig_y, 1, 1 );

	vertices[0] = screen_image_vertex( -1,  1, 0, 0 );
	vertices[1] = screen_image_vertex(  1,  1, 1, 0 );
	vertices[2] = screen_image_vertex( -1, -1, 0, 1 );
	vertices[3] = screen_image_vertex(  1, -1, 1, 1 );

	backend::ref().vertex.unlock();

	backend::ref().set_vb( backend::ref().vertex.buffer(), sizeof(screen_image_vertex));


// 	backend::ref().flush();
// 	device::ref().device()->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, vertices, 6*4);

	backend::ref().render( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, base_offset);
}

} // namespace render
} // namespace xray

