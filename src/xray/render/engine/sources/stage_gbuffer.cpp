////////////////////////////////////////////////////////////////////////////
//	Created		: 15.04.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright ( C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/stage_gbuffer.h>
#include <xray/render/engine/model_manager.h>
#include <xray/render/engine/visual.h>
#include <xray/render/engine/terrain.h>
#include <xray/render/core/state_descriptor.h>
#include <xray/render/core/backend.h>

namespace xray {
namespace render_dx10 {

stage_gbuffer::stage_gbuffer( scene_context* context): stage( context)
{
	// Remove after first success to render g buffer.
// 	state_descriptor sim;
// 
// 	sim.color_write_enable();
// 	sim.set_stencil( true, 0x01, 0xff, 0xff, D3D_COMPARISON_ALWAYS, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP);
// 
// 	m_state = resource_manager::ref().create_state( sim);
}

stage_gbuffer::~stage_gbuffer()
{
}

void stage_gbuffer::execute()
{
	PIX_EVENT( stage_gbuffer);

	backend::ref().set_render_targets	( &*m_context->m_rt_position, &*m_context->m_rt_normal, &*m_context->m_rt_color);
	backend::ref().reset_depth_stencil_target();
	backend::ref().clear_render_targets	( color( 0.f, 0.f, 0.f));
	backend::ref().clear_depth_stencil	( D3D_CLEAR_DEPTH|D3D_CLEAR_STENCIL, 1.0f, 0);

//////////////////////////////////////////////////////////////////////////
	if( model_manager::ref().get_draw_editor())
	{
		model_manager::Editor_Visuals& dynamic_visuals = model_manager::ref().get_editor_visuals();

		model_manager::Editor_Visuals::iterator	it_d  = dynamic_visuals.begin(),
			end_d = dynamic_visuals.end();

		for (; it_d!=end_d; ++it_d)
		{
			render_visual* visual = static_cast<render_visual*>(&(*(it_d->visual)));
			ASSERT(visual);
			if( !it_d->beditor_chain || it_d->system_object )
				continue;

			m_context->set_w( it_d->transform );
			visual->render();
		}

////// Rendering terrain cells ///////////////////////////////////////////

		terrain::cells const& terrain_cells = terrain::ref().get_editor_cells();

		terrain::cells::const_iterator it_tr = terrain_cells.begin();
		terrain::cells::const_iterator en_tr = terrain_cells.end();

		m_context->set_w( math::float4x4().identity() );

		for( ; it_tr != en_tr; ++it_tr)
			it_tr->visual->render();

	}

	if( model_manager::ref().get_draw_game())
	{
		model_manager::Editor_Visuals& dynamic_visuals = model_manager::ref().get_editor_visuals();

		model_manager::Editor_Visuals::iterator	it_d  = dynamic_visuals.begin(),
			end_d = dynamic_visuals.end();

		for (; it_d!=end_d; ++it_d)
		{
			render_visual* visual = static_cast<render_visual*>(&(*(it_d->visual)));
			ASSERT(visual);
			if( it_d->beditor_chain || it_d->system_object )
				continue;

			m_context->set_w( it_d->transform );
			visual->render();
		}

		////// Rendering terrain cells ///////////////////////////////////////////
		terrain::cells const& terrain_cells = terrain::ref().get_game_cells();

		terrain::cells::const_iterator it_tr = terrain_cells.begin();
		terrain::cells::const_iterator en_tr = terrain_cells.end();

		m_context->set_w( math::float4x4().identity() );

		for( ; it_tr != en_tr; ++it_tr)
			it_tr->visual->render();

	}

//////////////////////////////////////////////////////////////////////////


	model_manager::ref().select_visuals( m_context->get_view_pos(), m_context->get_vp()/*m_context->m_full_xform*/, m_visuals);
	vector<render_visual*>::iterator	it  = m_visuals.begin(), 
										end = m_visuals.end();

	for ( ; it!=end; ++it)
	{
		render_visual* visual = *it;
		//ASSERT( visual);
		if( !visual)
			continue;

		if ( !visual->m_effect)
			continue;

		key_rq key;
		res_pass const* pass = visual->m_effect->get_technique(0)->get_pass(0);
		queue_item item = {pass, visual, 0};

		key.priority = pass->get_priority();
		m_static_rq.add_dip( key, item);
	}

// --Porting to DX10_
// 	//CHK_DX( HW.pDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, FALSE));
// 	backend::ref().set_stencil( TRUE, D3D_COMPARISON_ALWAYS, 0x01, 0xff, 0xff, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP);
// 	backend::ref().set_cull_mode( D3DCULL_CCW);
// 	backend::ref().set_color_write_enable();

	m_static_rq.render( true);
	m_visuals.clear();
}

} // namespace render
} // namespace xray

