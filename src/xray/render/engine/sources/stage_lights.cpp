////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright ( C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/core/effect_manager.h>
#include <xray/render/core/resource_manager.h>
#include <xray/render/engine/stage_lights.h>
#include <xray/render/engine/lights_db.h>
#include <xray/render/engine/sources/du_sphere.h>
#include <xray/render/engine/effect_light_spot.h>
#include <xray/render/engine/effect_light_mask.h>
#include <xray/render/engine/vertex_formats.h>
//#include "dx9_world.h"

namespace xray {
namespace render_dx10 {

stage_lights::stage_lights( scene_context* context): stage( context)
{
	effect_light_spot	b_spot;
	m_sh_accum_point = effect_manager::ref().create_effect( &b_spot, "r2\\accum_point_s");

	// vertices

	m_accum_point_vb = resource_manager::ref().create_buffer( DU_SPHERE_NUMVERTEX*sizeof( float3), du_sphere_vertices, enum_buffer_type_vertex, false);

//	u32	dwUsage				= D3DUSAGE_WRITEONLY;
//	{
// 		u32		vCount		= DU_SPHERE_NUMVERTEX;
// 		u32		vSize		= 3*4;
// 		R_CHK( device::ref().device()->CreateVertexBuffer( 
// 			vCount*vSize,
// 			dwUsage,
// 			0,
// 			D3DPOOL_MANAGED,
// 			&m_accum_point_vb,
// 			0));
// 		BYTE*	pData				= 0;
// 		
// 		R_CHK( m_accum_point_vb->Lock( 0,0,( void**)&pData,0));
// 		CopyMemory( pData,du_sphere_vertices,vCount*vSize);
// 		m_accum_point_vb->Unlock	();
//	}

		m_accum_point_ib = resource_manager::ref().create_buffer( DU_SPHERE_NUMFACES*3*sizeof( u16), du_sphere_faces, enum_buffer_type_index, false);
// 	// indices_type
// 	{
// 		u32		iCount		= DU_SPHERE_NUMFACES*3;
// 
// 		BYTE*	pData = 0;
// 		R_CHK( device::ref().device()->CreateIndexBuffer( iCount*2,dwUsage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&m_accum_point_ib,0));
// 		R_CHK( m_accum_point_ib->Lock( 0,0,( void**)&pData,0));
// 		CopyMemory( pData,du_sphere_faces,iCount*2);
// 		m_accum_point_ib->Unlock	();
// 	}

	D3D_INPUT_ELEMENT_DESC desc[] = 
	{
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D_INPUT_PER_VERTEX_DATA, 0}	
	};

	m_g_accum_point = resource_manager::ref().create_geometry( desc, sizeof(float3), &*m_accum_point_vb, &*m_accum_point_ib);

