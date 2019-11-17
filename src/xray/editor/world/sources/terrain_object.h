////////////////////////////////////////////////////////////////////////////
//	Created		: 07.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_OBJECT_H_INCLUDED
#define TERRAIN_OBJECT_H_INCLUDED

#include "object_base.h"
#include "terrain_quad.h"
#include <xray/render/base/common_types.h>
#include <xray/render/base/world.h>
#include <xray/render/base/visual.h>

namespace xray {
namespace editor {

ref class tool_terrain;
ref class terrain_core;

public ref class terrain_node : public object_base
{ 
	typedef object_base		super;
public:
						terrain_node			( tool_base^ t, terrain_core^ core );
	virtual				~terrain_node			( );
	virtual void		render					( )	override;
	virtual void		load					( configs::lua_config_value const& t ) override;
	virtual void		save					( configs::lua_config_value t ) override;
	virtual void		load_defaults			( ) override;
	virtual	void		set_visible				( bool bvisible) override;
	virtual	void		set_transform			( float4x4 const& ) override		{};
	virtual bool		get_selectable			( ) override						{return false;}
	virtual enum_terrain_interaction get_terrain_interaction( ) override			{return enum_terrain_interaction::tr_free;}

	void				set_transform_internal	( float4x4 const& transform );
	void				generate_mesh			( );
	void				map_uv					( u32 layer_id, float k );

	void				export_vertices			( u16 start_idx, u16 count, vectora<xray::render::terrain_data>& dest_buffer );
	float				get_height_local		( float3 const& position_local );

	int					get_layer_for_paint		( System::String^ texture_name, float3 const& position_local );

			bool		get_quad				( terrain_quad% dest_quad, int x, int y );
			bool		get_quad				( terrain_quad% dest_quad, u16 quad_index );
	inline	void		vertex_xz				( u16 vertex_id, float& _x, float& _z );
	inline	void		vertex_xz				( u16 vertex_id, int& _x_idx, int& _z_idx );
	inline	u16			vertex_id				( int const _x_idx, int const _z_idx );
	inline	float		distance_xz_sqr			( u16 vertex_id, float3 const& point );
	inline	float		distance_xz				( u16 vertex_id, float3 const& point ) {return math::sqrt(distance_xz_sqr(vertex_id, point));}
	inline	float3		position				( u16 vertex_id );
			void		initialize_collision	( );
		terrain_core^	get_terrain_core		( )									{return m_core;}
			void		set_visual				( render::visual_ptr v );
xray::render::visual_ptr	get_visual			( )									{return *m_visual;}
protected:
			u16			get_nearest_vertex		( float3 const& position_local );
	inline	bool		is_in_bound				( float3 const& position_local );
			void		on_vertices_loaded		( resources::queries_result& data );

			terrain_core^				m_core;
	xray::render::visual_ptr*			m_visual;

public:
	void				add_used_texture		(System::String^);
	System::Collections::Generic::List<System::String^>	m_used_textures;
	bool				m_b_load_complete;
	terrain_node_key	m_tmp_key;
	vertex_list			m_vertices;
	int					m_dimension;			// quads array
	void				get_vertices_l			(float3 const& local_position, float const& radius, modifier_shape shape_type,  vert_id_list^ dest_vert_ids);
	void				get_vertices_g			(float3 const& global_position, float const& radius, modifier_shape shape_type,  vert_id_list^ dest_vert_ids);

	[DisplayNameAttribute("Draw wire"), DescriptionAttribute("Draw wire"), CategoryAttribute("general"), 
	ReadOnlyAttribute(false)]
	property bool		draw_wireframe;

	[DisplayNameAttribute("cell size"), DescriptionAttribute("cell size"), CategoryAttribute("general"), 
	ReadOnlyAttribute(true)]
	property float					cell_size;

	[DisplayNameAttribute("size"), DescriptionAttribute("terrain size"), CategoryAttribute("general"), 
	ReadOnlyAttribute(true)]
	property float					size;

}; // class terrain_object

public ref class terrain_core : public object_base
{
	typedef object_base				super;

	typedef System::Collections::Generic::KeyValuePair<terrain_node_key, terrain_node^>	key_value_pair;
	typedef System::Collections::Generic::Dictionary<terrain_node_key, terrain_node^>	TerrainNodesMap;

public:
						terrain_core	( tool_terrain^ tool_terrain);

	virtual void		load			( configs::lua_config_value const& t ) override;
	virtual void		save			( configs::lua_config_value t ) override;
	virtual void		render			( ) override;
	virtual void		load_defaults	( ) override;
	virtual	void		set_transform	( float4x4 const& transform ) override;
	virtual bool		get_selectable	( ) override							{return false;}
	virtual enum_terrain_interaction get_terrain_interaction( )override			{return enum_terrain_interaction::tr_free;}

	virtual	void		set_visible		( bool bvisible ) override;
			void		create_node		( terrain_node_key key, u32 quad_size );
			void		clear_node		( terrain_node_key key );
			void		destroy_all		( );
			void		update			( );
			float		get_height		( float3 const& p );
	inline terrain_node_key pick_node	( float3 const& p );

			void		select_vertices	( float3 const& point, float const radius, modifier_shape t, key_vert_id_dict^ dest_list );

			void		add_texture		( System::String^ texture_name );
			void		change_texture	( System::String^ old_texture_name, System::String^ new_texture_name );
			void		import			( terrain_import_settings^ settings, terrain_node_key lt, terrain_node_key rb );
			void		sync_visual_vertices( terrain_node^ terrain, vert_id_list^ id_list );

public:
	System::Collections::Generic::List<System::String^>		m_textures;
	TerrainNodesMap						m_nodes;

	property terrain_node_size			node_size; // meters

private:
				void	add_node_internal	( terrain_node_key key, terrain_node^ node );
				void	remove_node_internal( terrain_node_key key );
				void	command_update_cb	( u32 arg );
				void	load_node_impl		( terrain_node_key key );
				void	update_vertices_impl( u32 terrain_id );
				void	on_terrain_cell_ready( xray::resources::queries_result& data, u32 terrain_id );

	TerrainNodesList	m_load_queries;
	update_query_list	m_update_queries;

	id_list				m_active_updating_nodes;

	tool_terrain^		m_terrain_tool;
}; // ref class terrain_object

} //namespace editor
} //namespace xray

#include "terrain_object_inline.h"

#endif // #ifndef TERRAIN_OBJECT_H_INCLUDED