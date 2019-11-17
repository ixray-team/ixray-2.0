////////////////////////////////////////////////////////////////////////////
//	Created		: 04.06.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "cell.h"
#include "cell_manager.h"
#include "game_world.h"
#include "object.h"
#include "game.h"
#include <xray/render/base/game_renderer.h>
#include <xray/render/base/world.h>

namespace stalker2 {

#define cell_size (64.0f)

cell_key cell_key::pick( float3 const& point )
{
	cell_key			result;
	result.x			= (int)((point.x) / cell_size) -1;
	result.z			= (int)((point.z) / cell_size);
	if(point.z>0.0f)	++result.z;
	if(point.x>0.0f)	++result.x;

	return result;
}

cell_key cell_key::parse( pcstr key_str )
{
	cell_key			result;
	int offset = 0;
	pcstr c = strchr(key_str, '/');
	if( c )
		offset = (int)(c - key_str) + 1;

	int count			= XRAY_SSCANF(key_str+offset, "%d_%d", &result.x, &result.z);
	R_ASSERT			(count == 2);
	return				result;
}

cell_key::fixed_string16 cell_key::to_string( ) const
{
	fixed_string16		result;
	result.assignf		( "%d_%d", x, z );
	return				result;
}

game_cell::game_cell()
:m_quality	( false ),
m_game		( NULL )
{}

void game_cell::make_quality_optimal( )
{
	if( m_quality ) // already
		return;
	
	m_quality = true;

	if( contents_loaded() )
		make_quality_optimal_impl	( );

	load_terrain					( );
}

void game_cell::decrease_quality( )
{
	if( !m_quality ) // already
		return;
	
	m_quality = false;

	if( contents_loaded() )
		decrease_quality_impl		( );

	unload_terrain					( );
}

void game_cell::make_quality_optimal_impl( )
{
//	return;
	R_ASSERT					( m_quality );
	object_list::iterator it	= m_objects.begin();
	object_list::iterator it_e	= m_objects.end();
	
	for(; it!=it_e; ++it)
	{
		game_object_ptr& o		= *it;
		o->make_quality_optimal	( );
	}
}

void game_cell::decrease_quality_impl( )
{
//	return;
	object_list::iterator it	= m_objects.begin();
	object_list::iterator it_e	= m_objects.end();
	
	for(; it!=it_e; ++it)
	{
		game_object_ptr& o		= *it;
		o->decrease_quality		( );
	}

	m_quality					= false;
}

void game_cell::load_contents( )
{
	configs::lua_config_value t_objects		= (*m_config)["objects"];
	if(!t_objects.empty())
	{
		
		configs::lua_config_iterator it			= t_objects.begin();
		configs::lua_config_iterator it_e		= t_objects.end();

		vector<resources::creation_request>		objects_requests;
		objects_requests.resize					( t_objects.size() );

		for(u32 i=0; it!=it_e; ++it, ++i)
		{
			configs::lua_config_value* t_object	= NEW(configs::lua_config_value)(*it);
			pcstr object_name					= (*t_object)["name"];
			resources::creation_request& r		= objects_requests[i];
			r.name								= object_name;
			r.id								= resources::game_object_class;
			r.data								= const_buffer( (void*)t_object, sizeof(configs::lua_config_value*) );
		}
		resources::query_create_resources(
			&objects_requests[0],
			objects_requests.size(),
			boost::bind( &game_cell::on_contents_loaded, this, _1 ),
			g_allocator
			);
	}
}

void game_cell::on_contents_loaded(  resources::queries_result& data )
{
	R_ASSERT		( data.is_successful() );
	u32 count		= data.size();

	for( u32 i=0; i<count; ++i)
	{
		resources::query_result_for_user& q = data[i];
		const_buffer user_data			= q.creation_data_from_user();

		configs::lua_config_value* ud	= (configs::lua_config_value*)user_data.c_ptr();

		game_object_ptr object_ptr		= static_cast_resource_ptr<game_object_ptr>(q.get_unmanaged_resource());
		m_objects.push_back				( object_ptr );
		DELETE							( ud );
	}

	if( m_quality )
		make_quality_optimal_impl();
}

bool game_cell::contents_loaded( ) const
{
	return !m_objects.empty();
}

void game_cell::unload_terrain( )
{
	if(m_terrain)
	{
		m_game->render_world().game().terrain_remove_cell( m_terrain );
		m_terrain					= 0;
	}
}

void game_cell::load_terrain( )
{
	if(m_terrain) // already
		return;

	configs::lua_config_value t_terrain		= (*m_config)["terrain"];
	
	if(!t_terrain.empty())
	{
		int px				= t_terrain["px"];
		int pz				= t_terrain["pz"];

		fs::path_string		query_path;
		query_path.assignf	("terrain/%d_%d.terr", px, pz);

		pcstr project_name				= m_game->project_path().c_str();
		resources::user_data_variant	v;
		v.set							( project_name );

		resources::query_resource(
			query_path.c_str(),
			resources::terrain_cell_class,
			boost::bind(&game_cell::on_terrain_visual_ready, this, _1),
			g_allocator,
			&v );
	}
}

void game_cell::on_terrain_visual_ready( resources::queries_result& data )
{
	if(m_quality)
	{
		R_ASSERT				( data.is_successful() );
		m_terrain				= static_cast_resource_ptr<render::visual_ptr>( data[0].get_unmanaged_resource() );

		m_game->render_world().game().terrain_add_cell( m_terrain );
	}
}

} // namespace stalker2