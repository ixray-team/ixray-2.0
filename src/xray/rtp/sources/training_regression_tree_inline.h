////////////////////////////////////////////////////////////////////////////
//	Created		: 24.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "rtp_world.h"

namespace xray {
namespace rtp {

template< class space_param_type >
training_regression_tree<space_param_type>::training_regression_tree( 	training_set<training_sample_type>& set )
:super( set.min_in_leaf_samples() ),
m_num_samples( 0 )
{
	add_samples( set.samples() );
}

template< class space_param_type >
training_regression_tree<space_param_type>::training_regression_tree( u32 min_samples, buffer_vector<const training_sample_type*>	&samples )
:super( min_samples ),
m_num_samples( 0 )
{
	m_num_samples = 0;
	const u32 sz = samples.size();
	for( u32 i = 0; i< sz; ++i )
	{
		
		add_sample( samples[i] );
	}


	m_root.set_samples( &m_training_samples_buff[ 0 ],  &m_training_samples_buff[ m_num_samples ] );
}

template< class space_param_type >
void	training_regression_tree<space_param_type>::add_samples( vector< training_sample_type > &samples )
{
	m_num_samples = 0;
	const u32 sz = samples.size();
	
	for( u32 i = 0; i< sz; ++i )
		add_sample( &samples[i] );

	m_root.set_samples( &m_training_samples_buff[ 0 ],  &m_training_samples_buff[ m_num_samples ] );
	
}

template< class space_param_type >
void	training_regression_tree<space_param_type>::add_sample( const training_sample_type *s )
{
		
		R_ASSERT( m_num_samples < samples_max - 1 );

		m_training_samples_buff[ m_num_samples ] = s;	
		++m_num_samples;
}


template< class training_sample_type >	
struct sfnd
{
	sfnd(const training_sample_type* p ):ptr( p ){}
	const training_sample_type* ptr;

	bool operator ()( const training_sample_type &sample )
	{
		return &sample == ptr;
	}
} ;

template< class space_param_type >
void	training_regression_tree<space_param_type>::save		( xray::configs::lua_config_value cfg, const node_save_load_data< space_param_type >& data  )const
{
	
	super::save( cfg, data );

	xray::configs::lua_config_value lcfg = cfg["sample_idx"];
	ASSERT( data.get_training_save_load_data() );
	const rtp::vector< training_sample_type > &samples = data.get_training_save_load_data()->samples();
	for(u32 i = 0; i < num_samples(); ++i )
	{
		
		sfnd<training_sample_type> fnd( p_sample( i ) );
		
		typename rtp::vector<training_sample_type>::const_iterator s_it = std::find_if( samples.begin(), samples.end(), fnd );
		ASSERT( s_it != samples.end() );
		lcfg[i] = u32( s_it - samples.begin() );
	}


}

template< class space_param_type >
void training_regression_tree<space_param_type>::load		( const xray::configs::lua_config_value &cfg, node_save_load_data< space_param_type >& data )
{
	super::load( cfg, data );
	xray::configs::lua_config_value lcfg = cfg["sample_idx"];

	ASSERT( data.get_training_save_load_data() );
	const rtp::vector< training_sample_type > &samples = data.get_training_save_load_data()->samples();
	
	for(u32 i = 0; i < num_samples(); ++i )
			m_training_samples_buff[i] = &samples[lcfg[i]];
}


template< class space_param_type >	
void	training_regression_tree<space_param_type>::render( xray::render::debug::renderer& r, const iaction<space_param_type> &a ) const
{
	m_root.render( r, a );
}

template< class space_param_type >
const typename training_regression_tree<space_param_type>::training_sample_type* const&training_regression_tree<space_param_type>::p_sample( u32 i )const
{
	R_ASSERT( m_num_samples > i );
	return m_training_samples_buff[ i ];
}

template< class space_param_type >
const typename training_regression_tree<space_param_type>::training_sample_type* &training_regression_tree<space_param_type>::p_sample( u32 i )
{
	R_ASSERT( m_num_samples > i );
	return m_training_samples_buff[ i ];
}



inline void get_min_max( float v, float &min, float &max )
{
	if( v > max )
		max = v;
	if( v < min )
		min = v;
}

//template< class training_sample_type, class space_param_type > 
//inline void get_min_max( typename training_node<space_param_type>::node_sapmles_vec_type **samples, u32 sz, space_param_type &min, space_param_type& max  )
//{
//	//const	u32 sz =	samples.size();
//	for( u32 i = 0; i < space_param_type::dimensions_number; ++i )
//	{
//		min[i] = math::infinity;
//		max[i] = -math::infinity; 
//		for( u32 j = 0; j < sz; ++j  )
//			get_min_max( samples[j]->first[i], min[i], max[i] );
//			
//	}
//}

template< class space_param_type >	
void	training_regression_tree<space_param_type>::build	( )
{ 
	while ( !m_root.build	( ) ) {}; 
}

template< class space_param_type > 
inline float variance( const  typename training_node< space_param_type >::training_sample_type  *const* samples, u32 from, u32 to )
{ 
	//ASSERT( !samples.empty() );
	ASSERT( from < to );

	//ASSERT( from < samples.size() );
	//ASSERT( to <= samples.size() );

	float min = math::float_max;
	float max = math::float_min;

	for(u32 i = from; i < to; ++i )
	{
		if( samples[i]->second > max )
			max = samples[i]->second;
		if( samples[i]->second < min )
			min = samples[i]->second;
	}
	return max - min;
}

template< class space_param_type > 
inline float get_split_variance( const  typename training_node< space_param_type >::training_sample_type  *const*samples, u32 sz, u32 dimension, float split )
{
	
	float l_max = -math::infinity; 
	float r_max = -math::infinity;
	float l_min = math::infinity; 
	float r_min = math::infinity;
	u32 l_num =0	;
	u32 r_num =0	;

	for( u32 j = 0; j < sz; ++j  )
	{
		
		if( samples[j]->first[dimension] < split )
		{
			get_min_max( samples[j]->second, l_min, l_max );
			++l_num;
		}
		else
		{
			get_min_max( samples[j]->second, r_min, r_max );
			++r_num;
		}
	}
	//float ret = 0;
	float l_variance	=0;
	float r_variance	=0;

	if( l_num>0 )
		l_variance= l_max-l_min;

	if( r_num>0 )
		r_variance= r_max-r_min;

	float node_min = math::min( l_min, r_min );
	float node_max = math::max( l_max, r_max );
	
	float node_variance = node_max - node_min ;

	float score =  ( node_variance -( float( l_num ) / float( sz ) ) * l_variance -
									( float( r_num ) / float( sz ) ) * r_variance 
					) / node_variance;
	

	return score;
}

template< class training_sample_type >
struct partition_predicate {

