////////////////////////////////////////////////////////////////////////////
//	Created		: 06.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TEST_GRASPING_ACTION_H_INCLUDED
#define TEST_GRASPING_ACTION_H_INCLUDED

#include "action.h"
#include "grasping_space_params.h"
#include "action_base.h"

namespace xray {
namespace rtp {

class test_grasping_action:
	public abstract_action< grasping_space_params, test_grasping_world_params, test_grasping_action >
{

typedef	abstract_action< grasping_space_params, test_grasping_world_params, test_grasping_action > super;

public:


						test_grasping_action( action_base< test_grasping_action > &ab );
						test_grasping_action( 
							action_base<test_grasping_action> &ab,  
							float rotation0,float rotation1,
							const float2 &translation0,
							const float2 &translation1,
								float	 duration_time,
							bool  is_grasping,
							const grasping_space_params &grasping_position
							);

	virtual		float	reward				( const test_grasping_action& a ) const;
	virtual		float	reward				( const grasping_space_params& a ) const;

	virtual		void	run					( grasping_space_params& from, const test_grasping_action& af, u32 from_sample, u32 blend_sample  )const;
	//virtual		void	run					( space_params& a, u32 blend_sample, const action_type& from, u32 from_sample )const=0;

	virtual		float	run					( test_grasping_world_params& wfrom, const  grasping_space_params& lfrom, const test_grasping_action& afrom, u32 from_sample, u32 blend_sample, u32 step, float time )const	;

	//virtual		float	run					( const world_space_param_type& f, const action_type& from, u32 from_sample, world_space_param_type& to, u32 blend_sample, float time )const=0;


public:	
	virtual		u32		num_blend_samples	( )const ;													
	virtual		bool	taget				( const grasping_space_params& a )const;
	virtual		bool	is_taget_action		( )const ;
	virtual		void	render				( const grasping_space_params& a,  xray::render::debug::renderer& renderer ) const;
	virtual		void	render				( const test_grasping_world_params& a,  xray::render::debug::renderer& renderer ) const;

public:
	virtual		void	save				( xray::configs::lua_config_value cfg )const;
	virtual		void	load				( const xray::configs::lua_config_value &cfg );

public:
				float2	mean_translate		( )const;
				float   mean_rotate			( )const;
				float	duration			( )const	{return m_duration_time;}

private:
				bool	border				( const grasping_space_params& tp )const;
				bool	rtaget				( const grasping_space_params& tp )const;
				void	sample				(u32 blend_sample, float &rotate, float2 &translate )const;

private:
	bool					b_grasping;
	grasping_space_params	m_grasping_state;

private:
	float					m_rotation0;
	float2					m_translation0;
	float					m_rotation1;
	float2					m_translation1;
	float					m_duration_time;
}; // class test_grasping_action

} // namespace rtp
} // namespace xray

#endif // #ifndef TEST_GRASPING_ACTION_H_INCLUDED