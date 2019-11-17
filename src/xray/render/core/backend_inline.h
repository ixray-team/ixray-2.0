////////////////////////////////////////////////////////////////////////////
//	Created		: 16.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef BACKEND_INLINE_H_INCLUDED
#define BACKEND_INLINE_H_INCLUDED

#include <xray/render/core/device.h>

namespace xray {
namespace render_dx10 {

void backend::set_rasterizer_state		( ID3DRasterizerState*		rasterizer_state)
{
	m_dirty_objects.rasterizer_state |= m_rasterizer_state != rasterizer_state;
	m_rasterizer_state = rasterizer_state;
}

void backend::set_septh_stencil_state	( ID3DDepthStencilState*	depth_stencils_State)
{
	m_dirty_objects.depth_stencil_state |= m_depth_stencils_state != depth_stencils_State;
	m_depth_stencils_state = depth_stencils_State;
}

void backend::set_blend_state			( ID3DBlendState *			effect_State)
{
	m_dirty_objects.effect_state |= m_effect_state != effect_State;
	m_effect_state = effect_State;
}

inline void backend::set_vb( res_buffer* vb, u32 vb_stride, u32 offset /* in bytes*/)
{
	m_dirty_objects.vertex_buffer |= ( m_vb != vb || vb_stride != m_vb_stride || m_vb_offset != offset);

	m_vb		= vb;
	m_vb_stride	= vb_stride;
	m_vb_offset	= offset;
}

inline void backend::set_ib( res_buffer* ib, u32 offset /*in bytes*/)
{
	m_dirty_objects.index_buffer |= (m_ib != ib || m_ib_offset != offset);
	m_ib = ib;
	m_ib_offset = offset;
}

inline void backend::set_vs( res_vs_hw * vs)
{
	m_dirty_objects.vertex_shader |= m_vs != vs;
	m_vs = vs;
	
	// Reseting input layout that the new shader will need new layout.
	// if no layout will be specified until a draw call then a 
	// corresponding input layout will be chosen from resource_manager.
	m_input_layout = m_dirty_objects.vertex_shader ? NULL : m_input_layout;
	m_dirty_objects.input_layout = m_dirty_objects.vertex_shader;
}

inline void backend::set_ps( res_ps_hw * ps)
{
	m_dirty_objects.pixel_shader |= m_ps != ps;
	m_ps = ps;
}


inline void backend::set_declaration ( res_declaration * decl)
{
	if( m_decl != decl)
	{
		m_decl = decl;
		m_dirty_objects.input_declaration = true;

		// Reseting input layout that the new decl will need new layout.
		// if no layout will be specified until a draw call then a 
		// corresponding input layout will be chosen from resource_manager.
		m_input_layout = NULL;
		m_dirty_objects.input_layout = true;
	}
}

inline	void backend::set_input_layout( res_input_layout * layout)
{
	m_dirty_objects.input_layout |= m_input_layout != layout;
	m_input_layout = layout;
}

inline void backend::set_vs_constants( res_constant_table * ctable)
{
	if( m_vs_constants_handler.m_current == ctable)
		return;

	m_vs_constants_handler.assign		( ctable);

	m_dirty_objects.vertex_constants = true;
	m_constant_update_markers[enum_shader_type_vertex] = m_constant_update_counter;
}

inline void backend::set_ps_constants( res_constant_table * ctable)
{
	if( m_ps_constants_handler.m_current == ctable)
		return;

	m_ps_constants_handler.assign		( ctable);

	m_dirty_objects.pixel_constants = true;
	m_constant_update_markers[enum_shader_type_pixel] = m_constant_update_counter;
}

inline void backend::set_vs_textures	( res_texture_list * textures)
{
	if( m_vs_textures_handler.m_current == textures)
		return;
	
	m_vs_textures_handler.assign( textures);
	m_dirty_objects.vertex_textures = true;
}

inline void backend::set_ps_textures	( res_texture_list * textures)
{
	if( m_ps_textures_handler.m_current == textures)
		return;

	m_ps_textures_handler.assign( textures);
	m_dirty_objects.pixel_textures = true;
}

inline void backend::set_vs_samplers	( res_sampler_list * samplers)
{
	if( m_vs_samplers_handler.m_current == samplers)
		return;

	m_vs_samplers_handler.assign( samplers);
	m_dirty_objects.vertex_samplers = true;
}

inline void backend::set_ps_samplers	( res_sampler_list * samplers)
{
	if( m_ps_samplers_handler.m_current == samplers)
		return;

	m_ps_samplers_handler.assign( samplers);
	m_dirty_objects.pixel_samplers = true;
}



inline void backend::set_render_target( enum_render_target_enum target, res_rt const * rt)
{
	ID3DRenderTargetView * rt_view = (rt == NULL) ? NULL : rt->get_taget_view();

	if( m_targets[target] != rt_view)
	{
		m_targets[target] = rt_view;
		m_dirty_targets.render_targets[target] = true;
	}
}

inline	void backend::set_render_targets( res_rt const * rt0, res_rt const * rt1, res_rt const * rt2)
{
	set_render_target( enum_target_rt0, rt0);
	set_render_target( enum_target_rt1, rt1);
	set_render_target( enum_target_rt2, rt2);
}

inline void backend::set_depth_stencil_target( res_rt const * zrt)
{
	ID3DDepthStencilView* zrt_view = (zrt == NULL) ? NULL : zrt->get_depth_stencil_view();

	m_dirty_targets.depth_stencil |=	m_zb != zrt_view;
	m_zb = zrt_view;
}

inline	void backend::reset_render_targets		( bool only_the_base)
{
	m_dirty_targets.render_targets[0] |=	m_targets[0] != m_base_rt;
	m_targets[0] = m_base_rt;

	for( int i = 1; i < enum_target_count && !only_the_base; ++i)
	{
		m_dirty_targets.render_targets[i] |=  NULL != m_targets[i];
		m_targets[i] = NULL;
	}

}

inline	void backend::reset_depth_stencil_target	()
{
	m_dirty_targets.depth_stencil |=	m_zb != m_base_zb;
	m_zb = m_base_zb;
}

inline void backend::flush()
{
	//Here may be used caching to prevent reseting the same state.
	if( m_dirty_objects.rasterizer_state)
		device::ref().d3d_context()->RSSetState				( m_rasterizer_state);

	if( m_dirty_objects.depth_stencil_state)
		device::ref().d3d_context()->OMSetDepthStencilState	( m_depth_stencils_state, m_stencil_ref);

	if( m_dirty_objects.effect_state)
	{
		// --Porting to DX10_
		// give correct blend factors
		float blend_factor[4]  = {0.f,0.f,0.f,0.f} ;
		device::ref().d3d_context()->OMSetBlendState			( m_effect_state, blend_factor, m_sample_mask);
	}

	flush_rt();

// 	if( m_dirty_objects.any())
// 	{
		if( m_dirty_objects.vertex_shader)
		{
			ID3DVertexShader * vs	= (m_vs == (res_vs_hw*)NULL) ? NULL : m_vs->hw_shader();
			device::ref().d3d_context()->VSSetShader( vs SHADER_PARAM_STUB);
		}

		if( m_dirty_objects.pixel_shader)
		{
			ID3DPixelShader * ps	= (m_ps == (res_ps_hw*)NULL) ? NULL : m_ps->hw_shader();
			device::ref().d3d_context()->PSSetShader( ps SHADER_PARAM_STUB);
		}

		if( m_dirty_objects.vertex_buffer)
		{
			ID3DVertexBuffer * buffer = (m_vb == (res_buffer*)NULL) ? NULL : m_vb->hw_buffer();
			device::ref().d3d_context()->IASetVertexBuffers( 0, 1, &buffer, &m_vb_stride, &m_vb_offset);
			//R_CHK( m_device->SetStreamSource( 0, m_vb, 0, m_vb_stride));
		}

		if( m_dirty_objects.index_buffer)
		{
			ID3DIndexBuffer * buffer = (m_ib == (res_buffer*)NULL) ? NULL : m_ib->hw_buffer();
			device::ref().d3d_context()->IASetIndexBuffer( buffer, DXGI_FORMAT_R16_UINT, m_ib_offset);
			//R_CHK( m_device->SetIndices( m_ib));
		}


		m_vs_constants_handler.update_buffers();
		m_ps_constants_handler.update_buffers();

		if( m_dirty_objects.vertex_constants)
			m_vs_constants_handler.apply();

		if( m_dirty_objects.vertex_textures)
		{
			if( m_vs) 
				m_vs_textures_handler.check_for_unset_textures( m_vs->data().textures);

			m_vs_textures_handler.apply();
		}

		if( m_dirty_objects.vertex_samplers)
		{
			if( m_vs) 
				m_vs_samplers_handler.check_for_unset_samplers( m_vs->data().samplers);

			m_vs_samplers_handler.apply();
		}


		if( m_dirty_objects.pixel_constants)
			m_ps_constants_handler.apply();

		if( m_dirty_objects.pixel_textures)
		{
			if( m_ps) 
				m_ps_textures_handler.check_for_unset_textures( m_ps->data().textures);

			m_ps_textures_handler.apply();
		}

		if( m_dirty_objects.pixel_samplers)
		{
			if( m_ps) 
				m_ps_samplers_handler.check_for_unset_samplers( m_ps->data().samplers);

			m_ps_samplers_handler.apply();
		}

		m_constant_update_counter++;

		m_dirty_objects.reset();
//	}
}

inline void backend::flush_rt()
{
	ASSERT( enum_target_zb == enum_target_count+1);
	
	if( m_dirty_targets.any())
		device::ref().d3d_context()->OMSetRenderTargets( enum_target_count, &m_targets[0], m_zb);

	m_dirty_targets.reset();
}


void backend::set_constant( constant_host const * c, float	value)
{
	set_c_impl( c, value);
}

void backend::set_constant( constant_host const * c, math::float2	value)
{
	set_c_impl( c, value);
}

void backend::set_constant( constant_host const * c, math::float3	value)
{
	set_c_impl( c, value);
}

void backend::set_constant( constant_host const * c, math::float4 const&	value)
{
	set_c_impl( c, value);
}

void backend::set_constant( constant_host const * c, float4x4b	const& value)
{
	set_c_impl( c, value);
}

void backend::set_constant( constant_host const * c, int	value)
{
	set_c_impl( c, value);
}

void backend::set_constant( constant_host const * c, math::int2	value)
{
	set_c_impl( c, value);
}

template <typename T>
void backend::set_c_impl( constant_host const * c, T const & arg)
{
	ASSERT( c != NULL);
	COMPILE_ASSERT(	constant_type_traits<T>::value == true, Only_types_described_in_constant_h_are_supported );
	m_vs_constants_handler.set_constant( *c, arg);
	m_gs_constants_handler.set_constant( *c, arg);
	m_ps_constants_handler.set_constant( *c, arg);
}

// template <typename T>
// void backend::set_constant( shared_string name, T const & arg)
// {
// 	constant_host const * const_host = find_constant_host( name, false);
// 	if( const_host)
// 		set_constant( *const_host, arg);
// // 	else
// // 		LOG_ERROR( "A constant with the specified name wasn't registered.");
// }

template <typename T>
void backend::set_ca_impl( constant_host const * c, u32 index, const T& arg)
{
	//m_vs_constants_handler.set( c, index, arg);
}

template <typename T>
void backend::set_ca( shared_string name, u32 index, const T& arg)
{
	constant_host const * const_host = find_constant_host( name, false);
	if( const_host)
		set_ca( const_host, index, arg);
	else
		ASSERT(0, "A constant with the specified name wasn't registered.");
}

inline void backend::reset()
{
	set_stencil_ref			( 0x00);
	set_sample_mask			( 0xFFFFFFFF);

	set_declaration( 0);
	set_rasterizer_state( 0);

	set_septh_stencil_state	( 0);
	set_blend_state	( 0);
	set_vs( 0);
	set_vs_constants( 0);
	set_vs_samplers( 0);
	set_vs_textures( 0);

// 	set_gs( 0);
//  set_gs_constants( 0);
//  set_gs_samplers( 0);
//  set_gs_textures( 0);

	set_ps( 0);
	set_ps_constants( 0);
	set_ps_samplers( 0);
	set_ps_textures( 0);

	set_vs_constants( 0);
	set_vb( 0, 0, 0);
	set_ib( 0, 0);

	reset_render_targets();
	reset_depth_stencil_target();

	m_constant_update_counter ++;
}

inline void backend::clear_render_targets( math::color color)
{
	for( int i = 0; i< enum_target_count; ++i)
		if( m_targets[i])
			device::ref().d3d_context()->ClearRenderTargetView( m_targets[i], color.elements);
}

inline void backend::clear_depth_stencil( u32 flags, float z_value, u8 stencil_value)
{
 	//flush_rt();
	device::ref().d3d_context()->ClearDepthStencilView( m_zb, flags, z_value, stencil_value);
}

inline void backend::update_input_layout()
{
	// Set always
	//if( m_dirty_objects.input_layout)
	{
		if( !m_input_layout)
		{
			ASSERT( m_decl, "It is needed to define either input layout or a vertex declaration.");
			m_input_layout = m_decl->get( &*m_vs->data().signature);
		}

		ASSERT( m_input_layout);
		device::ref().d3d_context()->IASetInputLayout( m_input_layout->hw_layout());
	}
}

inline void	backend::render_indexed( D3D_PRIMITIVE_TOPOLOGY type, u32 index_count, u32 start_index, u32 base_vertex)
{
	flush();
	update_input_layout();
	//stat.calls			++;
	//stat.verts			+= 3*PC;
	//stat.polys			+= PC;

	device::ref().d3d_context()->IASetPrimitiveTopology( type);	

	device::ref().d3d_context()->DrawIndexed( index_count, start_index, base_vertex);
}

inline void	backend::render( D3D_PRIMITIVE_TOPOLOGY type, u32 vertex_count, u32 base_vertex)
{
	flush();
	update_input_layout();
	//stat.calls			++;
	//stat.verts			+= 3*PC;
	//stat.polys			+= PC;

	device::ref().d3d_context()->IASetPrimitiveTopology( type);
	device::ref().d3d_context()->Draw( vertex_count,  base_vertex);

}

void backend::set_stencil_ref			( u32 stencil_ref)
{
	m_dirty_objects.depth_stencil_state |= m_stencil_ref != stencil_ref;
	m_stencil_ref = stencil_ref;
}

void backend::set_sample_mask			( u32 sample_mask)
{
	m_dirty_objects.effect_state |= m_sample_mask != sample_mask;
	m_sample_mask = sample_mask;
}

template<enum_shader_type shader_type>
u32 backend::get_constants_marker ()
{
	return m_constant_update_markers[shader_type];
}

void backend::get_viewports ( u32 & count/*in/out*/, D3D_VIEWPORT * viewports)
{
	device::ref().d3d_context()->RSGetViewports( &count, viewports);
}

void backend::set_viewports ( u32 count, D3D_VIEWPORT const * viewports)
{
	device::ref().d3d_context()->RSSetViewports( count, viewports);
}

void backend::get_viewport ( D3D_VIEWPORT & viewport)
{
	u32 count = 1;
	get_viewports ( count, &viewport);
}

void backend::set_viewport ( D3D_VIEWPORT const & viewports)
{
	set_viewports( 1, &viewports);
}


} // namespace render 
} // namespace xray 


#endif // #ifndef BACKEND_INLINE_H_INCLUDED