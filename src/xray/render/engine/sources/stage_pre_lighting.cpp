////////////////////////////////////////////////////////////////////////////
//	Created		: 04.02.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/stage_pre_lighting.h>


namespace xray {
namespace render_dx10 {

void stage_pre_lighting::execute( )
{
	PIX_EVENT( stage_pre_lighting );
	
	m_context->m_light_marker_id = stencil_light_marker_start;
	backend::ref().set_render_targets( &*m_context->m_rt_accumulator, 0, 0);
	backend::ref().reset_depth_stencil_target();

	backend::ref().clear_render_targets( math::color( 0.0f, 0.0f, 0.0f, 0.0f));
//	backend::ref().clear_depth_stencil( D3D_CLEAR_DEPTH | D3D_CLEAR_DEPTH, 1.0f, 0);
}

} // namespace render
} // namespace xray
