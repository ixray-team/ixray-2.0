////////////////////////////////////////////////////////////////////////////
//	Created		: 31.05.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef ANIMATION_ACTION_INLINE_H_INCLUDED
#define ANIMATION_ACTION_INLINE_H_INCLUDED

#include <xray/animation/i_animation_action.h>
#include <xray/animation/i_animation_controller_set.h>

#include "animation_action_params.h"

namespace xray {
namespace rtp {

template< class space_params, class world_space_params, class action >
inline animation_action<space_params,world_space_params,action>::animation_action( 
	action_base<action> &ab,  
	animation::i_animation_action  const *anim ):
	super( ab ),
	m_animation_action( anim ),
	samples_per_weight(u32(-1))
{
	
	
}

template< class space_params, class world_space_params, class action >
inline animation_action<space_params,world_space_params,action>::animation_action( 
	xray::rtp::action_base<action> &ab
	):
	super( ab ),
	m_animation_action( 0 ),
	samples_per_weight(u32(-1))
{
	
}

template< class space_params, class world_space_params, class action >
inline float	animation_action<space_params,world_space_params,action>::duration			()const
{
	return m_animation_action->duration();
}

template< class space_params, class world_space_params, class action >
inline pcstr	animation_action<space_params,world_space_params,action>::name( )const 
{
	ASSERT( m_animation_action );
	return m_animation_action->name();
}

template< class space_params, class world_space_params, class action >
inline void	animation_action<space_params,world_space_params,action>::construct( )
{
	super::construct( );
	cache_transforms( );
}

template< class space_params, class world_space_params, class action >
inline void	animation_action<space_params,world_space_params,action>::save( xray::configs::lua_config_value cfg )const
{
	super::save( cfg );

	ASSERT( m_animation_action );

	cfg["group_id"] = m_animation_action->get_id( );

	
}

template< class space_params, class world_space_params, class action >
inline void	animation_action<space_params,world_space_params,action>::load( const xray::configs::lua_config_value &cfg )
{
	super::load( cfg );

	u32 id					= cfg["group_id"];
	animation::i_animation_controller_set *set = super::get_action_base().global_params().m_set;
	ASSERT( set );
	m_animation_action = set->action( id );
	samples_per_weight = u32( floor( exp( log( float( max_samples ) )/float( anim_count() -1 ) ) ) );

	cache_transforms	( );

}



template< class space_params, class world_space_params, class action >
inline void	animation_action<space_params,world_space_params,action>::calc_samples_per_weight	( )
{
	samples_per_weight = u32( floor( exp( log( float( max_samples ) )/float( anim_count() -1 ) ) ) );
}

template< class space_params, class world_space_params, class action >
inline u32		animation_action<space_params,world_space_params,action>::num_blend_samples	( )const
{
	ASSERT( m_animation_action );
	
	return math::pow( samples_per_weight  , m_animation_action->animations_number() - 1 );
}

template< class space_params, class world_space_params, class action >
inline u32		animation_action<space_params,world_space_params,action>::anim_count( )const
{
	ASSERT( m_animation_action );
	return m_animation_action->animations_number();
}

template< class space_params, class world_space_params, class action >
inline void	animation_action<space_params,world_space_params,action>::get_weights( u32 blend, buffer_vector< float > &weights )const
{
	const u32 anim_count = m_animation_action->animations_number() ;
	ASSERT( weights.capacity()>= anim_count );
	
	ASSERT( blend < num_blend_samples	( ) );
	weights.resize( anim_count );

	u32		bn	= blend;
	float	sum = 0;
	const u32 d = samples_per_weight ;
	for( u32 i = 0; i < anim_count-1 ; ++i )
	{
		u32 b = bn % d ;//samples_per_weight;
		bn = ( bn - b ) /  samples_per_weight;
		float fb = float( b )/float( d - 1 ); //anim_count - 1
		weights[i] = fb;
		sum += fb;
	}
	float clmp = sum; 
	math::clamp( clmp, 0.f,  1.f   );
	weights[ anim_count - 1 ] = 1.f - clmp;

	sum +=weights[ anim_count - 1 ];
	
	R_ASSERT( !math::is_zero( sum ) );

	{
		const float  isum = 1.f/sum;
		
		for( u32 i = 0; i < anim_count; ++i )
		{
			weights[i] *= isum;
			R_ASSERT( math::valid( weights[i] ) );
			R_ASSERT( weights[i] >= 0.f && weights[i] <= 1.f );
		}
	}

	//weights[ anim_count - 1 ] = 1.f - sum;
	// blend 

}

inline void	weight_mean( buffer_vector< float > &weights, const buffer_vector< float > &weights0, const buffer_vector< float > &weights1, float f )
{

	ASSERT( weights1.size() == weights0.size() );
	ASSERT( f >= 0 && f <=1 );

	const u32 sz = weights0.size();
	weights.resize( sz );
	
	float f1 = 1.f - f;
	for( u32 i = 0; i < sz; ++i )
		weights[i] = weights0[i] * f + weights1[i] * f1;

}

template< class space_params, class world_space_params, class action >
inline void	animation_action<space_params,world_space_params,action>::cache_blend_transform	( u32 blend, buffer_vector< float > &weights )
{
	m_animation_action->get_displacement_transform( m_cache_transforms[blend], weights );

}

template< class space_params, class world_space_params, class action >
inline void	animation_action<space_params,world_space_params,action>::cache_transforms	( )
{
	calc_samples_per_weight	( );
	const u32 blends_count	=	num_blend_samples( ) ;
	const u32 anim_count	=	m_animation_action->animations_number( );

	m_cache_transforms.resize( blends_count );

	buffer_vector<float>	weights( ALLOCA( sizeof( float ) * anim_count ), anim_count );

	for( u32 i =0; i < blends_count; ++i )
	{
		get_weights( i, weights );

		cache_blend_transform( i, weights );

	}

}

template< class space_params, class world_space_params, class action >
inline void	animation_action<space_params,world_space_params,action>::on_walk_run( world_space_params& wfrom, const space_params& lfrom , const float4x4 &disp, buffer_vector< float > &weights )const
{
	XRAY_UNREFERENCED_PARAMETERS( &weights, &lfrom );

	wfrom.controller_pos = disp * wfrom.controller_pos;//wfrom.taget_pos;
}

template< class space_params, class world_space_params, class action >
inline float	animation_action<space_params,world_space_params,action>::run( world_space_params& wfrom, const space_params& lfrom, const action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const
{
	
	
	const u32 lanim_count	=	anim_count();

	buffer_vector<float>	weights( ALLOCA( sizeof( float ) * lanim_count ), lanim_count );
	
	get_weights( blend_sample, weights );


	const u32 from_anim_count = afrom.anim_count();
	buffer_vector<float>	from_weights( ALLOCA( sizeof( float ) * from_anim_count ), from_anim_count );
	afrom.get_weights( from_sample, from_weights );


	
	float4x4 disp;
	m_animation_action->get_displacement_transform( disp, weights );

	

	on_walk_run( wfrom, lfrom,  disp, weights );
	
	animation::i_animation_controller_set *set = super::get_action_base( ).global_params().m_set;
	ASSERT_U( set );
	return m_animation_action->run( set, from_weights, weights, step, time );

}



template< class space_params, class world_space_params, class action >
inline void	animation_action<space_params,world_space_params,action>::weight_mean( buffer_vector< float > &weights, u32 blend[4], float factors[2] )const
{
	const u32 lanim_count	=	anim_count();

	buffer_vector<float>	weights0( ALLOCA( sizeof( float ) * lanim_count ), lanim_count );
	get_weights( blend[0], weights0 );

	buffer_vector<float>	weights1( ALLOCA( sizeof( float ) * lanim_count ), lanim_count );
	get_weights( blend[1], weights1 );

	buffer_vector<float>	mean0( ALLOCA( sizeof( float ) * lanim_count ), lanim_count );

	rtp::weight_mean( mean0, weights0, weights1, factors[0] );

	
	get_weights( blend[2], weights0 );
	get_weights( blend[3], weights1 );

	buffer_vector<float>	mean1( ALLOCA( sizeof( float ) * lanim_count ), lanim_count );

	rtp::weight_mean( mean1, weights0, weights1, factors[1] );

	rtp::weight_mean( weights, mean0, mean1, 0.5f );

}


inline animation_global_actions_params::animation_global_actions_params( xray::animation::i_animation_controller_set *set ):
m_set( set )
{
		
	float4x4 m;
	set->get_taget_transform( m );
	float3 vec( 0, 0, 0 );
	vec = m.transform_position( vec );

}


} // namespace rtp
} // namespace xray

#endif // #ifndef ANIMATION_ACTION_INLINE_H_INCLUDED