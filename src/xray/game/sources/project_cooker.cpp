////////////////////////////////////////////////////////////////////////////
//	Created		: 02.06.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "project_cooker.h"
#include "cell_manager.h"
#include <xray/resources_fs.h>

namespace stalker2 {

pcstr game_prj_ext			= ".prj";
pcstr editor_prj_ext		= ".xprj";
pcstr resources_converted	= "resources.converted/";
pcstr resources				= "resources/";

void change_substring(fs::path_string& src_and_dest, pcstr what, pcstr to)
{
	fs::path_string	result;
	fs::path_string::size_type pos	= src_and_dest.find( what );

	if(pos!=src_and_dest.npos)
	{
		u32 what_len		= strings::length( what );

		result.assign		(src_and_dest.begin(), src_and_dest.begin()+pos); // head
		result.append		( to ); // body
		result.append		( src_and_dest.begin()+pos+what_len, src_and_dest.end() ); // tail
		src_and_dest		= result;
	}
}

project_cooker::project_cooker( bool editor_present)
:super( resources::game_project_class, reuse_true, use_user_thread_id ),
m_editor_present( editor_present )
{
}

void project_cooker::translate_query( resources::query_result & parent )
{
	// ready config query 
	fs::path_string	req_path			= parent.get_requested_path();
	
	// compare source and converted resource file age
	fs::path_string	editor_proj_path= req_path;
	change_substring				( editor_proj_path, game_prj_ext, editor_prj_ext );
	change_substring				( editor_proj_path, resources_converted, resources );

	fs::path_info					editor_project_info;
									resources::get_path_info_by_logical_path(	&editor_project_info, 
																				editor_proj_path.c_str(), 
																				g_allocator);
	fs::path_info					game_project_info;
	fs::path_info::type_enum res_game= resources::get_path_info_by_logical_path(&game_project_info, 
																				req_path.c_str(), 
																				g_allocator);

	if(	!m_editor_present &&
		res_game==fs::path_info::type_file && // has converted entry
		editor_project_info.file_last_modify_time<game_project_info.file_last_modify_time)
	{// load prj
		resources::query_resource(
									req_path.c_str(),
									resources::config_lua_class,
									boost::bind(&project_cooker::on_game_project_loaded, this, _1, &parent ),
									g_allocator
								);
	}else
	{// load xprj
		resources::query_resource(
									editor_proj_path.c_str(),
									resources::config_lua_class,
									boost::bind(&project_cooker::on_editor_project_loaded, this, _1, &parent ),
									g_allocator
								);
	}
}

void project_cooker::on_game_project_loaded( resources::queries_result& data, resources::query_result_for_cook* parent_query )
{
	R_ASSERT(data.is_successful());

	resources::query_result_for_user const& result	= data[0];

	configs::lua_config_ptr game_proj_ptr	= static_cast_resource_ptr<configs::lua_config_ptr>( result.get_unmanaged_resource() );

	game_project*	resource				= NEW(game_project)();
	resource->m_config						= game_proj_ptr;
	parent_query->set_unmanaged_resource	( resource, resources::memory_type_non_cacheable_resource, sizeof(game_project) );
	parent_query->finish_query				( result_success );
}

void project_cooker::on_editor_project_loaded( resources::queries_result& data, resources::query_result_for_cook* parent_query )
{
	R_ASSERT							( data.is_successful() );

	resources::query_result_for_user const& result	= data[0];

	configs::lua_config_ptr editor_proj_ptr	= static_cast_resource_ptr<configs::lua_config_ptr>( result.get_unmanaged_resource() );

	make_game_project					( editor_proj_ptr, parent_query );
}

fs::path_string extract_project_name( fs::path_string full_name )
{
	//--resources/projects/xxx/project.prj
	// or
	// Untitled Project.prj

	if( full_name.find('/') )
	{
		u32 idx1 = full_name.find( "/projects/" );
		u32 idx2 = full_name.find( '/', idx1+1 );
		u32 idx3 = full_name.find( '/', idx2+1 );
		return full_name.substr(idx2+1, idx3-idx2);
	}else
		return "";
}

typedef associative_vector<cell_key, cell_cooker_user_data*, vector> user_data_cont;

cell_cooker_user_data* get_cell(	user_data_cont& container, 
									cell_key const& k, 
									configs::lua_config_value& cells_root,
									fs::path_string const& cell_prefix)
{
	cell_cooker_user_data* result = NULL;

	user_data_cont::iterator it = container.find( k );

	if( it!=container.end() )
	{ // insert new
		result	= it->second;
	}else
	{
		result				= NEW(cell_cooker_user_data)();
		result->key			= k;
		container[k]		= result;
		fixed_string<128>	buff;
		buff.assignf		("%s%s.cell", cell_prefix.c_str(), k.to_string().c_str());
		result->config		= configs::create_lua_config	( buff.c_str() );
		u32 c				= cells_root.size				( );
		cells_root[c]		= buff.c_str					( );
	}

	return result;
}

void project_cooker::make_game_project( configs::lua_config_ptr const& editor_config, resources::query_result_for_cook* parent_query )
{
	fs::path_string cell_prefix = extract_project_name( parent_query->get_requested_path() );

	game_project*	project				= NEW(game_project)();
	parent_query->set_unmanaged_resource( project, resources::memory_type_unmanaged_resource, sizeof(game_project) );

	configs::lua_config_ptr	game_config	= configs::create_lua_config( parent_query->get_requested_path() );
	project->m_config					= game_config;
	
	configs::lua_config_value game_project_root		= (*game_config);
	configs::lua_config_value editor_project_root	= (*editor_config);

	// startup camera
	game_project_root["camera"].assign_lua_value	(editor_project_root["camera"]);

	//cells subdivision
	configs::lua_config_value cells_root			= game_project_root["cells"];
	configs::lua_config_value t						= editor_project_root["project"]["objects_tree"]["subitems"];
	configs::lua_config_iterator it					= t.begin();
	configs::lua_config_iterator it_e				= t.end();
	
	user_data_cont								tmp_cells;

	for( ;it!=it_e; ++it)
	{
		configs::lua_config_value t_current = *it;
		pcstr lib_name						= t_current["lib_name"];

		if(0==strings::compare(lib_name, "base:terrain_core"))
		{ // process terrain here
			t_current			= t_current["nodes"];
			configs::lua_config_iterator nodes_it	= t_current.begin();
			configs::lua_config_iterator nodes_end	= t_current.end();
			for( ;nodes_it!=nodes_end; ++nodes_it)
			{
				configs::lua_config_value node	= *nodes_it;
				cell_key k						( node["px"], node["pz"] );
				cell_cooker_user_data* cell		= get_cell( tmp_cells, k, cells_root, cell_prefix );

				configs::lua_config_value t_terrain = (*cell->config)["terrain"];
				int count							= t_terrain.size();
				ASSERT								(count==0);
				t_terrain.assign_lua_value			(node);
			}
		}else
		{
			math::aabb object_aabb;
			object_aabb.min		= t_current["aabb_min"];
			object_aabb.max		= t_current["aabb_max"];

			cell_key k_min = cell_key::pick( object_aabb.min );
			cell_key k_max = cell_key::pick( object_aabb.max );

			ASSERT(k_min.x <= k_max.x && k_min.z <= k_max.z);
			
			cell_cooker_user_data*		current_cell = NULL;

			for (int kx=k_min.x; kx<=k_max.x; ++kx)
				for (int kz=k_min.z; kz<=k_max.z; ++kz)
				{
					cell_key			k( kx, kz );
					current_cell		= get_cell( tmp_cells, k, cells_root, cell_prefix );

					configs::lua_config_value t_objects = (*current_cell->config)["objects"];
					int count							= t_objects.size();
					t_objects[count].assign_lua_value(t_current);
					LOG_INFO		("[%d][%d]object %s", k.x, k.z, (pcstr)t_current["name"]);
				}

		}
	}

	if( !tmp_cells.empty() )
	{
		vector< fixed_string<256> >			cell_names;
		vector<resources::creation_request>	cell_requests;
		cell_requests.resize				( tmp_cells.size() );
		cell_names.resize					( tmp_cells.size() );
		user_data_cont::iterator itc		= tmp_cells.begin();
		user_data_cont::iterator itc_e		= tmp_cells.end();

		for( u32 idx=0 ;itc!=itc_e; ++itc,++idx)
		{
			resources::creation_request& r	= cell_requests[idx];
			cell_cooker_user_data* ud		= itc->second;
			cell_key const& k				= itc->first;

			fixed_string<256>& cell_name		= cell_names[idx];
			cell_name.assignf				("%s%s.cell", cell_prefix.c_str(), k.to_string().c_str() );

			r.id							= resources::game_cell_class;
			r.data							= const_buffer( (void*)ud, sizeof(cell_cooker_user_data*) );

			r.name							= cell_name.c_str();
		}

		resources::query_create_resources(
			&cell_requests[0],
			cell_requests.size(),
			boost::bind( &project_cooker::on_cells_loaded, this, _1, parent_query, project ),
			g_allocator
			);
	}else
	{	// no cells, only terrain
		parent_query->finish_query	( result_success );
	}
}

void project_cooker::on_cells_loaded(	resources::queries_result& data,
										resources::query_result_for_cook* parent_query,
										game_project*	project )
{
	R_ASSERT					( data.is_successful() );
	
	bool need_save_to_file		= !m_editor_present;

	parent_query->set_unmanaged_resource	( project, resources::memory_type_non_cacheable_resource, sizeof(game_project) );

	u32 cell_count				= data.size();

	for( u32 i=0; i<cell_count; ++i )
	{
		resources::query_result_for_user& q = data[i];
		const_buffer user_data		= q.creation_data_from_user();

		cell_cooker_user_data* ud	= (cell_cooker_user_data*)user_data.c_ptr();

		game_cell_ptr cell_ptr		= static_cast_resource_ptr<game_cell_ptr>(q.get_unmanaged_resource());
		project->m_cached_cells[ud->key]	= cell_ptr;
		DELETE						( ud );
	}
	
	if( need_save_to_file )
	{
		fs::path_string					path_to_save;
		if ( !parent_query->select_disk_path_from_request_path( &path_to_save ) )
		{
			parent_query->finish_query	(result_error);
			return;
		}

		fs::make_dir_r					( path_to_save.c_str(), false );
		project->m_config->save_as		( path_to_save.c_str() );

		game_project::cell_container::iterator it	= project->m_cached_cells.begin();
		game_project::cell_container::iterator it_e = project->m_cached_cells.end();
		fs::path_string dir_path;
		fs::directory_part_from_path (&dir_path, path_to_save.c_str() );
		for(; it!=it_e; ++it)
		{
				game_cell_ptr p			= it->second;
				fs::path_string	fn;
				fn.assignf				("%s/%s.cell", dir_path.c_str(), it->first.to_string().c_str() );
				p->m_config->save_as	( fn.c_str() );
		}
	}

	parent_query->finish_query	( result_success );
}

void project_cooker::delete_resource( resources::unmanaged_resource * resource )
{
	DELETE( resource );
}

} // namespace stalker2
