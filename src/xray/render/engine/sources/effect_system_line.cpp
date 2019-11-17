////////////////////////////////////////////////////////////////////////////
//	Created		: 28.05.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "../effect_system_line.h"
#include "xray/render/core/effect_compiler.h"

namespace xray {
namespace render_dx10 {

void effect_system_line::compile(effect_compiler& c, const effect_compilation_options& options)
{
	XRAY_UNREFERENCED_PARAMETER	( options );

	shader_defines_list	defines;
	make_defines(defines);

	c.begin_technique( /*z_enabled*/)
		.begin_pass	( "system_line", "system_line", defines)
			.set_depth( true, true)
		.end_pass	()
	.end_technique();

	c.begin_technique( /*z_disabled*/)
		.begin_pass	( "system_line", "system_line", defines)
			.set_depth( false, false)
		.end_pass	()
	.end_technique();
}

} // namespace render_dx10
} // namespace xray
