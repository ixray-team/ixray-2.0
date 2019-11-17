////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_object.h"

float3 calc_normal(float3 const& p0, float3 const& p1, float3 const& p2)
{
	float3 v01, v12;
	v01		= p1 - p0 ;
	v12		= p2 - p1 ;
	return cross_product( v01, v12 );
}

namespace xray {
namespace editor {

float3 terrain_node::position(u16 vertex_id)
{
	float3 result;
	vertex_xz		(vertex_id, result.x, result.z);
	result.y		= m_vertices[vertex_id].height;
	return result;
}

int terrain_node::get_layer_for_paint(System::String^ texture_name, float3 const& position_local)
{
	u16 nearest_vertex	= get_nearest_vertex(position_local);
	
	int tid = m_used_textures.IndexOf(texture_name);
	ASSERT(tid!=-1);
	terrain_vertex% v		= m_vertices[nearest_vertex];
	
	// find existing
	if(v.t0_index == tid)
		return 0;

	if(v.t1_index == tid)
		return 1;

	if(v.t2_index == tid)
		return 2;

	// displace layer with minimal influence
	if(v.t0_infl > v.t1_infl)
		return (v.t1_infl > v.t2_infl) ? 2 : 1;
	else
		return (v.t0_infl > v.t2_infl) ? 2 : 0;
}

bool terrain_node::get_quad(terrain_quad% dest_quad, u16 quad_index)
{
	dest_quad.setup		(quad_index, this);
	return				true;
}

bool terrain_node::get_quad(terrain_quad% dest_quad, int x, int y)
{
	ASSERT(x>=0 && x<=m_dimension);
	ASSERT(y>=0 && y<=m_dimension);

	return get_quad		(dest_quad, u16(m_dimension*y + x));
}

u16 terrain_node::get_nearest_vertex(float3 const& position_local)
{
	if(!is_in_bound(position_local))
	{
		u16 idx_lt	= 0;
		u16 idx_rt	= (u16)m_dimension+1;
		u16 idx_lb	= (u16)((m_dimension+1) * m_dimension);
		u16 idx_rb	= (u16)((m_dimension+1) * (m_dimension+1) -1);

		// classify
		bool x_min	= (position_local.x < 0);
		bool z_min	= (-position_local.z < 0);
		bool x_max	= (position_local.x > size);
		bool z_max	= (-position_local.z > size);

		if(x_min && z_min) return idx_lt;

		if(x_max && z_max) return idx_rb;
		
		if(x_max && z_min) return idx_rt;

		if(x_min && z_max) return idx_lb;

		if(x_min) return (u16)((m_dimension+1) * (u16)(-position_local.z / cell_size));

		if(x_max) return (u16)((m_dimension+1) * (int)(-position_local.z/cell_size + 1) - 1);

		if(z_min) return u16(position_local.x / cell_size);
		if(z_max) return idx_lb + u16(position_local.x / cell_size + 1);
	}

	// in bound
	int ix	= (int)(position_local.x / cell_size);
	int iy	= (int)(-position_local.z / cell_size);
	
	if((position_local.x - ix*cell_size)> cell_size/2)
		ix += 1;

	if((-position_local.z - iy*cell_size)> cell_size/2)
		iy += 1;

	return vertex_id(ix, iy);

	//u16 result_vertex_id = 0;
	//float min_dist		= math::float_max;
	//for(u16 idx=0; idx<m_vertices.Count; ++idx)
	//{
	//	float dist = distance_xz_sqr(idx, position_local);
	//	if(dist< min_dist)
	//	{
	//		min_dist			= dist;
	//		result_vertex_id	= idx;
	//	}
	//}
	//ASSERT(vertex_id==result_vertex_id);
	// return result_vertex_id;
}

// slow, full test. need optimization later
void terrain_node::get_vertices_l(float3 const& position_local, float const& radius, modifier_shape /*shape_type*/,  vert_id_list^ dest)
{
	float radius_sqr = radius*radius;

	for(u16 idx=0; idx<m_vertices.Count; ++idx)
	{
		if(distance_xz_sqr(idx, position_local) <= radius_sqr)
				dest->Add		(idx);
	}
}

void terrain_node::get_vertices_g(float3 const& position_global, float const& radius, modifier_shape shape_type,  vert_id_list^ dest)
{
	float4x4					inv_transform;
	inv_transform.try_invert	(get_transform());
	float3 position_local		= inv_transform.transform_position(position_global);
	get_vertices_l				(position_local, radius, shape_type, dest);
}

float terrain_node::get_height_local(float3 const& position_local)
{
	ASSERT(is_in_bound(position_local));

	int _xlt			= math::floor(position_local.x / cell_size);
	int _ylt			= math::floor(-position_local.z / cell_size + math::epsilon_7); // floor(-0.0 == -1) !!!

	u16 index_lt		= vertex_id(_xlt, _ylt);

	if(_xlt==m_dimension || _ylt==m_dimension)
		return m_vertices[index_lt].height;

	u16 index_lb		= u16(index_lt + m_dimension + 1);
	u16 index_rt		= index_lt + 1;

	float3 plb			= position(index_lb);
	float3 vlocal_lb	= position_local - plb;
	float3 v			= position(index_rt) - plb;
	float n				= v.z*vlocal_lb.x - v.x*vlocal_lb.z;
	
	float3	p0, p1, p2;
	p0	= position(index_lb);

	if(n<0)
	{
		p1	= position(index_lt);
		p2	= position(index_rt);
	}else
	{
		u16 index_rb		= index_lb + 1;
		p1	= position(index_rt);
		p2	= position(index_rb);
	}
	float3 tri_n	= calc_normal(p0, p1, p2);
	ASSERT(math::valid(tri_n));

	float a,b,c,d;		// plane ax+by+cz+d = 0

	a = tri_n.x;
	b = tri_n.y;
	c = tri_n.z;
	d				= - (p0).dot_product(tri_n);
	float result	= (-d -a*position_local.x - c*position_local.z) / b;
	ASSERT(math::valid(result));
	return			result;
}

} // namespaxe editor
} // namespaxe xray
