////////////////////////////////////////////////////////////////////////////
//	Created		: 17.02.2010
//	Author		: Armen Abroyan
//	Copyright ( C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/effect_forward_system.h>
#include <xray/render/core/effect_compiler.h>

namespace xray {
namespace render_dx10 {

effect_forward_system::effect_forward_system() 
{
}

void effect_forward_system::compile( effect_compiler& compiler, const effect_compilation_options& options)
{
		XRAY_UNREFERENCED_PARAMETER	( options);

		shader_defines_list	defines;
		make_defines( defines);

		compiler.begin_technique( /*LEVEL*/);
		compiler.begin_pass			( "test", "test", defines)
				.set_depth			( true, true)
				.set_alpha_blend	( false)
				.set_texture		( "t_base", options.tex_list[0])
			.end_pass		()
		.end_technique	();

		return;
}

void effect_forward_system::load( memory::reader& mem_reader)
{
	effect::load( mem_reader);
}

} // namespace render
} // namespace xray
