////////////////////////////////////////////////////////////////////////////
//	Created		: 31.05.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef NAVIGATION_ACTION_H_INCLUDED
#define NAVIGATION_ACTION_H_INCLUDED

#include "action_base.h"
#include "animation_action.h"

#include "navigation_params.h"
#include "animation_action_params.h"





namespace xray {
namespace rtp {

class navigation_action;

class navigation_theta_action;

template<>
struct global_actions_params< navigation_action >:
	public animation_global_actions_params
{
		global_actions_params< navigation_action >( xray::animation::i_animation_controller_set *set ):
			animation_global_actions_params(set){}
} ;

template<>
struct global_actions_params< navigation_theta_action >:
	public animation_global_actions_params
{
		global_actions_params< navigation_theta_action >( xray::animation::i_animation_controller_set *set ):
			animation_global_actions_params(set){}
} ;


//class navigation_action;

class navigation_action:
	public rtp::animation_action< navigation_params, navigation_world_params, navigation_action >
{
	typedef rtp::animation_action< navigation_params, navigation_world_params, navigation_action >	super;
public:
	navigation_action( action_base<navigation_action> &ab, animation::i_animation_action  const *animation ): super( ab, animation ){};
	navigation_action( action_base<navigation_action> &ab ): super( ab ){};

	virtual		float	reward							( const navigation_action& a ) const;
	virtual		float	reward							( const navigation_params& a ) const;

	virtual		bool	taget							( const navigation_params& )const	{ return false; }
	virtual		bool	is_taget_action					( )const { return false; }
	virtual		void	run								( navigation_params& from, const navigation_action& af, u32 from_sample, u32 blend_sample  )const;
	virtual		float	run								( navigation_world_params& wfrom, const navigation_params& lfrom, const navigation_action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const;


	virtual		bool	remove_out_of_area_trajectory	( )const { return false; }

private:

}; // class navigation_action

class navigation_theta_action:
	public rtp::animation_action< navigation_theta_params, navigation_theta_world_params, navigation_theta_action >
{
	typedef rtp::animation_action< navigation_theta_params, navigation_theta_world_params, navigation_theta_action >	super;
public:
	navigation_theta_action( action_base<navigation_theta_action> &ab, animation::i_animation_action  const *animation ): super( ab, animation ){};
	navigation_theta_action( action_base<navigation_theta_action> &ab ): super( ab ){};

	virtual		float	reward							( const navigation_theta_action& a ) const;
	virtual		float	reward							( const navigation_theta_params& a ) const;

	virtual		bool	taget							( const navigation_theta_params& )const	{ return false; }
	virtual		bool	is_taget_action					( )const { return false; }
	virtual		void	run								( navigation_theta_params& from, const navigation_theta_action& af, u32 from_sample, u32 blend_sample  )const;
	virtual		float	run								( navigation_theta_world_params& wfrom, const navigation_theta_params& lfrom, const navigation_theta_action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const;


	virtual		bool	remove_out_of_area_trajectory	( )const { return false; }

private:

}; // class navigation_action



} // namespace rtp
} // namespace xray

#endif // #ifndef NAVIGATION_ACTION_H_INCLUDED