	partition_predicate( u32 dimension, float split ): m_dimension( dimension ), m_split( split ) {}

	bool operator () ( const training_sample_type * sampl )
	{
		return sampl->first[m_dimension] < m_split;
	}
	u32		m_dimension;
	float	m_split;
}; // struct partition_predicate

template< class space_param_type > 
inline  const typename training_node< space_param_type >::training_sample_type**
split_by(	const	typename training_node< space_param_type >::training_sample_type  **samples_begin,
			const	typename training_node< space_param_type >::training_sample_type  **samples_end,
			u32 dimension, float split )
{
	partition_predicate<typename training_node< space_param_type >::training_sample_type> pred( dimension, split );
	return std::partition( samples_begin, samples_end, pred );
}

template< class space_param_type > 
void	training_node< space_param_type >::init_build	( )
{
	get_min_max( m_training_samples, m_min, m_max );
}


//template< class space_param_type > 
//void training_node< space_param_type >::sort_by_dimension( node_sapmles_vec_type &samples, u32 dimension )
//{
//	
//	struct cmp
//	{
//		u32 m_dimension;
//		cmp( u32 dimension ): m_dimension( dimension ){}
//		bool operator () ( const training_node< space_param_type >::training_sample_type* l, const training_node< space_param_type >::training_sample_type* r )
//		{
//			return l->first[m_dimension] < r->first[m_dimension];
//		}
//	};
//	std::sort( samples.begin(), samples.end(), cmp(dimension) );
//}

template< class space_param_type > 
training_node< space_param_type >::training_node	( u32 level, const training_sample_type** samples_begin, const training_sample_type** samples_end, const space_param_type &min, const space_param_type &max, u32 min_samples ):
			node< space_param_type >(),
			m_min(min),
			m_max(max),
			m_samples_begin	( samples_begin ),
			m_samples_end	( samples_end ),
			m_level(level),
			m_min_samples( min_samples )
{
	
}

template< class space_param_type > 
training_node< space_param_type >::training_node	( u32 level, u32 min_samples  ):
node< space_param_type >(),
m_min( space_param_type::area_min ),
m_max( space_param_type::area_max ),
m_samples_begin	( 0 ),
m_samples_end	( 0 ),
m_level(level),
m_min_samples( min_samples )
{
	
}

template< class space_param_type > 
training_node< space_param_type >::training_node( u32 level ):
m_min( space_param_type::area_min ),
m_max( space_param_type::area_max ),
m_samples_begin	( 0 ),
m_samples_end	( 0 ),
m_level			( level ),
m_min_samples( u32(-1) )
{

}

template< class space_param_type >
node<space_param_type>* training_node< space_param_type >::create( u32 level )
{
	return NEW(training_node<space_param_type>)( level );
}


template< class space_param_type > 
training_node< space_param_type >::~training_node( )
{

}	

template< class space_param_type >
void	training_node< space_param_type >::set_samples	( const training_sample_type **begin, const training_sample_type **end )
{ 
	ASSERT( 0 == m_samples_begin );
	ASSERT( 0 == m_samples_end );
	m_samples_begin = begin;
	m_samples_end	= end;
}



template< class space_param_type > 
bool training_node<space_param_type>::build()
{
	if( !super::is_leaf( ) )
	{
		ASSERT( super::left );
		ASSERT( super::right );
		ASSERT( samples_empty() );
		if( math::is_zero( super::m_split_value, math::epsilon_7 ) )
			super::m_split_value = 0.f;
		ASSERT( math::valid(super::m_split_value) );
		ASSERT( super::m_split_dimension != u32(-1) );
		ASSERT( super::m_split_dimension < space_param_type::dimensions_number );
		return super::left->build() && super::right->build();
	} else
		return divide();
}

template< class space_param_type > 
bool training_node<space_param_type>::recalculate( )
{
	if( !is_leaf( ) )
	{
		ASSERT( left );
		ASSERT( right );
		ASSERT( samples_empty() );
		ASSERT( math::valid(m_split_value) );
		ASSERT( m_split_dimension != u32(-1) );
		ASSERT( m_split_dimension < space_param_type::dimensions_number );
		return left->recalculate() && right->recalculate();
	}
	compute_reward		( );
	return true;
}

template< class space_param_type >
void	training_node<space_param_type>::compute_reward( )
{
	m_reward_value = 0;
	for( u32 i = 0; i< samples_size();  ++i )
		m_reward_value += m_samples_begin[i]->second;
	m_reward_value /= float( samples_size() );
}





template< class space_param_type >
bool	training_node<space_param_type>::divide()
{
		ASSERT( !left );
		ASSERT( !right );
//		ASSERT( !m_training_samples.empty() );
		ASSERT( !math::valid( m_split_value ) );
		ASSERT( m_split_dimension == u32(-1) );
		
		if( samples_empty() )
			return true;
		if( samples_size() <= m_min_samples || m_level > max_tree_depth )
		{
			
			if( m_level > max_tree_depth )
				LOG_DEBUG( "level max_tree_depth: %d exceeded ", max_tree_depth );

			compute_reward		( );
			return true;
		}
		float node_variance = variance<space_param_type>( m_samples_begin, 0, samples_size() );
		if( node_variance == 0.f )
		{
			compute_reward		( );
			return true;
		}
//		const float size	= float( m_training_samples.size() );
//		const float rp_sise = 1.f/size;

		float max_score		= -math::infinity;
		//u32   max_split_pos = u32(-1);
		float max_split		= -math::infinity;
		u32   max_dimension = u32(-1);
		//rtp::vector< const training_sample_type* >			max_sorted;
		for( u32 i = 0; i < space_param_type::dimensions_number; ++i )
		{
			
			float split = random( m_min[i], m_max[i], rtp_world::random() );
			float relative_var = get_split_variance<space_param_type>( m_samples_begin, samples_size(), i, split );
			float score =  relative_var;//( node_variance*size -	relative_var  );// / node_variance;
			if( score > max_score )
			{
				 max_score		= score;
				 max_split		= split;
				 max_dimension  = i;
			}

			//sort_by_dimension( m_training_samples, i );

			//for( u32 j = 1; j < m_training_samples.size() - 1; ++j  )
			//{
			//	float l_variance = variance( m_training_samples, 0, j );
			//	float r_variance = variance( m_training_samples, j, m_training_samples.size() );
			//	
			//	

			//	float score =  ( node_variance -	( float(j) * rp_sise ) * l_variance -
			//										( float(m_training_samples.size()-j) * rp_sise ) * r_variance 
			//										) / node_variance;
			//	if( score > max_score )
			//	{
			//		max_score = score;
			//		max_split_pos = j;
			//		max_dimension = i;
			//		max_sorted = m_training_samples;
			//	}
			//}
			
			//(Nl*Lvar + Nr*Rvar)


		}

		ASSERT( max_dimension != u32(-1) );
		m_split_dimension	= max_dimension;
		//m_split_value		= 0.5f *
		//( max_sorted[max_split_pos]->first[max_dimension] + max_sorted[max_split_pos-1]->first[max_dimension] );
		m_split_value = max_split;

		const training_sample_type		**samples_split =
				split_by<space_param_type>( m_samples_begin, m_samples_end, m_split_dimension , m_split_value );
		
		space_param_type l_min = m_min;
		space_param_type l_max = m_max;
		l_max[ m_split_dimension ] = m_split_value;

		left		=		NEW( training_node<space_param_type> )( level()+1, m_samples_begin, samples_split, l_min, l_max, m_min_samples  );

		space_param_type r_min = m_min;
		space_param_type r_max = m_max;
		r_min[ m_split_dimension ] = m_split_value;
		right		=		NEW( training_node<space_param_type> )( level()+1, samples_split, m_samples_end, r_min, r_max, m_min_samples  );

		m_samples_begin = 0;
		m_samples_end	= 0;

		return false;
}

template< class space_param_type >
void	training_node< space_param_type >::dbg_check_validity( ) const
{
	if( !left ) //is_leaf();
	{
		ASSERT( !left );
		ASSERT( !right );
		//ASSERT( !m_training_samples.empty() );
		ASSERT( !math::valid(m_split_value) );
		ASSERT( m_split_dimension == u32(-1) );
	} else
	{
		ASSERT( left );
		ASSERT( right );
		ASSERT( m_training_samples.empty() );
		ASSERT( math::valid(m_split_value) );
		ASSERT( m_split_dimension != u32(-1) );
		ASSERT( m_split_dimension < space_param_type::dimensions_number );
	}

}

template< class space_param_type >	
void	training_node<space_param_type>::render( xray::render::debug::renderer& r, const iaction_type &a ) const
{
	if( is_leaf( ) )
	{
		for( u32 i = 0; i< samples_size();  ++i )
			a.render( m_samples_begin[i]->first, r );
		return;
	}
	ASSERT( left );
	ASSERT( right );

	training_node<space_param_type> *lt = static_cast_checked< training_node<space_param_type>* >( left ); 
	training_node<space_param_type> *rt = static_cast_checked< training_node<space_param_type>* >( left ); 
	
	ASSERT( lt );
	ASSERT( rt );

	lt->render( r, a );
	rt->render( r, a );


}



template< class space_param_type >	
void	training_node<space_param_type>::save( xray::configs::lua_config_value cfg, const node_save_load_data<space_param_type>& data  )const
{
	
	super::save( cfg, data );
	if(!data.get_training_save_load_data())
		return;

	ASSERT( data.get_training_save_load_data() );
	
	m_min.save( cfg["min"] );
	m_max.save( cfg["max"] );
	cfg["min_samples"] = m_min_samples;


	training_sample_type const* const* tree_begin = &data.get_training_save_load_data()->tree().p_sample( 0 );

	u32 begin	=  m_samples_begin ? u32( m_samples_begin - tree_begin ) : 0;
	u32 end		=  m_samples_end ? u32( m_samples_end - tree_begin ) : 0;

	cfg["samples"]["begin"]		= begin;
	cfg["samples"]["end"]		= end;

}

template< class space_param_type >	
void	training_node< space_param_type >::load( const xray::configs::lua_config_value &cfg, node_save_load_data< space_param_type >& data  )
{
	super::load( cfg, data );
	if(!data.get_training_save_load_data())
		return;
	ASSERT( data.get_training_save_load_data() );

	
	m_min.load( cfg["min"] );
	m_max.load( cfg["max"] );

	m_min_samples = cfg["min_samples"];

	
	u32 begin	= u32(  cfg["samples"]["begin"] ) ;
	u32 end		=  u32( cfg["samples"]["end"] ) ;

	m_samples_begin = (	begin	== 0 ? 0 :  &data.get_training_save_load_data()->tree().p_sample(  u32( cfg["samples"]["begin"] ) - 1 ) + 1 );
	m_samples_end	= (	end		== 0 ? 0 :	&data.get_training_save_load_data()->tree().p_sample(  u32( cfg["samples"]["end"]	) - 1 ) + 1 );

}

} // namespace rtp
} // namespace xray