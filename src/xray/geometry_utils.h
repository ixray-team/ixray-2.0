////////////////////////////////////////////////////////////////////////////
//	Created		: 09.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef GEOMETRY_UTILS_H_INCLUDED
#define GEOMETRY_UTILS_H_INCLUDED

namespace xray {
namespace geometry_utils{

typedef xray::vectora< math::float3 >	geom_vertices_type;
typedef xray::vectora< u16 >			geom_indices_type;

//XRAY_CORE_API bool		create_torus	( debug_vertices_type& vertices, debug_indices_type& indices, 		
//												float outer_raduius,  float inner_raduius, u16 outer_segments, u16 inner_segments, color color );
XRAY_CORE_API bool		create_torus	( geom_vertices_type& vertices, geom_indices_type& indices, math::float4x4 transform, 
												float outer_raduius,  float inner_raduius, u16 outer_segments, u16 inner_segments );

XRAY_CORE_API bool		create_cone		( geom_vertices_type& vertices, geom_indices_type& indices, math::float4x4 transform, math::float3 size );

XRAY_CORE_API bool		create_cylinder	( geom_vertices_type& vertices, geom_indices_type& indices, math::float4x4 transform, math::float3 size );

XRAY_CORE_API bool		create_cube		( geom_vertices_type& vertices, geom_indices_type& indices, math::float4x4 transform, math::float3 size );

XRAY_CORE_API bool		create_ellipsoid( geom_vertices_type& vertices, geom_indices_type& indices, math::float4x4 transform, math::float3 size );

XRAY_CORE_API bool		create_ring		( geom_vertices_type& vertices, geom_indices_type& indices, float inner_radius, float width, u16 segments_count );


} // namespace geometry
} // namespace xray

#endif // GEOMETRY_UTILS_H_INCLUDED
