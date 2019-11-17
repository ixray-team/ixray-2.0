////////////////////////////////////////////////////////////////////////////
//	Created		: 11.05.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "solid_visual_writer.h"
#include <xray/ogf.h>
#include <d3d9types.h>

using namespace xray;

inline u32	color_argb	(u32 a, u32 r, u32 g, u32 b)	{	return ((a&0xff)<<24)|((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff);	}
inline u32	color_rgba	(u32 r, u32 g, u32 b, u32 a)	{	return color_argb(a,r,g,b);		}

MStatus solid_visual_writer::write_render_geometry( memory::writer& w )
{
	MStatus						status;
	m_bbox.invalidate			( );
	render_surfaces_it it		= m_render_surfaces.begin();
	render_surfaces_it it_e		= m_render_surfaces.end();
	for(; it!=it_e; ++it)
	{
		render_surface* s	= *it;
		s->prepare			( m_arglist );
		m_bbox.modify		( s->m_bbox );
	}


	if(m_render_surfaces.size()==1)
	{
		m_render_surfaces[0]->save	( w );
	}else
	{
        ogf_header 			H;
        H.format_version	= xrOGF_FormatVersion;
        H.type				= mt_hierrarchy;
        H.shader_id			= 0;
        H.bb				= m_bbox;
		H.bs				= m_bbox.sphere( );

		w.w_chunk			( ogf_chunk_header, &H, sizeof(H) );

        w.open_chunk		( ogf_children );

		u32 chunk_id		= 0;
		it					= m_render_surfaces.begin();
		for(; it!=it_e; ++it, ++chunk_id)
		{
			render_surface* s	= *it;
			w.open_chunk		( chunk_id );
			s->save				( w );
			w.close_chunk		( );
		}

        w.close_chunk		( ); // ogf_children
	}
	
	return					MStatus::kSuccess;
}

MStatus solid_visual_writer::write_collision_geometry( xray::memory::writer& F )
{
	return m_collision_surface.save( F );
}

D3DVERTEXELEMENT9	decl[] = // 12+4+4+4+8=32
{
	{0, 0,  D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },
	{0, 12, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_NORMAL,	0 },
	{0, 16, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TANGENT,	0 },
	{0, 20, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_BINORMAL,	0 },
	{0, 24, D3DDECLTYPE_SHORT4,		D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	0 },
	D3DDECL_END()
};

MStatus render_surface::save(xray::memory::writer& F)
{
    // Header
	F.open_chunk		(ogf_chunk_header);
    ogf_header			H;
	H.format_version	= xrOGF_FormatVersion;
	H.type				= /*(part->m_SWR.size())?MT_PROGRESSIVE:*/xray::mt_normal;
    H.shader_id			= 0;
	H.bb				= m_bbox;
	H.bs				= m_bbox.sphere( );

    F.w					(&H, sizeof(H));
    F.close_chunk		();

    // Texture
    F.open_chunk		(ogf_texture);
    F.w_stringZ			(m_texture_name.asChar());
    F.w_stringZ			(m_shader_name.asChar());
    F.close_chunk		();


    F.open_chunk		( ogf_chunk_vertices );
	F.w					( decl, sizeof(decl) );

	u32 vertices_count	= m_vertices.size();

	F.w_u32				( vertices_count );
	xray::base_basis	Basis;

//	u32 clr = Fcolor().set(0.0f, 0.5f, 1.0f, 0.0f).get();

	for (u32 idx = 0; idx<vertices_count; ++idx)
	{
		F.w_fvector3		(m_vertices[idx]);		// position (offset)

		// Normal
		{
			float3 N		= m_normals[idx];
			N				+= 1.0f;
			N				*= 0.5f*255.f;

			s32 nx			= math::floor(N.x);				math::clamp(nx,0,255);
			s32 ny			= math::floor(N.y);				math::clamp(ny,0,255);
			s32 nz			= math::floor(N.z);				math::clamp(nz,0,255);
//			s32 cc			= iFloor(oV_c.hemi*255.f);	clamp(cc,0,255);
			u32	uN			= color_rgba(nx, ny, nz, 127);
			F.w_u32			(uN);
		}
			
		float2 uv = m_uvs[idx];
		std::pair<s16,u8> _24u = s24_tc_base(uv.x);
		std::pair<s16,u8> _24v = s24_tc_base(uv.y);

		// Tangent
		{
			Basis.set		(m_tangents[idx]);
			u32	uT			= color_rgba(Basis.x, Basis.y, Basis.z, _24u.second);
			F.w_u32			(uT);
		}

		// Binormal
		{
			Basis.set		(m_binormals[idx]);
			u32	uB			= color_rgba(Basis.x, Basis.y, Basis.z, _24v.second);
			F.w_u32			(uB);
		}

		// TC
		{
			F.w_s16		(_24u.first);
			F.w_s16		(_24v.first);
			F.w_s16		(0); //frac
			F.w_s16		(0); //dummy
		}

	}
    F.close_chunk		();

    // indices
	u32 indices_count	= m_indices.size( ); 
    F.open_chunk		( ogf_chunk_indices );
    F.w_u32				( indices_count );
    F.w					( &m_indices.front(), indices_count * sizeof(u16) );
    F.close_chunk		( ); //ogf_chunk_indices

	return MStatus::kSuccess;
}

MStatus collision_surface::save(xray::memory::writer& F)
{
	// collision
	F.open_chunk		( ogf_chunk_collision_v );
	F.w					(&m_vertices[0], m_vertices.size()*sizeof(float3));
    F.close_chunk		( ); //ogf_chunk_collision_v

	F.open_chunk		( ogf_chunk_collision_i );
	F.w					(&m_indices[0], m_indices.size()*sizeof(u32));
	F.close_chunk		( ); //ogf_chunk_collision_i

	return MStatus::kSuccess;
}