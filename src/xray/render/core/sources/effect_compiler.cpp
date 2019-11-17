////////////////////////////////////////////////////////////////////////////
//	Created		: 13.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/core/effect_compiler.h>
#include <xray/render/core/effect_manager.h>
#include <xray/render/core/resource_manager.h>
#include <xray/render/core/backend.h>

namespace xray {
namespace render_dx10 {


effect_compiler::effect_compiler	( res_effect& effect): 
	m_compilation_target		( effect), 
	m_technique_idx				( 0), 
	m_pass_idx					( 0),
#pragma warning(push)
#pragma warning(disable:4355)
	m_sampler_state_descriptor	( *this),
#pragma warning(pop)
	m_curr_sampler_name			( NULL)

{

}

effect_compiler::~effect_compiler()
{

}

effect_compiler& effect_compiler::begin_pass( char const * vs, char const * ps, shader_defines_list& defines)
{
	m_state_descriptor.reset();
	m_bindings.clear();

	m_vs_hw = resource_manager::ref().create_vs_hw( vs, defines);
	m_ps_hw = resource_manager::ref().create_ps_hw( ps, defines);

	m_vs_descriptor.reset( &*m_vs_hw);
	m_ps_descriptor.reset( &*m_ps_hw);

	set_depth		( true, true);
	set_alpha_blend	( false, D3D_BLEND_ONE, D3D_BLEND_ZERO);

	return *this;
}

effect_compiler& effect_compiler::set_depth		( bool enable, bool write_enable, D3D_COMPARISON_FUNC cmp_func)
{
	m_state_descriptor.set_depth	( enable, write_enable, cmp_func);

	return *this;
}

// effect_compiler& effect_compiler::set_stencil				( BOOL enable, u32 ref, u8 read_mask, u8 write_mask)
// {
// 	m_state_descriptor.set_stencil	( enable, ref, read_mask, write_mask);
// 
// 	return *this;
// }

effect_compiler& effect_compiler::set_stencil				( BOOL enable, u32 ref, u8 read_mask, u8 write_mask, D3D_COMPARISON_FUNC func, D3D_STENCIL_OP fail, D3D_STENCIL_OP pass, D3D_STENCIL_OP zfail)
{
	m_state_descriptor.set_stencil	( enable, ref, read_mask, write_mask);
	m_state_descriptor.set_stencil_frontface	( func, fail, pass, zfail);
	m_state_descriptor.set_stencil_backface	( func , fail, pass, zfail);

	return *this;
}


effect_compiler& effect_compiler::set_stencil_ref	( u32 ref)
{
	m_state_descriptor.set_stencil_ref	( ref);

	return *this;
}

effect_compiler& effect_compiler::set_alpha_blend	( BOOL blend_enable, D3D_BLEND src_blend , D3D_BLEND dest_blend)
{
	m_state_descriptor.set_alpha_blend	( blend_enable, src_blend , dest_blend);

	return *this;
}

effect_compiler& effect_compiler::def_sampler		( char const * name, sampler_state_descriptor & sampler_desc)
{
	if( backend::ref().find_registered_sampler( name) )
	{
		ASSERT( 0, "A sampler with name \"%s\" was already registered globally.", name);
		return *this;
	}

	res_sampler_state* sampler  =  resource_manager::ref().create_sampler_state( sampler_desc);

	bool res = false;
	if( m_vs_hw)
		res |= m_vs_descriptor.set_sampler( name, sampler);

	if( m_ps_hw)
		res |= m_ps_descriptor.set_sampler( name, sampler);

	if( !res)
		LOG_WARNING( "The sampler \"%s\" was specified effect descriptor, but not used by any of effect shaders.", name);

	return *this;
}

effect_compiler& effect_compiler::def_sampler		( char const * name, D3D_TEXTURE_ADDRESS_MODE address , D3D_FILTER filter)
{
	m_sampler_state_descriptor.reset()
								.set_address_mode	( address, address, address)
								.set_filter		( filter);

	def_sampler( name, m_sampler_state_descriptor);

	return *this;
}

sampler_state_descriptor & effect_compiler::begin_sampler	( char const * name)
{
	m_curr_sampler_name = name;
	return m_sampler_state_descriptor;
}

effect_compiler			& effect_compiler::end_sampler		()
{
	ASSERT( m_curr_sampler_name, "There was no call of \"begin_sampler\" function!");

	if( !m_curr_sampler_name)
		return *this;

	def_sampler( m_curr_sampler_name, m_sampler_state_descriptor);
	m_curr_sampler_name = NULL;
	return *this;
}

effect_compiler& effect_compiler::set_texture		( char const * name, char const * phisical_name)
{
	bool res = false;
	if( m_vs_hw)
		res |= m_vs_descriptor.use_texture( name);

	if( m_ps_hw)
		res |= m_ps_descriptor.use_texture( name);

	if( !res)
	{
		LOG_INFO( "The texutre \"%s\" was specified effect descriptor, but not used by any of effect shaders.", name);
		return *this;
	}

	ref_texture ref = ( resource_manager::ref().create_texture( phisical_name/*, true*/));
	return set_texture( name, &*ref);
}

effect_compiler& effect_compiler::set_texture		( char const * name, res_texture * texutre)
{
	bool res = false;
	if( m_vs_hw)
		res |= m_vs_descriptor.set_texture( name, texutre);

	if( m_ps_hw)
		res |= m_ps_descriptor.set_texture( name, texutre);

	if( !res)
		LOG_INFO( "The texutre \"%s\" was specified effect descriptor, but not used by any of effect shaders.", name);

	return *this;
}

// effect_compiler& effect_compiler::def_sampler_clf( char const * name, shared_string texture)
// {
// 	return def_sampler( name, texture, D3D_TEXTURE_ADDRESS_CLAMP, D3D_FILTER_MIN_MAG_LINEAR_MIP_POINT);
// }
// 
// effect_compiler& effect_compiler::def_sampler_rtf( char const * name, shared_string texture)
// {
// 	return def_sampler( name,texture, D3D_TEXTURE_ADDRESS_CLAMP, D3D_FILTER_MIN_MAG_MIP_POINT);
// }
// 
// effect_compiler& effect_compiler::def_sampler_clw( char const * name, shared_string texture)
// {
// 	u32 s = get_sampler( name);
// 	
// 	if ( u32( -1) != s)
// 	{
// 		def_sampler( name, texture, D3D_TEXTURE_ADDRESS_CLAMP, D3D_FILTER_MIN_MAG_LINEAR_MIP_POINT);
// 	}
// 
// 	return *this;
// }

effect_compiler& effect_compiler::set_cull_mode		( D3D_CULL_MODE mode)
{
	m_state_descriptor.set_cull_mode( mode);
	return  *this;
}
effect_compiler& effect_compiler::color_write_enable	( D3D_COLOR_WRITE_ENABLE mode)
{
	m_state_descriptor.color_write_enable( mode);
	return *this;
}

effect_compiler& effect_compiler::set_fill_mode		( D3D_FILL_MODE fill_mode)
{
	m_state_descriptor.set_fill_mode	( fill_mode);
	return *this;
}

effect_compiler& effect_compiler::bind_constant( constant_binding const& binding)
{
	m_bindings.add( binding);
	return *this;
}

effect_compiler& effect_compiler::end_pass()
{
	m_vs_descriptor.data().constants.apply_bindings( m_bindings);
	m_ps_descriptor.data().constants.apply_bindings( m_bindings);

	ref_state	state	= resource_manager::ref().create_state	( m_state_descriptor);
	ref_vs		vs		= resource_manager::ref().create_vs		( m_vs_descriptor);
	ref_ps		ps		= resource_manager::ref().create_ps		( m_ps_descriptor);

//	ref_constant_table constants= resource_manager::ref().create_const_table( m_constants);
//	ref_texture_list tex_lst= effect_manager::ref().create_texture_list( m_tex_lst);
	
	ref_pass pass = effect_manager::ref().create_pass( res_pass(vs, NULL, ps, state));

	m_sh_technique.m_passes.push_back( pass);

	m_state_descriptor.reset();
//	m_constants.clear();
//	m_tex_lst.clear();
	m_bindings.clear();

	m_vs_hw = 0;
	m_ps_hw = 0;

	++m_pass_idx;

	return *this;
}

effect_compiler& effect_compiler::begin_technique()
{
	m_sh_technique.m_passes.clear();
	m_pass_idx = 0;

	return *this;
}

void effect_compiler::end_technique()
{
	ref_shader_technique se = effect_manager::ref().create_effect_technique( m_sh_technique);

	m_compilation_target.m_techniques.push_back( se);

	m_sh_technique.m_passes.clear();

	++m_technique_idx; m_pass_idx = 0;
}



} // namespace render 
} // namespace xray 

