////////////////////////////////////////////////////////////////////////////
//	Created 	: 11.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

namespace stalker2 {

class scene;
class game_world;
class game_object;
class main_menu;
class console;
class stats;
class stats_graph;
class project_cooker;
class cell_cooker;
class object_cooker;
class solid_visual_cooker;

class game :
	public xray::engine_user::world,
	private boost::noncopyable
{
public:
							game					(	xray::engine_user::engine& engine,
														xray::input::world& input,
														xray::render::world& render,
														xray::ui::world& ui,
														xray::physics::world& physics,
														xray::rtp::world& rtp,
														xray::animation::world& animation,
														xray::sound::world& sound
													);

	virtual					~game					( );

	virtual	void			enable					( bool value );
	virtual	void			tick					( u32 current_frame_id );
	virtual	void			clear_resources			( );

			void			set_view_matrix			( float4x4 const& view_matrix);
	float4x4 const&			get_inverted_view_matrix( ) const;
			void			set_inverted_view_matrix( float4x4 const& view );

			u32				time_ms					( );

	virtual	void			load					( pcstr path );
	virtual	void			unload					( pcstr );

	fs::path_string const&	project_path			( ) const;

	inline	engine_user::engine& engine				( ) const { return m_engine; }
//	game_resource_manager&	resource_manager		( ) { return *m_game_resource_manager; }
	xray::render::world&	render_world			( )	{ return m_render_world; }
	xray::input::world&		input_world				( )	{ return m_input_world; }
	xray::ui::world&		ui_world				( )	{ return m_ui_world; }
	xray::rtp::world&		rtp						( ) { return m_rtp_world; }

			void			toggle_console			( );

			void			main_menu_close_query	( );
			void			game_world_close_query	( );
			void			exit					( pcstr str );
private:
			void			test					( );
			void			switch_to_scene			( scene* scene );
			void			update_stats			( u32 const current_frame_id );
			void			register_console_commands( );

			void			register_cookers		( );
			void			unregister_cookers		( );
private:
//	game_resource_manager*					m_game_resource_manager;
	float4x4								m_inverted_view;
	float4x4								m_projection;
	stats_graph*							m_fps_graph;
	console*								m_console;
	stats*									m_stats;
	timing::timer							m_timer;
	fs::path_string							m_project_path;

	xray::engine_user::engine&				m_engine;
	xray::input::world&						m_input_world;
	xray::render::world&					m_render_world;
	xray::ui::world&						m_ui_world;
	xray::physics::world&					m_physics_world;
	xray::animation::world&					m_animation_world;
	xray::rtp::world&						m_rtp_world;
	xray::sound::world&						m_sound_world;

	game_world*								m_game_world;
	main_menu*								m_main_menu;
	scene*									m_active_scene;
	float									m_last_frame_time;
	bool									m_enabled;

	project_cooker*			m_project_cooker;
	cell_cooker*			m_cell_cooker;
	object_cooker*			m_object_cooker;
	solid_visual_cooker*	m_solid_visual_cooker;
}; // class game_world

} // namespace stalker2

#endif // #ifndef GAME_H_INCLUDED