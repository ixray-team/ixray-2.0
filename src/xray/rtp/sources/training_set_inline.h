////////////////////////////////////////////////////////////////////////////
//	Created		: 27.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

namespace xray {
namespace rtp {




template < class training_sample_type >
inline training_set< training_sample_type >::training_set( ) :
	m_min_in_leaf_samples( min_in_leaf_samples_default )
{

}


template < class training_sample_type >
inline void training_set< training_sample_type >::synchronize			()
{
	m_samples.insert( m_samples.end(), m_add_samples.begin(), m_add_samples.end() );
	m_add_samples.clear();
}


template < class training_sample_type >
inline void training_set< training_sample_type >::add( training_sample_type const& sumple )
{
	//m_samples.push_back( sumple );
	m_add_samples.push_back( sumple );
}

template < class training_sample_type >
inline void	training_set< training_sample_type >::randomize( )
{
	std::random_shuffle( m_samples.begin(), m_samples.end() );
}

template < class training_sample_type >
inline void training_set< training_sample_type >::save( xray::configs::lua_config_value cfg )const
{
	cfg["min_samples"] = m_min_in_leaf_samples;
	const u32 num_samples = m_samples.size();
	cfg["num_samples"] = num_samples;
	for( u32 i = 0; i<num_samples; ++i )
	{
		m_samples[i].first.save( cfg["samples"][i]["param"] );
		cfg["samples"][i]["value"] = m_samples[i].second;
	}
}

template < class training_sample_type >
inline void training_set< training_sample_type >::load( const xray::configs::lua_config_value &cfg )
{
	m_min_in_leaf_samples = cfg["min_samples"];
	const u32 num_samples = cfg["num_samples"];

	m_samples.resize( num_samples );
	for( u32 i = 0; i<num_samples; ++i )
	{
		m_samples[i].first.load( cfg["samples"][i]["param"] );
		m_samples[i].second = cfg["samples"][i]["value"];
	}
}

} // namespace rtp
} // namespace xray