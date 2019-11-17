////////////////////////////////////////////////////////////////////////////
//	Created 	: 22.12.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sound_world.h"
#include "sound_object.h"
#include "sound_stream.h"

#include "sound_cooker.h"
#include "sound_stream_cooker.h"
#include "ogg_source_cooker.h"

static xray::command_line::key	s_debug_audio("debug_audio", "", "sound engine", "running debug XAudio2 engine");


namespace xray {
namespace sound {

sound_world* g_sound_world = NULL;

#if !XRAY_PLATFORM_PS3
static void role_to_string (XAUDIO2_DEVICE_ROLE const role, xray::fixed_string2048& dest )
{
	if(role == NotDefaultDevice)
		dest += "NotDefaultDevice ";
	else
	{
	if(role & DefaultConsoleDevice)
		dest += "DefaultConsoleDevice ";

	if(role & DefaultMultimediaDevice)
		dest += "DefaultMultimediaDevice ";

	if(role & DefaultCommunicationsDevice)
		dest += "DefaultCommunicationsDevice ";

	if(role & DefaultGameDevice)
		dest += "DefaultGameDevice ";

	if(role & GlobalDefaultDevice)
		dest += "GlobalDefaultDevice ";
	}
}
#endif // #if !XRAY_PLATFORM_PS3

sound_world::sound_world( xray::sound::engine& engine, base_allocator_type* l_allocator, base_allocator_type* e_allocator ) 
:m_engine				( engine ),
m_logic_allocator		( l_allocator ),
m_editor_allocator		( e_allocator )
{
	g_sound_world	= this;
	register_sound_cookers					( );

	initialize_command_processor			( );

#if !XRAY_PLATFORM_PS3
	LOG_INFO								("Sound initialization...");

	u32 creation_flags = 0;
	if(s_debug_audio)
		creation_flags	|= XAUDIO2_DEBUG_ENGINE;

	HRESULT res					= XAudio2Create(&m_xaudio ,creation_flags, XAUDIO2_DEFAULT_PROCESSOR);

	if( FAILED(res) && s_debug_audio )
	{ // try load retail version of interface
		creation_flags			= 0;
		res						= XAudio2Create(&m_xaudio ,creation_flags, XAUDIO2_DEFAULT_PROCESSOR);
	}

	if( FAILED(res) )
	{
		LOG_ERROR				("Sound initialization FAILED. Can not create XAudio2 interface. %d", res);
		R_ASSERT				( 0 );
	}
	
	u32 device_count			= 0;
	m_xaudio->GetDeviceCount	(&device_count);
	LOG_INFO					("devices count: %d", device_count);

	XAUDIO2_DEVICE_DETAILS		deviceDetails;
	int preferred_device_id		= -1;
	fixed_string2048			device_role;

	for(u32 i=0; i<device_count; ++i)
	{
		m_xaudio->GetDeviceDetails	(i,&deviceDetails);
		device_role				= "";
		LOG_INFO				("%d: %S", i, deviceDetails.DisplayName);
		role_to_string			(deviceDetails.Role, device_role);
		LOG_INFO				("role: %s", device_role.c_str());
		if(deviceDetails.Role & DefaultMultimediaDevice)
			preferred_device_id	= i;
	}
	
	R_ASSERT(preferred_device_id!=-1, "There is no Default Multimedia Device in system");

	res							= m_xaudio->CreateMasteringVoice( 
										&m_master_voice, 
										XAUDIO2_DEFAULT_CHANNELS,
										XAUDIO2_DEFAULT_SAMPLERATE, 
										0, 
										preferred_device_id, 
										NULL 
										);
	ASSERT						(!FAILED(res));

	// initialize X3DAudio
	m_xaudio->GetDeviceDetails		(preferred_device_id, &deviceDetails);
	u32 channelMask					= deviceDetails.OutputFormat.dwChannelMask;
	X3DAudioInitialize				( channelMask, X3DAUDIO_SPEED_OF_SOUND, X3DInstance );
#endif // #if !XRAY_PLATFORM_PS3

	memory::zero					(&m_listener, sizeof(m_listener));
}

sound_world::~sound_world		( )
{
#if !XRAY_PLATFORM_PS3
	m_master_voice->DestroyVoice( );
	m_xaudio->Release			( );
#endif // #if !XRAY_PLATFORM_PS3

	g_sound_world				= NULL;
	unregister_sound_cookers	( );
}

void sound_world::tick( u32 const logic_frame_id )
{
	XRAY_UNREFERENCED_PARAMETER		( logic_frame_id );
	process_sound_thread_orders		( );
}


pbyte sound_world::get_cache_buffer( u32 size )
{
	return (pbyte)MALLOC( size, "pcm_data");
}

void sound_world::free_cache_buffer( pbyte ptr )
{
	FREE( ptr );
}

void sound_world::on_voice_stopped( sound_impl_ptr item )
{
	sounds::iterator it		= std::find(m_active_items.begin(), m_active_items.end(), item);
	ASSERT(it != m_active_items.end());
	m_active_items.erase	( it );
}

void sound_world::on_voice_started( sound_impl_ptr item )
{
	sounds::iterator it		= std::find(m_active_items.begin(), m_active_items.end(), item);
	R_ASSERT_U				(it == m_active_items.end());
	m_active_items.push_back( item );
}

void sound_world::set_listener_properties( float4x4 const& inv_view_matrix, base_allocator_type* a )
{
	//listener_properties_command* cmd	= XRAY_NEW_IMPL(a, listener_properties_command)( a, *this, inv_view_matrix );
	//add_end_user_command				( cmd );
}

#if !XRAY_PLATFORM_PS3
static void make_xaudio_vector(float3 const& src, X3DAUDIO_VECTOR& dst)
{
	dst.x = src.x; dst.y = src.y; dst.z = src.z;
}
#endif // #if !XRAY_PLATFORM_PS3

void sound_world::set_listener_properties_impl( float4x4 const& inv_view_matrix )
{
#if !XRAY_PLATFORM_PS3
	make_xaudio_vector		( inv_view_matrix.k.xyz(), m_listener.OrientFront);
	make_xaudio_vector		( inv_view_matrix.j.xyz(), m_listener.OrientTop);
	make_xaudio_vector		( inv_view_matrix.c.xyz(), m_listener.Position);
	make_xaudio_vector		(float3(0,0,0), m_listener.Velocity);
	m_listener.pCone		= NULL;
#endif // #if !XRAY_PLATFORM_PS3


	sounds::iterator it		= m_active_items.begin();
	sounds::iterator it_e	= m_active_items.end();
	for(; it!=it_e; ++it)
		calculate_3d_sound( *it );
}

void sound_world::on_voice_position_changed( sound_impl_ptr item )
{
}

void sound_world::calculate_3d_sound( sound_impl_ptr item )
{
#if !XRAY_PLATFORM_PS3
	X3DAUDIO_EMITTER Emitter		= {0};

	X3DAUDIO_DSP_SETTINGS DSPSettings = {0};
	float		matrix[8];

	// settings
	DSPSettings.pMatrixCoefficients = matrix;
	DSPSettings.pDelayTimes			= NULL;
	DSPSettings.SrcChannelCount		= 1;
	DSPSettings.DstChannelCount		= 2;

	// emitter
	Emitter.pCone					= NULL;
	make_xaudio_vector				( float3(0,0,1), Emitter.OrientFront);
	make_xaudio_vector				( float3(0,1,0), Emitter.OrientTop);
	make_xaudio_vector				( item->get_position(), Emitter.Position);
	make_xaudio_vector				( float3(0,0,0), Emitter.Velocity);
	Emitter.InnerRadius				= 0.0f;
	Emitter.InnerRadiusAngle		= 0.0f;
	Emitter.ChannelCount			= 1;
	Emitter.ChannelRadius			= 0.0f;
	Emitter.pChannelAzimuths		= NULL;	
	Emitter.pVolumeCurve			= NULL;//(X3DAUDIO_DISTANCE_CURVE*)&X3DAudioDefault_LinearCurve;
	Emitter.pLFECurve				= NULL;
	Emitter.pLPFDirectCurve			= NULL;
	Emitter.pLPFReverbCurve			= NULL;
	Emitter.pReverbCurve			= NULL;

	Emitter.CurveDistanceScaler			= 30.0; // 30m = min_dist
	Emitter.DopplerScaler				= 0.0f;

	X3DAudioCalculate(	X3DInstance, &m_listener, &Emitter,
						X3DAUDIO_CALCULATE_MATRIX , &DSPSettings );

	item->m_source_voice->SetOutputMatrix( m_master_voice, 
											item->wave_format().nChannels, 
											2, //deviceDetails.OutputFormat.Format.nChannels, FIXME
											DSPSettings.pMatrixCoefficients ) ;

#endif // #if !XRAY_PLATFORM_PS3
}

sound_cooker*			g_sound_cooker;
sound_stream_cooker*	g_sound_stream_cooker;
ogg_source_cooker*		g_ogg_source_cooker;

void sound_world::register_sound_cookers( )
{
	g_sound_cooker			= NEW(sound_cooker)( /**this*/ );
	g_sound_stream_cooker	= NEW(sound_stream_cooker)( /**this*/ );
	g_ogg_source_cooker		= NEW(ogg_source_cooker)( );
	register_cook			( g_sound_cooker );
	register_cook			( g_sound_stream_cooker );
	register_cook			( g_ogg_source_cooker );
}

void sound_world::unregister_sound_cookers( )
{
	DELETE					( g_sound_cooker );
	DELETE					( g_sound_stream_cooker );
	DELETE					( g_ogg_source_cooker );
}


} // namespace sound
} // namespace xray
