////////////////////////////////////////////////////////////////////////////
//	Created		: 27.05.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "cell_manager.h"
#include "game_world.h"
#include "object.h"
#include "game.h"
#include <xray/render/base/world.h>
#include <xray/render/base/game_renderer.h>
#include <xray/render/base/debug_renderer.h>

namespace stalker2 {

cell_manager::cell_manager( game_world& w )
:m_game_world	( w )
{
}

void cell_manager::set_inv_view_matrix( float4x4 const& inv_view_matrix )
{
	m_camera_position	= inv_view_matrix.c.xyz();
	m_camera_direction	= inv_view_matrix.k.xyz();
}

void cell_manager::tick( )
{
	if(!m_cells.empty())
	{
		update_active_keys		( 6 );
	}
	debug_draw				( );
}

bool cell_manager::has_cell( cell_key const& k ) const
{
	return m_cells.find( k ) != m_cells.end();
}

void cell_manager::load( game_project_ptr const& game_project )
{
	m_game_project							= game_project;
	configs::lua_config_value	t_cells		= (*m_game_project->m_config)["cells"];

	if( t_cells.empty() ) // no cells
		return;

	configs::lua_config_iterator it			= t_cells.begin();
	configs::lua_config_iterator it_e		= t_cells.end();

	typedef vector<resources::request>	requests_vec;
	requests_vec						cell_requests;
	cell_requests.resize				( t_cells.size() );
	
	for( u32 idx =0 ;it!=it_e; ++it, ++idx)
	{
		configs::lua_config_value	t_cell	= *it;
		resources::request& q		= cell_requests[idx];

		pcstr cell_name				= *it;
		q.path						= cell_name;
		q.id						= resources::game_cell_class;
	}

	LOG_INFO("===%d cells queried", cell_requests.size());
	resources::query_resources(
		&cell_requests[0],
		cell_requests.size(),
		boost::bind( &cell_manager::game_cell_loaded, this, _1 ),
		g_allocator
		);


}

void cell_manager::game_cell_loaded( resources::queries_result& data )
{
	R_ASSERT				( data.is_successful() );
	u32 count				= data.size();
	LOG_INFO("===%d cells loaded", count);
	for(u32 i=0; i<count; ++i )
	{
		game_cell_ptr cell	= static_cast_resource_ptr<game_cell_ptr>(data[i].get_unmanaged_resource( ) );
		pcstr cell_name		= data[i].get_requested_path( );
		cell_key k			= cell_key::parse( cell_name );
		R_ASSERT			( m_cells.find( k )==m_cells.end( ) );
		m_cells[k]			= cell;
		cell->m_game		= &m_game_world.get_game();
		cell->load_contents	( );
	}
	build_graph				( );
}

void cell_manager::unload( )
{
	m_game_project		= 0;

	cell_container::iterator it		= m_cells.begin();
	cell_container::iterator it_e	= m_cells.end();
	for( ; it!=it_e; ++it)
	{
		game_cell_ptr c			= it->second;
		c->decrease_quality		( );
	}	
	m_cells.clear	( );
}

bool cell_manager::empty( ) const
{
	return m_cells.empty( );
}

void cell_manager::build_graph( )
{
	cell_container::iterator it		= m_cells.begin();
	cell_container::iterator it_e	= m_cells.end();
	for( ; it!=it_e; ++it)
	{
		cell_key k					= it->first;
		game_cell_ptr c				= it->second;
		
		cell_key t = k;
		--t.x; // left
		if(has_cell(t))
			c->m_neighbors[n_left] = t;

		++t.z; // left-top
		if(has_cell(t))
			c->m_neighbors[n_left_top] = t;

		++t.x; // top
		if(has_cell(t))
			c->m_neighbors[n_top] = t;

		++t.x; // right-top
		if(has_cell(t))
			c->m_neighbors[n_right_top] = t;

		--t.z; // right
		if(has_cell(t))
			c->m_neighbors[n_right] = t;

		--t.z; // right-bottom
		if(has_cell(t))
			c->m_neighbors[n_right_bottom] = t;

		--t.x; // bottom
		if(has_cell(t))
			c->m_neighbors[n_bottom] = t;

		--t.x; // left-bottom
		if(has_cell(t))
			c->m_neighbors[n_left_bottom] = t;
	}	
}

void cell_manager::extract_neighbors( game_cell_ptr& cell, keys_container& dest )
{
	for( u32 i=0; i<n_neighbor_count; ++i)
	{
		if(cell->m_neighbors[i].valid())
			dest.push_back( cell->m_neighbors[i] );
	}
}

void cell_manager::update_active_keys( u32 max_depth )
{
	cell_key cam			= cell_key::pick( m_camera_position );

	if(m_current_key == cam)
		return;

	if(!has_cell(cam)) // fix it (get nearest )!!
		return;
	
	m_current_key			= cam;

	keys_container			prev_active_keys;

	m_active_keys.swap		( prev_active_keys );

	keys_container			tmp_keys;
	keys_container			tmp_keys2;

	tmp_keys.push_back		( cam );
	
	for( u32 i=0; i<max_depth; ++i)
	{
		keys_container::iterator it		= tmp_keys.begin();
		keys_container::iterator it_e	= tmp_keys.end();
		for(; it!=it_e; ++it)
		{
			cell_key& k			= *it;
			ASSERT				(has_cell(k));
			game_cell_ptr& cell = m_cells[k];
			extract_neighbors	( cell, tmp_keys2 );
		}

		std::sort						( tmp_keys2.begin(), tmp_keys2.end() );
		keys_container::iterator end_it = std::unique( tmp_keys2.begin(), tmp_keys2.end() );
		tmp_keys2.erase					( end_it, tmp_keys2.end() );

		ASSERT(tmp_keys2.size()<=m_cells.size());
		
		m_active_keys.insert			( m_active_keys.end(), tmp_keys.begin(), tmp_keys.end() );
		std::sort						( m_active_keys.begin(), m_active_keys.end() );
		end_it = std::unique			( m_active_keys.begin(), m_active_keys.end() );
		m_active_keys.erase				( end_it, m_active_keys.end() );
		ASSERT(m_active_keys.size()<=m_cells.size());

		tmp_keys.swap					( tmp_keys2 );
		tmp_keys2.clear					( );
	}

	keys_container					loading_queue;
	keys_container					unloading_queue;

	keys_container::iterator it		= prev_active_keys.begin	( );
	keys_container::iterator it_e	= prev_active_keys.end		( );
	
	for( ; it!=it_e; ++it)
	{
		cell_key& k			= *it;
		bool is_active_now	= (m_active_keys.end() != std::find(m_active_keys.begin(), m_active_keys.end(), k) );

		if( !is_active_now )
			unload_cell( k );
	}
	
	it		= m_active_keys.begin	( );
	it_e	= m_active_keys.end		( );
	
	for( ; it!=it_e; ++it)
	{
		cell_key& k			= *it;
		bool was_active		= (prev_active_keys.end() != std::find(prev_active_keys.begin(), prev_active_keys.end(), k) );

		if( !was_active )
			load_cell( k );
	}
}

bool cell_manager::load_cell( cell_key const& k )
{
	game_cell_ptr& c		= m_cells[ k ];
	c->make_quality_optimal( );

	return true;
}

bool cell_manager::unload_cell( cell_key const& k )
{
	game_cell_ptr& c	= m_cells[ k ];
	c->decrease_quality	( );

	return true;
}

void cell_manager::debug_draw( )
{
	//return;
	cell_container::iterator it		= m_cells.begin();
	cell_container::iterator it_e	= m_cells.end();
	math::color						active_clr(1.0f, 1.0f, 1.0f, 1.0f);
	math::color						inactive_clr(0.0f, 0.0f, 1.0f, 1.0f);
	for( ; it!=it_e; ++it)
	{
		cell_key k					= it->first;
		bool bactive				= m_active_keys.end()!=std::find( m_active_keys.begin(), m_active_keys.end(), k );

		debug_draw_key				( k, bactive?active_clr:inactive_clr );
	}
	debug_draw_key				( m_current_key, math::color(1.0f, 0.0f, 0.0f, 1.0f ) );

}

void cell_manager::debug_draw_key( cell_key& k, math::color const& clr )
{
	xray::render::debug::renderer& renderer = m_game_world.get_game().render_world().game().debug();
	float4x4 m;
	m.identity();
	m.c.xyz() = float3( k.x*64.0f+32, 0.0f, k.z*64.0f-32 );
	renderer.draw_rectangle	( m, float3(32.0f, 1, 32.0f), clr );
	
}

} // namespace stalker2