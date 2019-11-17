////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_OBJECT_INLINE_H_INCLUDED
#define TERRAIN_OBJECT_INLINE_H_INCLUDED

namespace xray {
namespace editor {

inline bool terrain_node::is_in_bound(float3 const& position_local)
{
	return ( position_local.x >= 0 && position_local.x <= size &&
			-position_local.z >= 0 && -position_local.z <= size);
}

inline terrain_node_key terrain_core::pick_node(float3 const& point)
{
	terrain_node_key		picked;
	int nsz					= (int)node_size; 
	picked.x				= (int)((point.x) / nsz) -1;
	picked.z				= (int)((point.z) / nsz);
	if(point.z>0.0f)		++picked.z;
	if(point.x>0.0f)		++picked.x;

	return picked;
}

inline float terrain_node::distance_xz_sqr(u16 vertex_id, float3 const& point)
{
	float x,z;
	vertex_xz		(vertex_id, x, z);
	return math::sqr(x-point.x) + math::sqr(z-point.z);
}

inline u16	terrain_node::vertex_id(int const _x_idx, int const _z_idx)
{
	ASSERT(_x_idx>=0 && _x_idx<=m_dimension && _z_idx>=0 && _z_idx<=m_dimension);
	return (u16)((m_dimension+1)*_z_idx + _x_idx);
}

inline void terrain_node::vertex_xz(u16 vertex_id, int& _x_idx, int& _z_idx)
{
	_z_idx			= vertex_id / (m_dimension+1);
	_x_idx			= vertex_id - (m_dimension+1) * _z_idx;
}

inline void terrain_node::vertex_xz(u16 vertex_id, float& _x, float& _z)
{
	int dim		= m_dimension+1;
	int _iz		= vertex_id / dim;
	_x			= cell_size * (vertex_id - dim * _iz);
	_z			= _iz * -cell_size;
}

} // namespace editor
} // namespace xray

#endif // #ifndef TERRAIN_OBJECT_INLINE_H_INCLUDED
