////////////////////////////////////////////////////////////////////////////
//	Created		: 17.05.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "../effect_system_colored.h"
#include <xray/render/core/effect_compiler.h>

namespace xray {
namespace render_dx10 {

void effect_system_colored::compile(effect_compiler& c, const effect_compilation_options& options)
{
	XRAY_UNREFERENCED_PARAMETER	( options );

	shader_defines_list	defines;
	make_defines(defines);

	c.begin_technique( /*solid*/)
		.begin_pass	( "color", "color", defines)
			//.set_cull_mode(D3D11_CULL_NONE)
		.end_pass	()
	.end_technique();

	c.begin_technique( /*stenciled*/)
		.begin_pass	( "color", "color_doted", defines)
			.set_depth	( true, false, D3D_COMPARISON_ALWAYS)
			//.set_cull_mode(D3D11_CULL_NONE)
		.end_pass	()
	.end_technique();

	c.begin_technique( /*voided*/)
		.begin_pass	( "color", "color", defines)
			.color_write_enable( D3D_COLOR_WRITE_ENABLE_NONE)
			//.set_cull_mode(D3D11_CULL_NONE)
		.end_pass	()
	.end_technique();
}

} // namespace render
} // namespace xray
