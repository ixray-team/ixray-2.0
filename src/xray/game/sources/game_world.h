////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef GAME_WORLD_H_INCLUDED
#define GAME_WORLD_H_INCLUDED

#include "scene.h"

namespace stalker2{

class game;
class cell_manager;
class free_fly_handler;

class game_world :	public scene
{
	typedef			scene				super;
public:
					game_world				( game& w );
	virtual			~game_world				( );
public:
	virtual void	on_activate				( );
	virtual void	on_deactivate			( );
	virtual void	tick					( );

	virtual	bool	on_keyboard_action		( input::world& input_world, input::enum_keyboard key, input::enum_keyboard_action action );

	void			load					( pcstr path );
	void			unload					( );
	bool			empty					( );
	
	game&			get_game				( ) {return m_game;}

	pcstr			get_project_name		( ) const { return m_project_name.c_str(); }
protected:
	void			on_project_files_loaded	( resources::queries_result& data );

private:
	void			rtp_debug_update		( );

private:
	free_fly_handler*						m_free_fly_camera;
	cell_manager*							m_cell_manager;
	fs::path_string							m_project_name;
}; // class game_world

} // namespace stalker2

#endif // #ifndef GAME_WORLD_H_INCLUDED