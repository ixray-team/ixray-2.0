////////////////////////////////////////////////////////////////////////////
//	Created		: 31.05.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef ANIMATION_ACTION_H_INCLUDED
#define ANIMATION_ACTION_H_INCLUDED

#include "action.h"



namespace xray {

namespace animation {
class i_animation_action;
class i_animation_controller_set;
} // namespace animation


namespace rtp {




template< class space_params, class world_space_params, class action >
class animation_action:
	public abstract_action< space_params, world_space_params, action >
{
	
	typedef abstract_action< space_params, world_space_params, action >	super;

public:
	animation_action( action_base<action> &ab, animation::i_animation_action  const *animation );
	animation_action( action_base<action> &ab );

public:
	virtual		void	render					( const space_params& ,  xray::render::debug::renderer&  ) const{}
	virtual		void	render					( const world_space_params&,  xray::render::debug::renderer&  ) const{}
	virtual		void	save					( xray::configs::lua_config_value cfg )const;
	virtual		void	load					( const xray::configs::lua_config_value &cfg );
	virtual		void	construct				( );



public:
				void	get_weights				( u32 blend, buffer_vector< float > &weights )const;

protected:
	virtual		void	cache_transforms		( );
	virtual		void	cache_blend_transform	( u32 blend, buffer_vector< float > &weights );

public:
	virtual		float	run						( world_space_params& wfrom, const space_params& lfrom, const action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const	;
	virtual		void	run						( space_params& , const action& , u32 , u32  )const{}

protected:
	virtual		void	on_walk_run				( world_space_params& wfrom, const space_params& lfrom, const float4x4 &disp, buffer_vector< float > &weights )const;

public:
	virtual		u32		num_blend_samples		()const;

public:
				u32		anim_count				()const;
				float	duration				()const;

public:
	virtual		pcstr	name					()const;


				
protected:
				void	weight_mean				( buffer_vector< float > &weights, u32 blend[4], float factors[2] )const;
				void	calc_samples_per_weight	();




protected:
	animation::i_animation_action		const *m_animation_action;

	static 			u32					max_samples	;
					u32					samples_per_weight;

protected:
	vector< float4x4 >					m_cache_transforms;

}; // class animation_action






} // namespace rtp
} // namespace xray

#include "animation_action_inline.h"

#endif // #ifndef ANIMATION_ACTION_H_INCLUDED