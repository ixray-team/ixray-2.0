////////////////////////////////////////////////////////////////////////////
//	Created		: 04.06.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef CELL_H_INCLUDED
#define CELL_H_INCLUDED

#include "object.h"
#include <xray/render/base/visual.h>

namespace stalker2 {


class cell_manager;

struct cell_key
{
	typedef fixed_string<16> fixed_string16;

			explicit	cell_key	( int kx=0x7fffffff, int kz=0x7fffffff ):x(kx),z(kz)	{}
	static cell_key		pick		( float3 const& point );
	static cell_key		parse		( pcstr key_str );
	fixed_string16		to_string	( ) const;
	bool				operator <	( cell_key const& other ) const		{return (x==other.x)?(z<other.z):(x<other.x); }
	bool				operator ==	( cell_key const& other ) const		{return (x==other.x) && (z==other.z); }
	bool				valid		( ) const {return x!=0x7fffffff && z!=0x7fffffff; }
	int x;
	int z;
}; // struct key

enum neighbor_enum
{
	n_left_top		=0,
	n_top,
	n_right_top,
	n_right,
	n_right_bottom,
	n_bottom,
	n_left_bottom,
	n_left,
	n_neighbor_count
};


class game_cell :	public resources::unmanaged_resource,
					public boost::noncopyable
{
public:
						game_cell						( );
	//virtual void		recalculate_memory_usage_impl	( ) { m_memory_usage_self.unmanaged = get_size(); }
	void				make_quality_optimal			( );
	void				decrease_quality				( );
	void				load_contents					( );
	bool				contents_loaded					( ) const;
	
	configs::lua_config_ptr			m_config;
	bool							m_quality;
	game*							m_game;

	cell_key			m_neighbors [n_neighbor_count];
private:
	void				make_quality_optimal_impl		( );
	void				decrease_quality_impl			( );
	void				load_terrain					( );
	void				unload_terrain					( );
	void				on_contents_loaded				(  resources::queries_result& data );
	void				on_terrain_visual_ready			( resources::queries_result& data );
	//object_list
	typedef vector<game_object_ptr>	object_list;
	object_list						m_objects;
	xray::render::visual_ptr		m_terrain;
}; // class game_cell

typedef	xray::resources::resource_ptr<
			game_cell,
			resources::unmanaged_intrusive_base
		> game_cell_ptr;


struct cell_cooker_user_data
{
	cell_key					key;
	configs::lua_config_ptr		config;
};

} // namespace stalker2

#endif // #ifndef CELL_H_INCLUDED