////////////////////////////////////////////////////////////////////////////
//	Created		: 09.04.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "bone_mixer.h"
#include "animation_layer.h"
#include "bone_mixer_data.h"

namespace xray {
namespace animation {


	bone_mixer::bone_mixer( const skeleton &skel ):
	m_mix( skel ),
	m_current_mix_reference_pose( zero ),
	m_change_reference_time( u32(-1) )
	{

	}

	void	bone_mixer::on_new_animation( mixing::animation_clip const& animation, bone_mixer_data& data, u32 const current_time_in_ms )
	{ 
		XRAY_UNREFERENCED_PARAMETER(animation); //data.matrix.identity();
		if( m_mix.animations_count() == 0 )
			return;

		if( m_change_reference_time != current_time_in_ms )
		{
			//float4x4 bone;
			//m_mix.bone_matrix( 0, 0, bone );
			//
			//m_mix.reset_start_time( );
			//float4x4 previous_mix_pose ;

			//m_mix.bone_matrix( 0, 0,  previous_mix_pose );

			//float4x4 inv; inv.try_invert( previous_mix_pose );
			//float4x4 bone_disp =  bone * inv;
			
			float4x4 bone_disp;	m_mix.bone_matrix( 0, 0, bone_disp );
////////////////////////////////////////////////////////////////////////////////////////////
			//frame f = matrix_to_frame( bone_disp );
			//math::to_close_xyz( f.rotation,  m_current_mix_reference_pose.rotation );
			//add ( m_current_mix_reference_pose, f );
///////////////////////////////////////////////////////////////////////////////////////////

			float4x4 current_mix_reference_pose_matrix;
			frame_matrix( m_current_mix_reference_pose, current_mix_reference_pose_matrix );

			current_mix_reference_pose_matrix = bone_disp * current_mix_reference_pose_matrix;
			m_current_mix_reference_pose = matrix_to_frame( current_mix_reference_pose_matrix );

			m_change_reference_time = current_time_in_ms;
		}
			

		data.m_frame = m_current_mix_reference_pose;
		m_mix.clear();
	}

	void	bone_mixer::process( mixing::animation_state const* const data_begin, mixing::animation_state const* const data_end )
	{ 

		m_mix.clear();

		mixing::animation_state const* i = data_begin;

		for( ; i != data_end  ; ++i )
		{
			ASSERT( i->time <= i->animation->time() );
			
			animation_layer l( *i );
			
			//frame f( m_current_mix_reference_pose   );
			//frame fc ( i->bone_mixer_data->m_frame );

			float4x4 current_mix_reference_pose_matrix;
			frame_matrix( m_current_mix_reference_pose, current_mix_reference_pose_matrix );
			
			float4x4 mframe;
			frame_matrix( i->bone_mixer_data->m_frame, mframe );

			//math::to_close_xyz( fc.rotation, f.rotation   );

			//sub( f, fc );

			float4x4 inv_frame;
			inv_frame.try_invert(mframe);
			const float4x4 m = float4x4( current_mix_reference_pose_matrix * inv_frame );
			frame f = matrix_to_frame( m );

			l.set_root_start( f );
			m_mix.add( l );

		}
	}
	
	void	bone_mixer::reset_reference_pose( float4x4 &m )
	{
		m_current_mix_reference_pose = matrix_to_frame( m );
	}

	void	bone_mixer::render( render::debug::renderer	&renderer, const float4x4 &, float  )
	{
		float4x4 matrix;
		frame_matrix( m_current_mix_reference_pose, matrix );
		
		m_mix.render( renderer, matrix, 0 );

	}

} // namespace animation
} // namespace xray