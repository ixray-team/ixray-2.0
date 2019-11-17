////////////////////////////////////////////////////////////////////////////
//	Created		: 24.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TRAINING_REGRESSION_TREE_H_INCLUDED
#define TRAINING_REGRESSION_TREE_H_INCLUDED

#include "regression_tree.h"
#include "space_params.h"
#include "training_set.h"
#include <xray/math_randoms_generator.h>

namespace xray {

namespace configs{
	class lua_config_value;
} // namespace configs

namespace rtp {

template< class space_param_type >
class training_node:
	public node< space_param_type >
{
		typedef	node< space_param_type > super;
		using super::left;
		using super::right;
		using super::m_split_value;
		using super::m_split_dimension;
		using super::m_reward_value;
		using super::is_leaf;

public:
		typedef	std::pair< space_param_type, float >				training_sample_type;
		typedef	iaction<space_param_type>							iaction_type;

public:
		training_node	(	u32 level, const training_sample_type** samples_begin, const training_sample_type** samples_end, 
							const space_param_type &min, const space_param_type &max, u32 min_samples );

		training_node	( u32 level, u32 min_samples );
		training_node	( u32 level );

public:
virtual	~training_node	( );

public:
virtual	bool	build		( );
virtual	bool	recalculate ( );

public:
		void	init_build	( );
		void	set_samples	( const training_sample_type **begin, const training_sample_type **end );
		void	render		( xray::render::debug::renderer& renderer, const iaction_type &a ) const;

private:
virtual	super*	create			( u32 level );
virtual	u32		level			( ) { return m_level; };
		bool	divide			( );
		void	compute_reward	( );

private:
		void	dbg_check_validity	( ) const;

public:
virtual		void	save		( xray::configs::lua_config_value cfg, const node_save_load_data< space_param_type >& data )const;
virtual		void	load		( const xray::configs::lua_config_value &cfg, node_save_load_data< space_param_type >& data );

public:
	static const u32							min_samples_max			= 50;
	static const u32							max_tree_depth			= 28000;

private:
	bool	samples_empty		( ) const {return m_samples_begin == m_samples_end; } 
	u32		samples_size		( ) const {return u32( m_samples_end - m_samples_begin ); } 

private:
	space_param_type								m_min, m_max;

	const training_sample_type						**m_samples_begin,  **m_samples_end;

	//rtp::vector< const training_sample_type* >	m_training_samples;
	u32												m_level;
	u32												m_min_samples;

};


template< class space_param_type >
class training_regression_tree :
	public regression_tree< training_node< space_param_type > >
{
	typedef	regression_tree< training_node< space_param_type > > super;

	using super::m_root;

public:
	typedef typename super::node_type::training_sample_type		training_sample_type;
//rtp::vector< const training_sample_type* >
//	typedef		buffer_vector<const training_sample_type*>		node_sapmles_vec_type;

public:
	training_regression_tree	(  ){};
	training_regression_tree	(  training_set<training_sample_type>& set );
	training_regression_tree	(  u32 min_samples, buffer_vector<const training_sample_type*>	&samples );

public:

		void							recalculate	( )								 { m_root.recalculate()		; }	
		void							build		( );
public:
virtual	void							save		( xray::configs::lua_config_value cfg, const node_save_load_data< space_param_type >& data  )const;
virtual	void							load		( const xray::configs::lua_config_value &cfg, node_save_load_data< space_param_type >& data );

public:
	void							render		( xray::render::debug::renderer& renderer, const iaction< space_param_type > &a ) const;

private:
	void							add_sample		( const training_sample_type *s );

private:
	void							add_samples		( vector< training_sample_type > &samples );

public:
	const training_sample_type*		const &p_sample	( u32 i )const;
	const training_sample_type*		&p_sample		( u32 i );
	const	u32						num_samples		( )const{ return m_num_samples; }

private:
	static const u32							samples_max			= 50000;
	const training_sample_type*					m_training_samples_buff[ samples_max ];
	u32											m_num_samples;

	//node_sapmles_vec_type						m_training_samples;

}; // class regression_tree




template< class space_param >
class training_save_load_data :
	public node_save_load_data< space_param >,
	private boost::noncopyable
{
	typedef	std::pair< space_param, float >						training_sample_type;
//rtp::vector< const training_sample_type* >
	//typedef		buffer_vector<const training_sample_type*>		node_sapmles_vec_type;

public:
					training_save_load_data( const rtp::vector< training_sample_type > &samples, training_regression_tree<space_param> &tree ): m_samples( samples ), m_tree(tree){}
public:
	virtual const	training_save_load_data< space_param >*	get_training_save_load_data()const		{ return this; }
	virtual			training_save_load_data< space_param >*	get_training_save_load_data()			{ return this; }

public:
	inline const	rtp::vector< training_sample_type >		&samples()const	{ return m_samples; }
	inline const	training_regression_tree<space_param>	&tree	()const { return m_tree; }
	inline 			training_regression_tree<space_param>	&tree	()		{ return m_tree; }
private:
	const rtp::vector< training_sample_type >	&m_samples;
	training_regression_tree<space_param>		&m_tree;
}; // class training_save_load_data




} // namespace rtp
} // namespace xray

#include "training_regression_tree_inline.h"

#endif // #ifndef TRAINING_REGRESSION_TREE_H_INCLUDED