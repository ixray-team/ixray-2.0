////////////////////////////////////////////////////////////////////////////
//	Created		: 26.03.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright ( C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/effect_terrain_NEW.h>
#include <xray/render/core/effect_compiler.h>


namespace xray {
namespace render_dx10 {

effect_terrain_NEW::effect_terrain_NEW(): effect_deffer_base( true, true, true)
{
	m_blend = true;
	m_desc.m_version = 1;
	m_not_an_tree = false;
}

void effect_terrain_NEW::compile( effect_compiler& compiler, const effect_compilation_options& options)
{
		XRAY_UNREFERENCED_PARAMETER	( options);

		shader_defines_list	defines;
		make_defines( defines);

		compiler.begin_technique( /*LEVEL*/);
		compiler.begin_pass			( "terrain_NEW", "terrain_NEW", defines)
				.set_depth			( true, true)
				.set_stencil		( true, 0x01, 0xff, 0xff, D3D_COMPARISON_ALWAYS, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP)
				.set_alpha_blend	( FALSE)
				.set_texture		( "t_tile", options.tex_list[0])
				.def_sampler		( "s_tile", D3D_TEXTURE_ADDRESS_WRAP, D3D_FILTER_ANISOTROPIC)
			.end_pass	()
		.end_technique();

		return;
}

void effect_terrain_NEW::load( memory::reader& mem_reader)
{
	effect::load( mem_reader);

	if ( 1==m_desc.m_version)
	{
		xrP_BOOL prop_bool;
		xrPREAD_PROP( mem_reader, xrPID_BOOL, prop_bool);
		m_blend = prop_bool.value != 0;

		xrPREAD_PROP( mem_reader, xrPID_BOOL, prop_bool);
		m_not_an_tree = prop_bool.value != 0;
	}
}

} // namespace render
} // namespace xray
