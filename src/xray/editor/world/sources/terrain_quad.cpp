////////////////////////////////////////////////////////////////////////////
//	Created		: 16.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_quad.h"
#include "terrain_object.h"

namespace xray {
namespace editor {

bool terrain_quad::setup(u16 id, terrain_node^ o)
{
	owner			= o;
	quad_id			= id;
	u32	dim_w		= owner->m_dimension;
	// fill indises here
	int _y			= id / dim_w;
	int _x			= id - dim_w*_y;

	index_lt		= u16( (dim_w+1)*_y + _x);
	index_lb		= u16(index_lt + dim_w + 1);
	index_rt		= index_lt + 1;
	index_rb		= index_lb + 1;

	return true;
}

terrain_vertex^ terrain_quad::vertex(u16 index) 
{
	return owner->m_vertices[index];
}

// xz plane distance, height component is ignored
float terrain_quad::min_distance_to_sqr(float3 const& point)
{
	float result					= math::float_max;

	float const dist_xz_sqr_lt		= owner->distance_xz_sqr(index_lt, point);
	float const dist_xz_sqr_lb		= owner->distance_xz_sqr(index_lb, point);
	float const dist_xz_sqr_rt		= owner->distance_xz_sqr(index_rt, point);
	float const dist_xz_sqr_rb		= owner->distance_xz_sqr(index_rb, point);

	result 							= math::min(result, dist_xz_sqr_lt);
	result 							= math::min(result, dist_xz_sqr_lb);
	result 							= math::min(result, dist_xz_sqr_rt);
	result 							= math::min(result, dist_xz_sqr_rb);

	return 							math::sqrt(result);
}

void terrain_quad::export_lines(xray::vectora<u16>& dest_indices, u32& idx)
{
	int	dim					= owner->m_dimension;

	int _y					= quad_id / dim;
	int _x					= quad_id - dim*_y;

	dest_indices[idx++]		= index_lt;
	dest_indices[idx++]		= index_rt;

	dest_indices[idx++]		= index_rt;
	dest_indices[idx++]		= index_lb;

	dest_indices[idx++]		= index_lb;
	dest_indices[idx++]		= index_lt;

	if(_x==dim-1)
	{
		dest_indices[idx++]		= index_rt;
		dest_indices[idx++]		= index_rb;
	}
	if(_y==dim-1)
	{
		dest_indices[idx++]		= index_rb;
		dest_indices[idx++]		= index_lb;
	}

}

void terrain_quad::export_tris(xray::vectora<u16>& dest_indices, u32& idx, bool clockwise)
{
	if(clockwise)
	{
	// CW
		dest_indices[idx++]		= index_lb;
		dest_indices[idx++]		= index_lt;
		dest_indices[idx++]		= index_rt;

		dest_indices[idx++]		= index_lb;
		dest_indices[idx++]		= index_rt;
		dest_indices[idx++]		= index_rb;
	}else
	{
	// CCW
		dest_indices[idx++]		= index_rt;
		dest_indices[idx++]		= index_lt;
		dest_indices[idx++]		= index_lb;

		dest_indices[idx++]		= index_rt;
		dest_indices[idx++]		= index_lb;
		dest_indices[idx++]		= index_rb;
	}
}

// collision use 32bit- based index
void terrain_quad::export_tris(xray::vectora<u32>& dest_indices, u32& idx, bool clockwise)
{
	if(clockwise)
	{
	// CW
		dest_indices[idx++]		= index_lb;
		dest_indices[idx++]		= index_lt;
		dest_indices[idx++]		= index_rt;

		dest_indices[idx++]		= index_lb;
		dest_indices[idx++]		= index_rt;
		dest_indices[idx++]		= index_rb;
	}else
	{
	// CCW
		dest_indices[idx++]		= index_rt;
		dest_indices[idx++]		= index_lt;
		dest_indices[idx++]		= index_lb;

		dest_indices[idx++]		= index_rt;
		dest_indices[idx++]		= index_lb;
		dest_indices[idx++]		= index_rb;
	}
}

} // namespace editor
} // namespace xray