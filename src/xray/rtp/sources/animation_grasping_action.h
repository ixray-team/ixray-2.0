////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RTP_ANIMATION_GRASPING_ACTION_H_INCLUDED
#define XRAY_RTP_ANIMATION_GRASPING_ACTION_H_INCLUDED



#include "action_base.h"
#include "animation_grasping_params.h"
#include "animation_action.h"

namespace xray {

namespace rtp {

class animation_grasping_action;

typedef	 grasping_space_params								animation_grasping_space_params;

typedef global_actions_params<animation_grasping_action>	animation_grasping_global_params;

class taget_space_param_devision
{

public:
	void	add			( animation_grasping_space_params const &p, u32 blend );
	void	sort		( );
	bool	taget_blends( u32 blend[4], float factors[2], animation_grasping_space_params const &p )const;

private:
	vector< std::pair< float, u32 > >		m_x_sorted_taget;
	vector< std::pair< float, u32 > >		m_y_sorted_taget;
};


class animation_grasping_action:
	public rtp::animation_action< rtp::animation_grasping_space_params, rtp::animation_grasping_world_params, animation_grasping_action >
{

typedef rtp::animation_action< animation_grasping_space_params, animation_grasping_world_params, animation_grasping_action >	super;

public:
	animation_grasping_action( action_base<animation_grasping_action> &ab, animation::i_animation_action  const *animation );
	animation_grasping_action( action_base<animation_grasping_action> &ab );


	virtual		float	reward					( const animation_grasping_action& a ) const;
	virtual		float	reward					( const grasping_space_params& a ) const;

	virtual		void	run						( animation_grasping_space_params& from, const animation_grasping_action& af, u32 from_sample, u32 blend_sample  )const;
	virtual		float	run						( animation_grasping_world_params& wfrom, const animation_grasping_space_params& lfrom, const animation_grasping_action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const	;

	virtual		bool	taget					( const animation_grasping_space_params& a )const;

public:
	virtual		void	save					( xray::configs::lua_config_value cfg )const;
	virtual		void	load					( const xray::configs::lua_config_value &cfg );
	virtual		void	construct				( );

public:
	virtual		bool	is_taget_action			( )const;

protected:
	virtual		void	cache_transforms		( );
	virtual		void	cache_blend_transform	( u32 blend, buffer_vector< float > &weights );
	virtual		void	on_walk_run				( animation_grasping_world_params& wfrom, const animation_grasping_space_params& lfrom, const float4x4 &disp, buffer_vector< float > &weights  )const;
//( const animation_grasping_world_params& wfrom, const animation_grasping_space_params& lfrom, const animation_grasping_action& afrom, u32 from_sample, animation_grasping_global_params& to, u32 blend_sample, u32 step, float time )
private:		
				bool	taget_blends			( u32 blend[4], float factors[2], animation_grasping_space_params const &p )const;
				void	transform_params		( animation_grasping_space_params& deleta, buffer_vector< float > &weights )const;

private:
	animation_grasping_space_params get_param_taget( const buffer_vector< float > &weights )const;

private:
				void	dbg_set_taget_params	( );


private:
	bool								is_taget;
	taget_space_param_devision			m_taget_space_param_devision;

}; // class animation_grasping_action



template<>
struct global_actions_params< animation_grasping_action >:
	public animation_global_actions_params
{
	global_actions_params< animation_grasping_action >( xray::animation::i_animation_controller_set *set ):
		animation_global_actions_params(set){}
} ;



} // namespace rtp
} // namespace xray

#endif // #ifndef XRAY_RTP_ANIMATION_GRASPING_ACTION_H_INCLUDED