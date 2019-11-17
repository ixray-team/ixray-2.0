////////////////////////////////////////////////////////////////////////////
//	Created		: 07.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_QUAD_H_INCLUDED
#define TERRAIN_QUAD_H_INCLUDED

#include <xray/editor/base/managed_delegate.h>

namespace xray {
namespace editor {

public delegate void command_finished_delegate_t(u32);
typedef qr<u32, command_finished_delegate_t> command_finished_delegate;


public enum class terrain_node_size{tzero=0, t32x32=32, t64x64=64, t128x128=128};
public enum class modifier_shape : int {circle=0, square=1};

ref class terrain_node;

public value struct terrain_node_key
{
	terrain_node_key(int _x, int _z):x(_x), z(_z){}
	bool eq(terrain_node_key other){return x==other.x && z==other.z;}
	int x;
	int z;
}; // struct terrain_node_key

typedef System::Collections::Generic::List<terrain_node_key>			TerrainNodesList;

public value struct terrain_vertex
{
	float		height;
	u8			t0_infl;
	u8			t1_infl;
	u8			t2_infl;

	u8			t0_index;
	u8			t1_index;
	u8			t2_index;

	u32			vertex_color;
//	float2		uvset[3];
}; // terrain_vertex


typedef System::Collections::Generic::List<u16>					vert_id_list;

typedef System::Collections::Generic::Dictionary<terrain_node_key, vert_id_list^>		key_vert_id_dict;

typedef System::Collections::Generic::List<terrain_vertex>								vertex_list;

typedef System::Collections::Generic::Dictionary<u16, terrain_vertex>					vertex_dict;
typedef System::Collections::Generic::Dictionary<terrain_node_key, vertex_dict^>		key_vertex_dict;

typedef System::Collections::Generic::Dictionary<u16, float>							height_diff_dict;
typedef System::Collections::Generic::Dictionary<terrain_node_key, height_diff_dict^>	key_height_diff_dict;

public ref struct update_verts_query
{
	u32				id;
	vert_id_list^	list;
};
typedef System::Collections::Generic::List<update_verts_query^>							update_query_list;

public ref struct terrain_quad 
{
	bool	setup			(u16 id, terrain_node^ o);
	u16		quad_id;
	u16		index_lt;
	u16		index_lb;
	u16		index_rt;
	u16		index_rb;
	terrain_node^		owner;
	void	export_lines	(xray::vectora<u16>& dest, u32& start_idx);
	void	export_tris		(xray::vectora<u16>& dest, u32& start_idx, bool clockwise);
	void	export_tris		(xray::vectora<u32>& dest, u32& start_idx, bool clockwise);
	float	min_distance_to_sqr(float3 const& position);
	terrain_vertex^		vertex(u16 index);

}; // class terrain_quad

public ref struct terrain_import_settings
{
	enum class options{rgba_diffuse, rgb_diffuse, rgb_diffuse_a_heightmap, a_heightmap};
	System::String^		m_source_filename;
	options				m_options;
	bool				m_b_create_cell;
	bool				m_b_stretch_image;
	float				m_height_scale;

}; // heightmap_import_settings

inline float distance_xz(float3 const& v0, float3 const& v1)
{	return math::sqrt( math::sqr((v0).x-v1.x) + math::sqr(v0.z-v1.z) ); }

inline float distance_xz_sqr(float3 const& v0, float3 const& v1)
{	return math::sqr((v0).x-v1.x) + math::sqr(v0.z-v1.z); }

} //namespace editor
} //namespace xray


#endif // #ifndef TERRAIN_QUAD_H_INCLUDED