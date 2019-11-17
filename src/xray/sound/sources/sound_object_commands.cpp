////////////////////////////////////////////////////////////////////////////
//	Created		: 27.04.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sound_object_commands.h"
#include "sound_object.h"
#include "sound_world.h"

namespace xray {
namespace sound {

callback_command::callback_command( base_allocator_type* a, cb_event const& ev, sound_object_impl* o, void* context)
:super(a,o),
m_event_type(ev),
m_context(context)
{};

void callback_command::execute( )
{
	switch (m_event_type)
	{
	case ev_stream_end:
		m_owner->on_end_stream( );
		break;

	case ev_buffer_end:
		
		m_owner->on_buffer_end( m_context );
		break;

	case ev_buffer_error:
		m_owner->on_buffer_error( m_context );
		break;

	default:
		LOG_ERROR("unknown callback event: %d", (int)m_event_type);
		NODEFAULT();
	}
}

listener_properties_command::listener_properties_command( base_allocator_type* a, sound_world& sound_world, float4x4 const& inv_view_matrix )
:super(a), 
m_sound_world(sound_world), 
m_inv_view_matrix(inv_view_matrix)
{}

void listener_properties_command::execute()
{
	m_sound_world.set_listener_properties_impl( m_inv_view_matrix );
}

playback_command::playback_command( base_allocator_type* a, playback_event const& ev, sound_object_impl* o )
:super(a,o),
m_event(ev)
{}

void playback_command::execute( )
{
	switch (m_event)
	{
	case ev_play:
		m_owner->play_impl();
		break;

	case ev_stop:
		m_owner->stop_impl();
		break;

	default:
		NODEFAULT();
	};
}

sound_position_command::sound_position_command( base_allocator_type* a, float3 const& position, sound_object_impl* o )
:super(a, o),
m_position(position)
{
}

void sound_position_command::execute()
{
	m_owner->set_position_impl(m_position);
}

} // namespace sound
} // namespace xray
