////////////////////////////////////////////////////////////////////////////
//	Created 	: 22.12.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef SOUND_WORLD_H_INCLUDED
#define SOUND_WORLD_H_INCLUDED

#include <xray/sound/world.h>
#include "sound_object_commands.h"
#include "sound_object.h"

namespace xray {
namespace sound {

struct engine;
class sound_stream;

class sound_world :	public sound::world,
					private boost::noncopyable
{
public:
					sound_world						( sound::engine& engine, base_allocator_type* l_allocator, base_allocator_type* e_allocator );
	virtual			~sound_world					( );
	virtual	void	tick							( u32 logic_frame_id );

	virtual void	set_listener_properties			( float4x4 const& inv_view_matrix, base_allocator_type* a );

	virtual	void	clear_resources					( );
	virtual	void	clear_logic_resources			( );
	virtual	void	clear_editor_resources			( );

	IXAudio2*		xaudio_engine					( ) const	{return m_xaudio;}

			void	add_xaudio_callback_command		( callback_command* cmd );
			void	add_end_user_command			( sound_command* cmd );
			void	add_editor_callback_command		( sound_command* cmd );
			void	add_logic_callback_command		( sound_command* cmd );

			void	process_logic_callback_orders	( );
			void	process_editor_callback_orders	( );

			pbyte	get_cache_buffer				( u32 size );
			void	free_cache_buffer				( pbyte ptr );
			void	on_voice_stopped				( sound_impl_ptr item );
			void	on_voice_started				( sound_impl_ptr item );
			void	on_voice_position_changed		( sound_impl_ptr item );
			void	set_listener_properties_impl	( float4x4 const& inv_view_matrix );
private:
			void	process_sound_thread_orders		( );

			void	register_sound_cookers			( );
			void	unregister_sound_cookers		( );
			void	initialize_command_processor	( );
			void	calculate_3d_sound				( sound_impl_ptr item );

			template <typename T> 
			inline void delete_orders	( T& queue );

	engine&							m_engine;
	// xaudio2
	IXAudio2*						m_xaudio;
	IXAudio2MasteringVoice*			m_master_voice;
	X3DAUDIO_HANDLE					X3DInstance;	
	X3DAUDIO_LISTENER				m_listener;
	
	typedef vector<sound_impl_ptr>	sounds;
	sounds							m_active_items;

	base_allocator_type*			m_logic_allocator;
	base_allocator_type*			m_editor_allocator;

	sound_commands_spsc_type		m_xaudio_callback_orders;	// xaudio thread->sound thread
	sound_commands_mpsc_type		m_end_user_orders;			// logic, editor thread->sound thread
	sound_commands_spsc_type		m_logic_callbacks_orders;	// sound thread->logic thread
	sound_commands_spsc_type		m_editor_callbacks_orders;	// sound thread->editor thread
	
	sound_commands_spsc_type		m_logic_orders_to_delete;
	sound_commands_spsc_type		m_editor_orders_to_delete;
	sound_commands_mpsc_type		m_sound_orders_to_delete;

}; // class world
extern sound_world* g_sound_world; // temporary, dont remove;
} // namespace sound
} // namespace xray

#include "sound_world_inline.h"

#endif // #ifndef SOUND_WORLD_H_INCLUDED