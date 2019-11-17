////////////////////////////////////////////////////////////////////////////
//	Created		: 09.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/geometry_utils.h>
#include <xray/geometry_primitives.h>
#include <xray/linkage_helper.h>

#ifndef XRAY_STATIC_LIBRARIES
	DECLARE_LINKAGE_ID(core_geometry_utils)
#endif // #ifndef XRAY_STATIC_LIBRARIES

using xray::math::float4x4;
using xray::math::float3;

namespace xray {
namespace geometry_utils{

void create_primitive(	geom_vertices_type& avertices, 
						 geom_indices_type& aindices, 
						 float4x4 transform, 
						 float const* vertices,  
						 u32 vertex_count, 
						 u16 const* faces, 
						 u32 index_count )
{
	float3 tmp_vertex;
	avertices.resize( vertex_count );
	for( u32 i = 0, j = 0 ; i < vertex_count; ++i, j+=3 )
	{
		tmp_vertex.x = vertices[j];
		tmp_vertex.y = vertices[j+1];
		tmp_vertex.z = vertices[j+2];

		avertices[i] = transform.transform_position( tmp_vertex );
	}

	aindices.resize( index_count );
	for( u32 i = 0; i < index_count; ++i)
		aindices[i] = faces[i];
}

//bool create_torus( debug_vertices_type& vertices, debug_indices_type& indices, float outer_raduius,  float inner_raduius, u16 outer_segments, u16 inner_segments, color color )
// {
//	xray::vectora< float3 > tmp_vertices ( *g_allocator );
//	xray::vectora< u16 > tmp_indices ( *g_allocator );
//
//	bool result = create_torus	( tmp_vertices, tmp_indices, float4x4().identity(), outer_raduius,  inner_raduius, outer_segments, inner_segments );
//
//	vertices.resize( tmp_vertices.size() );
//	for ( u32 i = 0; i < tmp_vertices.size(); ++i)
//	{
//		vertices[i].position = tmp_vertices[i];
//		vertices[i].color = color;
//	}
//
//	indices.resize( tmp_indices.size() );
//	for ( u32 i = 0; i < tmp_indices.size(); ++i)
//		indices[i] = tmp_indices[i];
//
//	return result;
// }

 bool create_torus( geom_vertices_type& vertices, 
					geom_indices_type& indices, 
					float4x4 transform,
					float outer_raduius,  
					float inner_raduius, 
					u16 outer_segments, 
					u16 inner_segments )
{
	return generate_torus( vertices, indices, transform, outer_raduius, inner_raduius, outer_segments, inner_segments );
}

bool create_cylinder( geom_vertices_type& vertices, geom_indices_type& indices, float4x4 transform, float3 size )
{
	 create_primitive	( vertices, 
							indices, 
							create_scale(size)*transform, 
							cylinder_solid::vertices,  
							cylinder_solid::vertex_count, 
							cylinder_solid::faces, 
							cylinder_solid::index_count );
	 return true;
}

bool create_cone( geom_vertices_type& vertices, xray::vectora< u16 >& indices, float4x4 transform, float3 size )
{
	create_primitive	( vertices, 
		indices, 
		create_scale(size)*transform, 
		cone_solid::vertices,  
		cone_solid::vertex_count, 
		cone_solid::faces, 
		cone_solid::index_count );
	return true;
}


bool create_cube( geom_vertices_type& vertices, geom_indices_type& indices, float4x4 transform, float3 size )
{
	create_primitive	( vertices, 
		indices, 
		create_scale(size)*transform, 
		cube_solid::vertices,  
		cube_solid::vertex_count, 
		cube_solid::faces, 
		cube_solid::index_count );
	return true;
}

bool create_ellipsoid( geom_vertices_type& vertices, geom_indices_type& indices, float4x4 transform, float3 size )
{
	create_primitive	( vertices, 
		indices, 
		create_scale(size)*transform, 
		ellipsoid_solid::vertices,  
		ellipsoid_solid::vertex_count, 
		ellipsoid_solid::faces, 
		ellipsoid_solid::index_count );

	return true;
}

bool create_ring( geom_vertices_type& vertices, 
					geom_indices_type& indices, 
					float inner_radius, 
					float width,
					u16 segments_count)
{
	ASSERT					(segments_count>3);
	
	vertices.resize			(segments_count*2);
	indices.resize			(segments_count*6);

	float segment_ang		= math::pi_x2/segments_count;
	u16 i					= 0;
	u32 vidx				= 0;
	float outer_radius		= inner_radius+width;
	for(i=0; i<segments_count;++i)
	{
		math::sine_cosine	sincos(segment_ang*i);
		vertices[vidx++].set(inner_radius*sincos.cosine, 0.0f, -inner_radius*sincos.sine);
		vertices[vidx++].set(outer_radius*sincos.cosine, 0.0f, -outer_radius*sincos.sine);
	}

	u32 iidx				= 0;
	for(i=0; i<segments_count-1; ++i)
	{
		indices[iidx++]		= (i*2);
		indices[iidx++]		= (i*2+1);
		indices[iidx++]		= (i*2+3);

		indices[iidx++]		= (i*2);
		indices[iidx++]		= (i*2+3);
		indices[iidx++]		= (i*2+2);
	}
	{ // last segment
		indices[iidx++]		= (i*2);
		indices[iidx++]		= (i*2+1);
		indices[iidx++]		= (1);

		indices[iidx++]		= (i*2);
		indices[iidx++]		= (1);
		indices[iidx++]		= (0);
	}
	return true;
}

} //namespace geometry_utils
} //namespace xray
