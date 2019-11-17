////////////////////////////////////////////////////////////////////////////
//	Created		: 09.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

namespace xray {
namespace rtp {

static const float discount	= 0.8f;//dicount factor

template< class regression_tree, class action >
value_function< regression_tree, action >::value_function( const action_base<action_type>& ab ):
	m_action_base( ab )
{

}

template< class action >
inline float	state_reward( const	action &st, const typename action::space_param_type &params  )
{
	
	return st.reward( params );
}

template< class action >
inline float	instant_reward( const	action &afrom, const	action &ato, const typename action::space_param_type &param  ) 
{
	return afrom.reward( ato ) + state_reward( afrom, param ) ;
}



template< class regression_tree, class action >
float	value_function< regression_tree, action >::nu( const	action_type &afrom, u32 from_blend_id,  const space_param_type &from_param ) const
{
	float reward = 0, value = 0; 
	space_param_type to ;//=  pcurrent;
	u32 blend(u32(-1));//const action &a_to =
	*pi( afrom, from_blend_id, from_param, blend, to, reward, value );// r = R(a,a') + discount * V(s') ; v = V( s') ; s' = (next,pnext)
	return  reward + state_reward( afrom,  from_param );
}

template< class regression_tree, class action >
float	value_function<regression_tree, action>::value(  const	action_type &ato, const space_param_type &to_param  )const
{
	return  m_regression_trees[ ato.id() ]->reward( to_param ) ;
}

inline void set_random_buffer( buffer_vector<u32> &rnd_buff )
{
	const u32 size = rnd_buff.size();
	for( u32 p = 0; p < size; ++p ) rnd_buff[p] = p;
	std::random_shuffle( rnd_buff.begin(), rnd_buff.end() );
}


template< class regression_tree, class action >
float	value_function< regression_tree, action >::core_pi	( const	action_type &afrom, u32 from_blend_id, const	action_type &ato, u32 &blend_id, const space_param_type &from_param , space_param_type &to_param, float &max_v  )const
{

	u32 num_samples = ato.num_blend_samples();
	ASSERT( num_samples > 0 );

	buffer_vector<u32> rnd_samples( ALLOCA( sizeof(32) * num_samples ), num_samples, num_samples );
	set_random_buffer( rnd_samples );

	max_v = -math::infinity;
	blend_id = rnd_samples[0];
	for( u32 i = 0; i < num_samples; ++i )
	{
		space_param_type to = from_param;//( from_param.taget_position());
		
		//pair!
		ato.run( to, afrom, from_blend_id, rnd_samples[i] );
		
		float v = value( ato, to );

		if( v > max_v )
		{
			to_param = to;
			blend_id = rnd_samples[i];
			max_v = v;
		}
	}

	return afrom.reward( ato ) + discount * max_v;//core_pi( afrom, ato, to_param, v );
}

template< class regression_tree, class action >
const action*	value_function< regression_tree, action >::pi( const action_type &from, u32 from_blend_id, const space_param_type &from_params, u32 &blend_id ,space_param_type &to_params, float &max_reward, float &v ) const
{
	const u32 num_actions = m_action_base.size();
	ASSERT( num_actions != 0 );

//
	buffer_vector<u32> rnd_actions( ALLOCA( sizeof(32) * num_actions ), num_actions, num_actions );
	set_random_buffer( rnd_actions );
//

	const action	*ret = m_action_base.get( rnd_actions[0] ); 
	
	//max_reward = core_pi( from, *ret, from_params, to_params );
	max_reward =- math::infinity;
	for( u32 i=0; i < num_actions; ++i )
	{
		space_param_type p;
		const action	*a =  m_action_base.get( rnd_actions[ i ] ); 
		float val = 0;
		u32 blend = u32(-1);
		float reward = 	core_pi( from, from_blend_id,  *a, blend, from_params,  p, val );

		if( reward > max_reward )
		{
			ret = a;
			max_reward = reward;
			to_params = p;
			v = val;
			blend_id = blend;
		}
	}
	return ret;
}

template< class regression_tree, class action >
const action*	value_function< regression_tree, action >::pi( const action_type &from, u32 from_blend_id, const space_param_type &from_param, u32 &blend_id ) const
{
	
	space_param_type to_param;
	float reward, value;
	return pi( from, from_blend_id, from_param, blend_id, to_param, reward, value );
}

template< class regression_tree, class action >
void	value_function< regression_tree, action >::render(  xray::render::debug::renderer& r ) const
{
	typename rtp::vector< regression_tree_type* > ::const_iterator i, b = m_regression_trees.begin(), e = m_regression_trees.end();
	for( i = b ; i!=e; ++i )
		(*i)->render( r, *m_action_base.get(u32(i-b)) );
}

template< class regression_tree, class action >
void	value_function< regression_tree, action >::		clear( )
{
	typename rtp::vector< regression_tree_type* > ::iterator i = m_regression_trees.begin(), e = m_regression_trees.end();
	for( ; i!=e; ++i )
		DELETE(*i);
}

template< class regression_tree, class action >
void	value_function< regression_tree, action >::init_trees( )
{
	clear();
	const u32 sz = m_action_base.size();
	for( u32 i = 0; i<sz; ++i )
		m_regression_trees[i] = NEW(regression_tree_type)( );
}

template< class regression_tree, class action >
void	value_function< regression_tree, action >::save( xray::configs::lua_config_value cfg )const
{
	bool empty = m_regression_trees.empty();
	cfg["empty"] = empty;

	if( empty )
		return;

	const u32 sz = m_action_base.size();
	node_save_load_data<space_param_type> data;
	for( u32 i = 0; i<sz; ++i )
		m_regression_trees[i]->save( cfg["trees"][i], data );
}

template< class regression_tree, class action >
void	value_function< regression_tree, action >::load( const xray::configs::lua_config_value &cfg )
{
	clear();
	init_trees();

	if( cfg["empty"] )
		return;

	const u32 sz = m_action_base.size();
	node_save_load_data< space_param_type > data;
	for( u32 i = 0; i<sz; ++i )
		m_regression_trees[i]->load( cfg["trees"][i], data );
}

template< class regression_tree, class action >
bool value_function< regression_tree, action >::empty( )const
{
	if( m_regression_trees.empty() )
		return true;

	ASSERT( m_regression_trees.size() == m_action_base.size() );

	return false;

}


} // namespace rtp
} // namespace xray