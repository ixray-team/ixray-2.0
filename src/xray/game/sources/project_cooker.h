////////////////////////////////////////////////////////////////////////////
//	Created		: 02.06.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef PROJECT_COOKER_H_INCLUDED
#define PROJECT_COOKER_H_INCLUDED

#include "cell.h"
#include <xray/resources_cook_classes.h>

namespace stalker2 {

class game_project :	public resources::unmanaged_resource,
						public boost::noncopyable
{
public:
	//virtual void				recalculate_memory_usage_impl	( )	{ m_memory_usage_self.unmanaged = get_size(); }
	configs::lua_config_ptr		m_config;

	typedef associative_vector<cell_key, game_cell_ptr, vector>		cell_container;
	cell_container				m_cached_cells; // for editor mode only
}; // class game_project

typedef	xray::resources::resource_ptr<
			game_project,
			resources::unmanaged_intrusive_base
		> game_project_ptr;

class project_cooker	:	public resources::translate_query_cook,
							public boost::noncopyable					
{
	typedef resources::translate_query_cook		super;
public:
								project_cooker			( bool editor_present );

	virtual	void				translate_query			( resources::query_result&	parent );

	virtual void				delete_resource			( resources::unmanaged_resource* resource );

private:
			void				on_game_project_loaded( resources::queries_result& data, 
														  resources::query_result_for_cook* parent_query );

			void				on_editor_project_loaded( resources::queries_result& data, 
														  resources::query_result_for_cook* parent_query );
			
			void				make_game_project		( configs::lua_config_ptr const& editor_project, 
															resources::query_result_for_cook* parent_query );

			void				on_cells_loaded			( resources::queries_result& data,
														  resources::query_result_for_cook* parent_query,
														  game_project* project );

			bool				m_editor_present;
}; // class project_cooker


} // namespace stalker2

#endif // #ifndef PROJECT_COOKER_H_INCLUDED