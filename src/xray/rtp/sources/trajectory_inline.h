////////////////////////////////////////////////////////////////////////////
//	Created		: 18.05.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef TRAJECTORY_INLINE_H_INCLUDED
#define TRAJECTORY_INLINE_H_INCLUDED

namespace xray {
namespace rtp {

	template< class action >
	trajectory<action>::trajectory	( u32 self_id ): m_id( self_id )
	{
		
	}

	template< class action >
	void trajectory<action>::add( const training_sample_type& sample, const action* a )
	{
		m_samples.push_back( item_type( sample, a ) );
	}

	template< class action >
	void	trajectory<action>::add_to_sets	( vector< training_set< training_sample_type > > &sets )
	{
		typename vector<item_type>::iterator i = m_samples.begin(), e = m_samples.end();
		
		for( ; i!=e; ++i )
		{
			item_type& it = *i;
			sets[it.second->id()].add(it.first);
		}
		//m_samples.clear();
	}

	template< class action >	
	void	trajectory<action>::render		( xray::render::debug::renderer& r ) const
	{
		if( m_samples.size() < 2 )
			return;

		typename vector<item_type>::const_iterator i = m_samples.begin(), e = m_samples.end();
		for( ; i!=e-1; ++i )
		{
			const item_type& it = *i;
			const item_type& next_it = *(i+1);

			
				
			if( m_in_taget && i == (e - 2) )
				it.first.first.render( next_it.first.first, r, math::color_xrgb( 255, 0, 0 ) );
			else
				it.first.first.render( next_it.first.first, r, math::color_xrgb( u8(m_in_taget) * 255, ( m_id + 100 ) % 256,  ( m_id + 200 ) % 256 ) );

			it.second->render( it.first.first, r  );
		}
	

	}

} // namespace rtp
} // namespace xray

#endif // #ifndef TRAJECTORY_INLINE_H_INCLUDED