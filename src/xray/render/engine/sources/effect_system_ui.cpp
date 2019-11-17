////////////////////////////////////////////////////////////////////////////
//	Created		: 17.05.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/effect_system_ui.h>
#include <xray/render/core/effect_compiler.h>

namespace xray {
namespace render_dx10 {

void effect_system_ui::compile(effect_compiler& c, const effect_compilation_options& options)
{
	XRAY_UNREFERENCED_PARAMETER	( options );

	shader_defines_list	defines;
	make_defines(defines);

	c.begin_technique( /*ui_font*/)
		.begin_pass	( "stub_notransform_t", "ui_font", defines)
			.set_depth( false, false)
			.set_alpha_blend( true, D3D_BLEND_SRC_ALPHA, D3D_BLEND_INV_SRC_ALPHA)
			.set_cull_mode( D3D_CULL_NONE)
			.set_texture( "t_base", options.tex_list[0])
		.end_pass	()
	.end_technique();

	c.begin_technique( /*ui*/)
		.begin_pass	( "stub_notransform_t", "ui", defines)
			.set_depth( false, false)
			.set_alpha_blend( true, D3D_BLEND_SRC_ALPHA, D3D_BLEND_INV_SRC_ALPHA)
			.set_cull_mode( D3D_CULL_NONE)
			.set_texture( "t_base", options.tex_list[1])
		.end_pass	()
	.end_technique();

	c.begin_technique( /*ui_fill*/)
		.begin_pass	( "stub_notransform_t", "ui_fill", defines)
			.set_depth( false, false)
			.set_alpha_blend( true, D3D_BLEND_SRC_ALPHA, D3D_BLEND_INV_SRC_ALPHA)
			.set_cull_mode( D3D_CULL_NONE)
		.end_pass	()
	.end_technique();

	c.begin_technique( /*ui_line_strip*/)
		.begin_pass	( "stub_notransform_t", "ui_fill", defines)
			.set_depth( false, false)
			.set_cull_mode( D3D_CULL_NONE)
		.end_pass	()
	.end_technique();
}

} // namespace render
} // namespace xray
