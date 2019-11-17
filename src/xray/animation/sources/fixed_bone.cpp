////////////////////////////////////////////////////////////////////////////
//	Created		: 09.06.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "fixed_bone.h"

#include "skeleton.h"


#include <xray/configs_lua_config.h>

namespace xray {

namespace animation {


	void fixed_bone::read( configs::lua_config_value const& config, const animation::skeleton &s )
	{
		pcstr bone_name = config["name"];
		
		m_anim_skeleton_idx = skeleton_bone_index( s, bone_name );

		m_fixed_vertex = config["vertex"];
	}


	void	fixed_bones::read ( configs::lua_config_value const& config  )
	{
		const u32 sz = config.size();
		m_bones.resize( sz );
		vector<fixed_bone>::iterator i, b = m_bones.begin(), e = m_bones.end();
		for( i = b ; i != e; ++i )
			(*i).read( config[ u32( i - b ) ], m_skeleton );

	}
	
	struct find_predicate
	{
		u32 idx;

		find_predicate( u32 bone_idx ): idx( bone_idx ){}

		bool operator () ( const fixed_bone &b )const
		{
			return b.m_anim_skeleton_idx == idx;
		}
	};

	u32	fixed_bones::fixed_bone_idx	( pcstr name )const
	{
		u32 skel_bone_idx = skeleton_bone_idx( name );
		
		find_predicate fp( skel_bone_idx );
		vector<fixed_bone>::const_iterator r = std::find_if( m_bones.begin(), m_bones.end(), fp );

		if( r == m_bones.end() )
			return u32(-1);

		return u32( r - m_bones.begin() );
	}

	u32 fixed_bones::skeleton_bone_idx( pcstr name )const
	{
		pcstr	free_interval = "none";
		if( strings::compare( name, free_interval ) == 0 )
			return u32( -1 );
		
		return skeleton_bone_index( m_skeleton, name );
	}

	void	fixed_bones::read_steps ( foot_steps &steps, float play_time,  configs::lua_config_value const& config )const
	{
		
		const u32 sz_bones		= config["bones"].size();
		
		if( sz_bones  == 0 )
			return;

		for( u32 i = 0; i < sz_bones; ++i )
			steps.m_bones.bones.push_back( fixed_bone_idx( config["bones"][ i ] ) );

		const u32 sz_anims = config["intervals"].size();

		steps.m_steps.resize( sz_anims );

		for( u32 j = 0; j < sz_anims; ++j )
		{
			const u32 sz_intervals	= config["intervals"][j].size();
			R_ASSERT( sz_intervals > 2 );
			R_ASSERT( sz_bones + 1   == sz_intervals );

			
			for( u32 i = 0; i < sz_intervals; ++i )
			{
				steps.m_steps[j].intervals.push_back( config["intervals"][j][i] );
			}
			R_ASSERT_CMP( steps.m_steps[j].intervals[ sz_intervals - 2 ], <=, play_time );

			steps.m_steps[j].intervals[ sz_intervals - 1 ] = play_time;

		}
	}
	
	float	sum_weights		( const buffer_vector< float > &weights );
	void foot_steps::get_intervals( buffer_vector< float > &intervals,  const buffer_vector< float > &weights )const
	{
		const u32 intervals_size = intervals.size();
		const u32 sz = m_steps.size();
		ASSERT( weights.size() >= sz );
		ASSERT( math::similar( sum_weights( weights ) , 1.f )  );

		for( u32 i = 0; i < intervals_size; ++i )
		{
			intervals[i] = 0;

			for( u32 j = 0; j < sz; ++j )
			{
				R_ASSERT( m_steps[ j ].intervals.size() == intervals_size );
				
				intervals[i] += weights[ j ] * m_steps[ j ].intervals[ i ];
			}

		}
	}




	u32 foot_steps::bone( float time, const buffer_vector< float > &weights )const
	{
		float dummi;
		return bone( time, weights, dummi );
	}
	u32		foot_steps::bone			( float time, const buffer_vector< float > &weights, float& interval_start )const
	{
		const u32 sz = weights.size();

		R_ASSERT( sz == m_steps.size( ) );

		const u32 intervals_size = foot_steps::intervals_size( );

		buffer_vector< float > intervals( ALLOCA( sizeof(float) * intervals_size  ), intervals_size, intervals_size ) ;
		
		get_intervals( intervals, weights );
		
		buffer_vector< float >::iterator  ib = intervals.begin() , ie = intervals.end();
		
		buffer_vector< float >::iterator res = std::lower_bound( ib, ie, time );
		
		//if( res == ib || res == ie )
		//	return u32(-1);

		if( res == ib  )
		{
			interval_start = *ib;
			return m_bones.bones[ 0 ];
		}
		if( res == ie  )
		{
			interval_start = *(ie-1);
			return m_bones.bones[ m_bones.bones.size() - 1 ];
		}
		const u32 step = u32( res - ib ) - 1;
		u32 fixed_b_idx = m_bones.bones[ step ];
		
		interval_start = *( res - 1 );

		return fixed_b_idx;

	}


} // namespace animation
} // namespace xray