	m_c_ldynamic_pos	= backend::ref().register_constant_host( "Ldynamic_pos");
	m_c_ldynamic_color	= backend::ref().register_constant_host( "Ldynamic_color");
	m_c_texgen			= backend::ref().register_constant_host( "Ldynamic_pos");

}

stage_lights::~stage_lights()
{
//#ifdef DEBUG
//	_SHOW_REF( "g_accum_point_ib",m_g_accum_point_ib);
//#endif // DEBUG
//	safe_release( m_accum_point_ib);
//#ifdef DEBUG
//	_SHOW_REF( "g_accum_point_vb",m_g_accum_point_vb);
//#endif // DEBUG
//	safe_release( m_accum_point_vb);
}

void stage_lights::execute()
{
	PIX_EVENT( stage_lights);

	backend::ref().set_render_targets	( &*m_context->m_rt_accumulator, 0, 0); 
	backend::ref().reset_depth_stencil_target();

/////////////// Editor lights rendering //////////////////////////////////

	lights_db::editor_lights&	e_lights = lights_db::ref().get_editor_lights();

	lights_db::editor_lights::iterator	e_it  = e_lights.begin(),
										e_end = e_lights.end();

	for ( ; e_it!=e_end; ++e_it)
	{
		accum_point( &*( e_it->light));
	}

//////////////////////////////////////////////////////////////////////////

	vector<ref_light>& lights = lights_db::ref().get_lights();

	vector<ref_light>::iterator	it  = lights.begin(),
								end = lights.end();

	for ( ; it!=end; ++it)
	{
		accum_point( &**it);
	}

	m_context->set_w_identity();
}

void stage_lights::accum_point( light* l)
{
	increment_light_marker();

	backend&	be = backend::ref();
	//phase_accumulator				();
	//RImplementation.stats.l_visible	++;

	//ref_effect		shader			= L->s_point;
	//if ( !shader)	shader			= m_sh_accum_point;
	
	ref_effect	shader = m_sh_accum_point;

	//float4x4 Pold;//=Fidentity;
	//Pold.identity();
	//float4x4 FTold = m_context->m_full_xform;//=Fidentity;

	//if ( L->flags.bHudMode)
	//{
	//	extern ENGINE_API float		psHUD_FOV;
	//	Pold				= Device.mProject;
	//	FTold				= Device.mFullTransform;
	//	Device.mProject.build_projection( 
	//		deg2rad( psHUD_FOV*Device.fFOV /* *Device.fASPECT*/), 
	//		Device.fASPECT, VIEWPORT_NEAR, 
	//		g_pGamePersistent->Environment().CurrentEnv->far_plane);

	//	Device.mFullTransform.mul	( Device.mProject, Device.mView);
	//	be.set_xform_project	( Device.mProject);
	//	RImplementation.rmNear		();
	//}

	// Common
	float3		L_pos;
	float		L_spec;
	float		L_R					= l->range*0.95f;
	float3		L_clr;				L_clr.set		( l->color.r,l->color.g,l->color.b);
	L_spec	=	utils::u_diffuse2s	( L_clr);
	L_pos	= m_context->get_v().transform_position( l->position);

	// Xforms
	l->xform_calc();
	m_context->set_w( l->m_xform);
	
	//be.set_xform_world			( l->m_xform);
	//be.set_xform_view			( Device.mView);
	//be.set_xform_project		( Device.mProject);
	//enable_scissor					( L);
	//enable_dbt_bounds				( L);

	// *****************************	Mask by stencil		*************************************
	// *** similar to "Carmack's reverse", but assumes convex, non intersecting objects,
	// *** thus can cope without stencil clear with 127 lights
	// *** in practice, 'cause we "clear" it back to 0x1 it usually allows us to > 200 lights :)

	// --Porting to DX10_
	// removed as the effect overwrites this state.
	//be.set_z( TRUE);
	m_context->m_sh_accum_mask->apply( effect_light_mask::tech_mask_point, 0);			// masker

	// --Porting to DX10_
	// moved to effect
	//be.set_color_write_enable( FALSE);

	// backfaces: if ( stencil>=1 && zfail)	stencil = light_id
	
	// --Porting to DX10_
	// moved to effect
	//be.set_cull_mode( D3DCULL_CW);
	//be.set_stencil( TRUE, D3D_COMPARISON_LESS_EQUAL, m_context->m_light_marker_id, 0x01, 0xff, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP);
	be.set_stencil_ref( m_context->m_light_marker_id);
	
	draw_sphere( l); //////////////////////////////////////////////////////////////////////////

	m_context->m_sh_accum_mask->apply( effect_light_mask::tech_mask_point, 1);

	//// frontfaces: if ( stencil>=light_id && zpass)	stencil = 0x1
	// --Porting to DX10_
	//be.set_cull_mode( D3DCULL_CCW);

											//       | 
											// this  V need to be changed to a constant or variable 
// --Porting to DX10_
// 	be.set_stencil( TRUE,D3D_COMPARISON_LESS_EQUAL,0x01,0xff,0xff,D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP);
// 	be.set_stencil_ref( 0x01);
	draw_sphere( l);

	//// nv-stencil recompression
	//if ( RImplementation.o.nvstencil)		u_stencil_optimize();

	// *****************************	Minimize overdraw	*************************************
	// Select shader ( front or back-faces), *** back, if intersect near plane
// --Porting to DX10_
// 	be.set_color_write_enable();
// 	be.set_cull_mode( D3DCULL_CW);		// back

	/*
	if ( bIntersect)	be.set_CullMode		( CULL_CW);		// back
	else			be.set_CullMode		( CULL_CCW);		// front
	*/

	// 2D texgens 
	float4x4			m_texgen;			u_compute_texgen_screen	( m_texgen	);

	// Draw volume with projective texgen
	{
		// Select shader
		u32		_id					= 0;
		//if ( L->flags.bShadow)		{
		//	bool	bFullSize			= ( L->X.S.size == u32( RImplementation.o.smapsize));
		//	if ( L->X.S.transluent)	_id	= SE_L_TRANSLUENT;
		//	else if		( bFullSize)	_id	= SE_L_FULLSIZE;
		//	else					_id	= SE_L_NORMAL;
		//} else {
		//	_id						= SE_L_UNSHADOWED;
		//	//m_Shadow				= m_Lmap;
		//}

		_id = effect_light_spot::tech_unshadowed/*SE_L_NORMAL*/;

		float4x4	ident;
		ident.identity();
		//get_w().set_world_matrix( ident);
		shader->apply( _id);

		// Constants
		be.set_constant	( m_c_ldynamic_pos,	float4(L_pos.x,L_pos.y,L_pos.z,1/( L_R*L_R)));
		be.set_constant	( m_c_ldynamic_color,	float4(L_clr.x,L_clr.y,L_clr.z,L_spec));
		//!!!!!!!!!!!! FIX THIS !!!!!!!!!!!!!!!!!!!! trashes dt_scaler!!! is it even working?????
		be.set_constant	( m_c_texgen, m_texgen);

//		// Fetch4 : enable
//		if ( RImplementation.o.HW_smap_FETCH4)	{
//			//. we hacked the shader to force smap on S0
//#			define FOURCC_GET4  MAKEFOURCC( 'G','E','T','4') 
//			HW.pDevice->SetSamplerState	( 0, D3DSAMP_MIPMAPLODBIAS, FOURCC_GET4);
//		}

		// Render if ( stencil >= light_id && z-pass)
		// --Porting to DX10_
		//moved to the effect
		//be.set_stencil( TRUE, D3D_COMPARISON_LESS_EQUAL,m_context->m_light_marker_id,0xff,0x00,D3D_STENCIL_OP_KEEP,D3D_STENCIL_OP_KEEP,D3D_STENCIL_OP_KEEP);

		m_g_accum_point->apply();
		be.set_stencil_ref( m_context->m_light_marker_id);
		draw_sphere( l);

//		// Fetch4 : disable
//		if ( RImplementation.o.HW_smap_FETCH4)	{
//			//. we hacked the shader to force smap on S0
//#			define FOURCC_GET1  MAKEFOURCC( 'G','E','T','1') 
//			HW.pDevice->SetSamplerState	( 0, D3DSAMP_MIPMAPLODBIAS, FOURCC_GET1);
//		}
	}

	//// blend-copy
	//if ( !RImplementation.o.fp16_blend)	{
	//	u_setrt						( rt_Accumulator,NULL,NULL,HW.pBaseZB);
	//	be.set_Element			( s_accum_mask->E[SE_MASK_ACCUM_VOL]	);
	//	be.set_constant				( "m_texgen",		m_Texgen);
	//	draw_volume					( L);
	//}

	//R_CHK( HW.pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE,FALSE));

	//dwLightMarkerID					+=	2;	// keep lowest bit always setted up

	//u_DBT_disable				();



	//if ( L->flags.bHudMode)
	//{
	//	RImplementation.rmNormal					();
	//	// Restore projection
	//	Device.mProject				= Pold;
	//	Device.mFullTransform		= FTold;
	//	be.set_xform_project	( Device.mProject);
	//}

	//m_context->m_full_xform = FTold;
}

void stage_lights::draw_sphere( light*)
{
	backend::ref().render_indexed( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, DU_SPHERE_NUMFACES*3, 0, 0);
}

void stage_lights::reset_light_marker( bool bResetStencil)
{
	backend&	be = backend::ref();
	m_context->m_light_marker_id = stencil_light_marker_start;
	
	if ( bResetStencil)
	{
		u32		offset;
		
		float	_w	= float( device::ref().get_width());
		float	_h	= float( device::ref().get_width());
		u32		C	= utils::color_rgba( 255,255,255,255);
		float	eps	= math::epsilon_3;
		
		vertex_formats::TL* pv	= be.vertex.lock<vertex_formats::TL>( 4, offset);
		pv->set( eps,				float( _h+eps),	eps,	1.f, C, 0, 0);	pv++;
		pv->set( eps,				eps,			eps,	1.f, C, 0, 0);	pv++;
		pv->set( float( _w+eps),	float( _h+eps),	eps,	1.f, C, 0, 0);	pv++;
		pv->set( float( _w+eps),	eps,			eps,	1.f, C, 0, 0);	pv++;
		be.vertex.unlock();
		
		//RCache.set_Stencil	( TRUE,D3D_COMPARISON_ALWAYS,dwLightMarkerID,0x00,0xFF, D3DSTENCILOP_ZERO, D3DSTENCILOP_ZERO, D3DSTENCILOP_ZERO);
		m_context->m_sh_accum_mask->apply( effect_light_mask::tech_mask_stencil);
		m_context->m_g_quad_uv->apply();
		be.render_indexed( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 2*3, 0, offset);

/*
		u32		Offset;
		float	_w					= float( Device.dwWidth);
		float	_h					= float( Device.dwHeight);
		u32		C					= utils::color_rgba	( 255,255,255,255);
		float	eps					= 0;
		float	_dw					= 0.5f;
		float	_dh					= 0.5f;
		FVF::TL* pv					= ( FVF::TL*) RCache.Vertex.Lock	( 4,g_combine->vb_stride,Offset);
		pv->set						( -_dw,		_h-_dh,		eps,	1.f, C, 0, 0);	pv++;
		pv->set						( -_dw,		-_dh,		eps,	1.f, C, 0, 0);	pv++;
		pv->set						( _w-_dw,	_h-_dh,		eps,	1.f, C, 0, 0);	pv++;
		pv->set						( _w-_dw,	-_dh,		eps,	1.f, C, 0, 0);	pv++;
		RCache.Vertex.Unlock		( 4,g_combine->vb_stride);
		RCache.set_Element			( s_occq->E[2]	);
		RCache.set_Geometry			( g_combine		);
		RCache.Render				( D3DPT_TRIANGLELIST,Offset,0,4,0,2);
*/
	}
}

void stage_lights::increment_light_marker()
{
#pragma message( RENDER_TODO( "literal \"2\" need to be a constant depending on bits given for light marking."))

	m_context->m_light_marker_id += 2;

	//if ( dwLightMarkerID>10)
	if ( m_context->m_light_marker_id>255)
		reset_light_marker( true);
}

} // namespace render
} // namespace xray
