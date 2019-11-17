////////////////////////////////////////////////////////////////////////////
//	Created		: 25.03.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright ( C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/effect_light_mask.h>
#include <xray/render/core/effect_compiler.h>


namespace xray {
namespace render_dx10 {

void effect_light_mask::compile( effect_compiler& compiler, const effect_compilation_options& options)
{
	XRAY_UNREFERENCED_PARAMETER	( options);
	shader_defines_list	defines;
	make_defines( defines);

	compiler.begin_technique( /*SE_MASK_SPOT*/)
		.begin_pass	( "accum_mask", "dumb", defines)
			.set_depth		( true, false, D3D_COMPARISON_GREATER_EQUAL)
			.set_texture	( "t_position",    r2_rt_p)
		.end_pass		()
	.end_technique();

	compiler.begin_technique( /*SE_MASK_POINT*/)
		.begin_pass	( "accum_mask", "dumb", defines)
			.set_depth			( true, false, D3D_COMPARISON_GREATER_EQUAL)
			.color_write_enable ( D3D_COLOR_WRITE_ENABLE_NONE)
			.set_cull_mode		( D3D_CULL_FRONT)
			.set_stencil		( true, 0x00, 0x01, 0xff, D3D_COMPARISON_LESS_EQUAL, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP)
			.set_texture		( "t_position",    r2_rt_p)
		.end_pass	()

		.begin_pass	( "accum_mask", "dumb", defines)
			.set_depth			( true, false, D3D_COMPARISON_GREATER_EQUAL)
			.color_write_enable ( D3D_COLOR_WRITE_ENABLE_NONE)
			.set_cull_mode		( D3D_CULL_BACK)
			.set_stencil		( true, 0x01, 0x01, 0xff, D3D_COMPARISON_LESS_EQUAL, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP)
			.set_texture		( "t_position",    r2_rt_p)
		.end_pass	()

	.end_technique();

	static const float temp_alpha_ref = 1.f;
	compiler.begin_technique( /*SE_MASK_DIRECT*/)
		.begin_pass	( "stub_notransform_t", "accum_sun_mask", defines)
			.set_depth		( false, false)
			.set_stencil	( true, 0x00, 0x01, 0xff, D3D_COMPARISON_LESS_EQUAL, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP)
			.set_cull_mode	( D3D_CULL_NONE)
			.color_write_enable	( D3D_COLOR_WRITE_ENABLE_NONE)
			.set_alpha_blend	( TRUE, D3D_BLEND_ZERO, D3D_BLEND_ONE)
			.bind_constant		( "alpha_ref",	&temp_alpha_ref)
			.set_texture		( "t_normal",      r2_rt_n)
		.end_pass()
	.end_technique();

	compiler.begin_technique( /*SE_MASK_STENCIL*/)
		.begin_pass		( "stub_notransform_t", "dumb", defines)
		.set_cull_mode		( D3D_CULL_NONE)
		.color_write_enable	( D3D_COLOR_WRITE_ENABLE_NONE)
		.set_stencil		( true, 0x00, 0x00, 0xFE, D3D_COMPARISON_ALWAYS, D3D_STENCIL_OP_ZERO, D3D_STENCIL_OP_ZERO, D3D_STENCIL_OP_ZERO)
		.end_pass		()
	.end_technique	();
	//switch ( C.iElement) 
	//{
	//case SE_MASK_SPOT:		// spot or omni-part
	//	C.r_Pass			( "accum_mask", 		"dumb", 				false, 	TRUE, FALSE);
	//	C.r_Sampler_rtf		( "s_position", 		r2_RT_P);
	//	C.r_End				();
	//	break;
	//case SE_MASK_POINT:		// point
	//	C.r_Pass			( "accum_mask", 		"dumb", 				false, 	TRUE, FALSE);
	//	C.r_Sampler_rtf		( "s_position", 		r2_RT_P);
	//	C.r_End				();
	//	break;
	//case SE_MASK_DIRECT:	// stencil mask for directional light
	//	C.r_Pass			( "null", 			"accum_sun_mask", 	false, 	FALSE, FALSE, TRUE, D3D_BLEND_ZERO, D3D_BLEND_ONE, TRUE, 1);
	//	C.r_Sampler_rtf		( "s_normal", 		r2_RT_N);
	//	C.r_End				();
	//	break;
	//case SE_MASK_ACCUM_VOL:	// copy accumulator ( temp -> real), volumetric ( usually after blend)
	//	C.r_Pass			( "accum_volume", 	"copy_p", 			false, 	FALSE, FALSE);
	//	C.r_Sampler_rtf		( "s_base", 			r2_RT_accum_temp	);
	//	C.r_End				();
	//	break;
	//case SE_MASK_ACCUM_2D:	// copy accumulator ( temp -> real), 2D ( usually after sun-blend)
	//	C.r_Pass			( "null", 			"copy", 				false, 	FALSE, FALSE);
	//	C.r_Sampler_rtf		( "s_base", 			r2_RT_accum_temp	);
	//	C.r_End				();
	//	break;
	//case SE_MASK_ALBEDO:	// copy accumulator, 2D ( for accum->color, albedo_wo)
	//	C.r_Pass			( "null", 			"copy", 				false, 	FALSE, FALSE);
	//	C.r_Sampler_rtf		( "s_base", 			r2_RT_accum			);
	//	C.r_End				();
	//	break;
	//}
}

} // namespace render
} // namespace xray

