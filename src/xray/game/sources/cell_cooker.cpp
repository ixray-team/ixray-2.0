////////////////////////////////////////////////////////////////////////////
//	Created		: 04.06.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "cell_cooker.h"

namespace stalker2 {

cell_cooker::cell_cooker( )
:super( resources::game_cell_class, reuse_true, use_user_thread_id )
{
}

void cell_cooker::translate_query( resources::query_result& parent )
{
	const_buffer user_data_to_create	= parent.creation_data_from_user();
	
	if( user_data_to_create.size() )
	{
		cell_cooker_user_data* data				= (cell_cooker_user_data*)user_data_to_create.c_ptr();
		configs::lua_config_ptr cell_config_ptr	= data->config;
		
		game_cell* resource				= create_game_cell( cell_config_ptr );

		parent.set_unmanaged_resource	( resource, resources::memory_type_non_cacheable_resource, sizeof(game_cell) );
		parent.finish_query				( result_success );
	}else
	{
		// ready config query 
		fs::path_string			req_path = "resources.converted/projects/";
		req_path.append			( parent.get_requested_path() );
		//"tst/1_2.cell"
		// query config for creation
		resources::query_resource(
									req_path.c_str(),
									resources::config_lua_class,
									boost::bind(&cell_cooker::on_source_config_loaded, this, _1, &parent ),
									g_allocator
								);
	}
}

void cell_cooker::on_source_config_loaded( resources::queries_result& data, resources::query_result_for_cook* parent_query )
{
	R_ASSERT( data.is_successful() );

	resources::query_result_for_user const& result	= data[0];

	configs::lua_config_ptr cell_config_ptr	= static_cast_resource_ptr<configs::lua_config_ptr>( result.get_unmanaged_resource() );
	
	game_cell* resource						= create_game_cell( cell_config_ptr );

	parent_query->set_unmanaged_resource	( resource, resources::memory_type_non_cacheable_resource, sizeof(game_cell) );
	parent_query->finish_query				( result_success );
}

game_cell* cell_cooker::create_game_cell( configs::lua_config_ptr const& config )
{
	game_cell*	resource		= NEW(game_cell)();
	resource->m_config			= config;
	return						resource;
}

void cell_cooker::delete_resource( resources::unmanaged_resource* resource )
{
	DELETE( resource );
}

} // namespace stalker2