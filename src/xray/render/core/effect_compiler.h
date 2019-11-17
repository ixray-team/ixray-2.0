////////////////////////////////////////////////////////////////////////////
//	Created		: 13.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef EFFECT_COMPILER_H_INCLUDED
#define EFFECT_COMPILER_H_INCLUDED

#include <xray/fixed_string.h>
#include "constant_bindings.h"
#include "state_descriptor.h"
#include "sampler_state_descriptor.h"
#include "res_effect.h"
#include "res_vs_hw.h"
#include "res_gs_hw.h"
#include "res_ps_hw.h"

#include "res_xs.h"


namespace xray {
namespace render_dx10 {



class effect_compiler: public boost::noncopyable
{
public:

	void	set_mapping			();

	effect_compiler& begin_pass	( LPCSTR vs, char const * ps, shader_defines_list& defines);
	effect_compiler& end_pass	();

	effect_compiler& begin_technique();
	void end_technique();

	effect_compiler& set_depth			( bool enable, bool write_enable, D3D_COMPARISON_FUNC cmp_func = D3D_COMPARISON_LESS_EQUAL);
//	effect_compiler& set_stencil		( BOOL enable, u32 ref=0x00, u8 mask=0x00, u8 writemask=0x00);
	effect_compiler& set_stencil		( BOOL enable, u32 ref=0x00, u8 mask=0x00, u8 writemask=0x00, D3D_COMPARISON_FUNC func = D3D_COMPARISON_ALWAYS, D3D_STENCIL_OP fail=D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP pass=D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP zfail=D3D_STENCIL_OP_KEEP);
	effect_compiler& set_stencil_ref	( u32 ref);
	effect_compiler& set_alpha_blend	( BOOL blend_enabled, D3D_BLEND src_blend = D3D_BLEND_ONE, D3D_BLEND dest_blend = D3D_BLEND_ZERO);
	effect_compiler& set_cull_mode		( D3D_CULL_MODE mode);
	effect_compiler& color_write_enable	( D3D_COLOR_WRITE_ENABLE mode = D3D_COLOR_WRITE_ENABLE_ALL);
	effect_compiler& set_fill_mode		( D3D_FILL_MODE fill_mode);

	// Sampler functions
	effect_compiler& def_sampler		( char const * hlsl_name, sampler_state_descriptor & sampler_desc);
	effect_compiler& def_sampler		( char const * hlsl_name, D3D_TEXTURE_ADDRESS_MODE address = D3D_TEXTURE_ADDRESS_WRAP, D3D_FILTER filter = D3D_FILTER_MIN_MAG_MIP_LINEAR);

	sampler_state_descriptor & begin_sampler	( char const * name); 
	effect_compiler			& end_sampler	(); 

	// Texture functons
	effect_compiler& set_texture		( char const * hlsl_name, char const *	phisical_name);
	effect_compiler& set_texture		( char const * hlsl_name, shared_string phisical_name)  { return set_texture( hlsl_name, phisical_name.c_str());}
	effect_compiler& set_texture		( char const * hlsl_name, res_texture * texutre);

	
// 	effect_compiler& def_sampler_clf	( char const * name, shared_string texture);
// 	effect_compiler& def_sampler_rtf	( char const * name, shared_string texture);
// 	effect_compiler& def_sampler_clw	( char const * name, shared_string texture);

	// This overwrites global binding.
	effect_compiler& bind_constant( constant_binding const& binding);
	template <typename T>
	effect_compiler& bind_constant( shared_string hlsl_name, T const * source)					{ return bind_constant( constant_binding( hlsl_name, source)); }

public:
	effect_compiler	( res_effect& effect);
	~effect_compiler();

private:

	u32	effect_compiler::get_sampler( char const * name);

	void set_samp_texture		( u32 samp, char const * name);
	void set_samp_address		( u32 samp, u32	address);
	void set_samp_filter_min	( u32 samp, u32 filter);
	void set_samp_filter_mip	( u32 samp, u32 filter);
	void set_samp_filter_mag	( u32 samp, u32 filter);
	void set_samp_filter		( u32 samp, D3D_FILTER filter);

private:
	res_effect&	m_compilation_target;
	
	u32	m_technique_idx;
	u32	m_pass_idx;

	ref_ps_hw m_ps_hw;
	ref_vs_hw m_vs_hw;

	state_descriptor				m_state_descriptor;
	sampler_state_descriptor		m_sampler_state_descriptor;
	vs_descriptor						m_vs_descriptor;
	ps_descriptor						m_ps_descriptor;
	char const *					m_curr_sampler_name;
	//res_texture_list				m_tex_lst;
	//res_constant_table			m_constants;
	constant_bindings				m_bindings;

	res_shader_technique			m_sh_technique;

}; // class effect_compiler


} // namespace render 
} // namespace xray 


#endif // #ifndef EFFECT_COMPILER_H_INCLUDED