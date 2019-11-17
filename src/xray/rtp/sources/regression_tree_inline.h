////////////////////////////////////////////////////////////////////////////
//	Created		: 09.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

namespace xray {
namespace rtp {

template< class space_param_type > 
void regression_tree<space_param_type>::save( xray::configs::lua_config_value cfg, const  node_save_load_data<space_param_type>& data  )const
{
	m_root.save( cfg, data );
}

template< class space_param_type > 
void	regression_tree<space_param_type>::load( const xray::configs::lua_config_value &cfg,  node_save_load_data<space_param_type>& data  )
{
	m_root.load( cfg, data );
}

template< class space_param_type > 
node< space_param_type >::node():
			m_split_dimension( u32(-1) ),
			m_split_value( math::QNaN ),
			left( 0 ), right( 0 ),//, parent( prnt ),
			m_reward_value( 0 )
{
	
}

template< class space_param_type > 
node< space_param_type >::~node( )
{
	DELETE( left );
	DELETE( right );
}

template< class space_param_type > 
bool	node< space_param_type >::is_leaf( ) const
{
	//dbg_check_validity	( );
	return !left;
}

template< class space_param_type >
float	node< space_param_type >::reward( const space_param_type &params  ) const
{
	if( is_leaf( ) )
		return m_reward_value;
	if( params[ m_split_dimension ] < m_split_value ) // <= ?
		return left->reward( params );
	else
		return right->reward( params );
}

template< class space_param_type >
node< space_param_type > * node<space_param_type>::create		(  u32   )
{
	return NEW(node< space_param_type >)(  );
}

template< class space_param_type > 	
void	node< space_param_type >::save( xray::configs::lua_config_value cfg, const  node_save_load_data< space_param_type >& data  )const
{
		cfg["leaf"] = is_leaf();
		if( !is_leaf() )
		{
			cfg["split_dimension"] = m_split_dimension;
			cfg["split_value"] = m_split_value;
			left->save( cfg["left"], data );
			right->save( cfg["right"], data );
		} else
			cfg["reward_value"] = m_reward_value;

}

template< class space_param_type > 
void	node< space_param_type >::load( const xray::configs::lua_config_value &cfg, node_save_load_data< space_param_type >& data  )
{
		bool leaf = cfg["leaf"];
		if( !leaf )
		{
			m_split_dimension = cfg["split_dimension"] ;
			m_split_value = cfg["split_value"];
			DELETE(left);
			DELETE(right);
			left	= create( level()+1 );
			right	= create( level()+1 );
			left	->load( cfg["left"], data );
			right	->load( cfg["right"], data );
		} else
		{
			ASSERT(!left);
			ASSERT(!right);
			m_reward_value = cfg["reward_value"];
		}
}

} // namespace rtp
} // namespace xray