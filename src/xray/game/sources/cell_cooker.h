////////////////////////////////////////////////////////////////////////////
//	Created		: 04.06.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef CELL_COOKER_H_INCLUDED
#define CELL_COOKER_H_INCLUDED

#include "cell.h"
#include <xray/resources_cook_classes.h>


namespace stalker2 {


class cell_cooker	:	public resources::translate_query_cook,
						public boost::noncopyable					
{
	typedef resources::translate_query_cook		super;
public:
								cell_cooker				( );

	virtual	void				translate_query			( resources::query_result& parent );

	virtual void				delete_resource			( resources::unmanaged_resource* resource );

private:
			void				on_source_config_loaded	( resources::queries_result& data, 
														  resources::query_result_for_cook* parent_query );

			game_cell*			create_game_cell		( configs::lua_config_ptr const& config );
}; // class cell_cooker

} // namespace stalker2

#endif // #ifndef CELL_COOKER_H_INCLUDED