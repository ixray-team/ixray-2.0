////////////////////////////////////////////////////////////////////////////
//	Created 	: 21.11.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef ENGINE_WORLD_H_INCLUDED
#define ENGINE_WORLD_H_INCLUDED

#include <xray/core/engine.h>
#include <xray/input/engine.h>
#include <xray/render/base/engine_wrapper.h>
#include <xray/editor/world/engine.h>
#include <xray/ui/engine.h>
#include <xray/physics/engine.h>
#include <xray/network/engine.h>
#include <xray/sound/engine.h>
#include <xray/intrusive_spsc_queue.h>
#include <xray/animation/engine.h>
#include <xray/rtp/engine.h>
#include <xray/engine/engine_user.h>

#if !defined(_MSC_VER)
	typedef pvoid HWND;
#endif // #if !defined(_MSC_VER)

namespace xray {
namespace engine {

typedef boost::function< void () >					logic_thread_callback_type;

class engine_world : 
	public core::engine,
	public input::engine,
	public render::engine::wrapper,
	public xray::engine_user::engine,
	public editor::engine,
	public ui::engine,
	public physics::engine,
	public network::engine,
	public rtp::engine,
	public sound::engine,
	public animation::engine,
	private boost::noncopyable
{
public:
							engine_world			( pcstr command_line, pcstr application, pcstr build_date );
	virtual					~engine_world			( );
			void			initialize				( );
			void			run						( );

	virtual void			set_exit_code			( int exit_code ) { m_exit_code = exit_code; }
	virtual	int				get_exit_code			( ) const { return m_exit_code; }
	virtual	pcstr			get_resource_path		( ) const;
	virtual	pcstr			get_mounts_path			( ) const;
	virtual	pcstr			get_underscore_G_path	( ) const;
	virtual bool			app_is_active			( );
	virtual void			on_app_activate			( );
	virtual void			on_app_deactivate		( );

			bool			command_line_editor		( );
			bool	command_line_no_splash_screen	( );
			bool			setup_engine_user		( xray::engine_user::module_proxy* module_proxy );
public:
	render::world&			render_world			( )	const	{ return *m_render_world;	}
	xray::engine_user::world& engine_user_world		( ) const	{ return *m_engine_user_world; }
	ui::world const&		ui_world				( )	const	{ return *m_ui_world;		}
	input::world&			input_world				( )	const	{ return *m_input_world;	}
	editor::world*			editor_world			( )	const	{ return m_editor;			}
	rtp::world*				rtp_world				( )			{ return m_rtp_world; }
private:
	virtual	void			tick					( );
	virtual	bool			on_before_editor_tick	( );
	virtual	void			on_after_editor_tick	( );
	virtual	void			run_renderer_commands	( );
	virtual	void			draw_frame				( );
	virtual	void			flush_debug_commands	( );
	virtual	void			on_resize				( );
	virtual	void			enable_game				( bool value );
	virtual	void			delete_on_logic_tick	( render::engine::command* command );
	virtual	void			delete_on_editor_tick	( render::engine::command* command );
	virtual	void			run_in_window_thread	( render::engine::window_thread_callback_type const& callback );
	virtual	void			draw_frame_logic		( );
	virtual	void			draw_frame_editor		( );
	virtual	void			exit					( int exit_code );
	virtual	void			enter_editor_mode		( );
	virtual	render::world&	get_renderer_world		( );
	virtual	sound::world&	get_sound_world			( );
	virtual	animation::world& get_animation_world	( );
	virtual	float			get_last_frame_time		( );
	virtual	bool			command_line_editor_singlethread	( );

	virtual	void 			load_level				( pcstr project_name );
	virtual	void 			unload_level			( );


private:
			void			initialize_core			( );
			void			initialize_editor		( );
			void			initialize_render		( );
			void			initialize_logic		( );
			void			post_initialize			( );
			void			enable_game_impl		( bool const value );

private:
			void			initialize_input		( );
			void			initialize_engine_user	( );
			void			initialize_network		( );
			void			initialize_network_modules ( );
			void			initialize_sound		( );
			void			initialize_sound_modules( );
			void			initialize_ui			( );
			void			initialize_physics		( );
			void			initialize_rtp			( );
			void			initialize_animation	( );
			void			initialize_logic_modules( );

private:
			void			create_render			( );
			void			destroy_render			( );
			void	delete_processed_logic_orders	( bool destroying );
			void	delete_processed_editor_orders	( bool destroying );
			void			logic_tick				( );
			void			logic					( );
			void			editor					( );
			void			network_tick			( );
			void			network					( );
			void			sound_tick				( );
			void			sound					( );
			void			try_load_editor			( );
			void			unload_editor			( );
			void			show_window				( );
			bool			process_messages		( );
			void			logic_clear_resources	( );
			void			network_clear_resources	( );
			void			sound_clear_resources	( );
			void			editor_clear_resources	( );
			void			finalize_logic_modules	( );
			void	initialize_editor_thread_ids	( );
private:
	typedef render::engine::command			command_type;

	struct command_type_impl : public command_type {
		virtual			~command_type_impl			( ) { }
		virtual	void	execute						( ) { }
	}; // struct command_type_impl
	typedef intrusive_spsc_queue< command_type, command_type, &command_type::next >	command_list_type;
	typedef memory::doug_lea_allocator_type								doug_lea_allocator_type;

private:
	static	void			delete_processed_orders	( command_list_type& list, doug_lea_allocator_type& allocator, u32 frame_id, bool destroying );

private:
	doug_lea_allocator_type			m_input_allocator;
	doug_lea_allocator_type			m_engine_user_allocator;
	doug_lea_allocator_type			m_network_allocator;
	doug_lea_allocator_type			m_sound_allocator;
	doug_lea_allocator_type			m_ui_allocator;
	doug_lea_allocator_type			m_physics_allocator;
	doug_lea_allocator_type			m_rtp_allocator;
	doug_lea_allocator_type			m_animation_allocator;
	doug_lea_allocator_type			m_render_allocator;
	doug_lea_allocator_type			m_editor_allocator;

	command_list_type				m_processed_logic;
	command_list_type				m_processed_editor;

	timing::timer					m_timer;
	input::world*					m_input_world;
	render::world*					m_render_world;
	network::world*					m_network_world;
	sound::world*					m_sound_world;
	ui::world*						m_ui_world;
	physics::world*					m_physics_world;
	rtp::world*						m_rtp_world;
	editor::world*					m_editor;
	animation::world*				m_animation_world;

//	engine_user
	xray::engine_user::module_proxy* m_engine_user_module_proxy;
	xray::engine_user::world*		 m_engine_user_world;

	HWND							m_window_handle;
	HWND							m_main_window_handle;
	u32								m_logic_frame_id;
	u32								m_editor_frame_id;
	float							m_last_frame_time;
	threading::atomic32_type		m_destruction_started;
	bool							m_early_destruction_started;
	int								m_exit_code;
	bool							m_logic_frame_ended;
	bool							m_editor_frame_ended;
	bool							m_initialized;
	bool							m_game_enabled;
	bool							m_game_paused_last;
	bool							m_app_active;
}; // class engine_world

} // namespace engine

bool	command_line_editor					( );
bool	command_line_editor_singlethread	( );

} // namespace xray

#endif // #ifndef ENGINE_WORLD_H_INCLUDED	