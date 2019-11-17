////////////////////////////////////////////////////////////////////////////
//	Created		: 24.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TRAINING_VALUE_FUNCTION_H_INCLUDED
#define TRAINING_VALUE_FUNCTION_H_INCLUDED

#include "value_function.h"
#include "training_regression_tree.h"
#include "trajectory.h"

#include <xray/threading_mutex.h>

namespace xray {
namespace rtp {

template< class action >
class training_value_function:
	public value_function< training_regression_tree< typename action::space_param_type >, action >
{
	typedef value_function< training_regression_tree< typename action::space_param_type >, action > super;

	using super::clear;
	using super::m_action_base;
	using super::m_regression_trees;

	typedef	action												action_type;
	typedef	typename action::space_param_type					space_param_type;
	typedef	training_regression_tree< space_param_type >		regression_tree_type;
	typedef	typename regression_tree_type::training_sample_type	training_sample_type;

public:
					training_value_function	( const action_base<action_type>& ab );
		virtual		~training_value_function( );

public:
	void			learn				( );
	bool			samples_step		( );
	float			learn_step			( );
	void			learn_init			( );

public:
	void		save				( xray::configs::lua_config_value cfg, bool training )const;
	void		load				( const xray::configs::lua_config_value &cfg );

	void		save_training_sets	( xray::configs::lua_config_value cfg )const;
	void		load_training_sets	( const xray::configs::lua_config_value &cfg );

public:
virtual	void	render		( xray::render::debug::renderer& renderer ) const;

private:
		float					generate_samples	( );
virtual	void					init_trees			( );
		void					build_trees			( );
		void					recalculate_trees	( );
		float					update_sets			( );
		void					prune				( );
		void					prune_build_tree	( u32 min_samples, u32 id_action );
		void					randomize_sets		( );
		float					prune_residual		( );
		float					prune_residual		( u32 id_action );
		float					prune_sum_residual	( u32 id_action, u32 &number );
		u32						samples_total		( );
		void					clear_training_sets ( );
		void					clear_trajectories  ( );
private:
	rtp::vector< training_set< training_sample_type > >	m_training_sets;
	rtp::vector< regression_tree_type* >				m_building_regression_trees;
	rtp::vector< trajectory<action_type> >				m_trajectories;
	//math::random32 rnd;

private:
threading::mutex		m_add_samples_mutex;
u32						m_steps_tries;
u32						m_steps_samples;

u32						m_prune_steps;

}; // class training_value_function

} // namespace rtp
} // namespace xray

#include "training_value_function_inline.h"

#endif // #ifndef TRAINING_VALUE_FUNCTION_H_INCLUDED