////////////////////////////////////////////////////////////////////////////
//	Created		: 27.05.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef CELL_MANAGER_H_INCLUDED
#define CELL_MANAGER_H_INCLUDED

#include "cell.h"
#include "project_cooker.h"

namespace stalker2 {

class game_world;
class terrain_manager;

class cell_manager :public boost::noncopyable
{

public:
									cell_manager		( game_world& w );
	void							load				( game_project_ptr const& game_project );
	void							unload				( );
	void							tick				( );
	void							set_inv_view_matrix	( float4x4 const& inv_view_matrix );
	game_world& 					get_game_world		( ) const		{ return m_game_world; }
	bool							empty				( ) const;
private:
	game_world&						m_game_world;
	game_project_ptr				m_game_project;

	float3							m_camera_position;
	float3							m_camera_direction;
	cell_key						m_current_key;

	typedef associative_vector<cell_key, game_cell_ptr, vector>	cell_container;
	cell_container					m_cells;

	typedef vector<cell_key>		keys_container;

	keys_container					m_active_keys;

	void							update_active_keys	( u32 max_depth );

	void							game_cell_loaded	( resources::queries_result& data );

	bool							has_cell			( cell_key const& k ) const;
	bool							load_cell			( cell_key const& k );
	bool							unload_cell			( cell_key const& k );
	void							build_graph			( );
	void							extract_neighbors	( game_cell_ptr& cell, keys_container& dest );
	void							debug_draw			( );
	void							debug_draw_key		( cell_key& k, math::color const& clr );
}; // class cell_manager

} // namespace stalker2

#endif // #ifndef CELL_MANAGER_H_INCLUDED