////////////////////////////////////////////////////////////////////////////
//	Created		: 27.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TRAINING_SET_H_INCLUDED
#define TRAINING_SET_H_INCLUDED

namespace xray {

namespace configs{
class lua_config_value;
} // namespace configs

namespace rtp {

template < class training_sample_type >
class training_set
{

public:
												training_set		( );
	void										add					( training_sample_type const& sumple );

	rtp::vector< training_sample_type >			&samples			( ){ return m_samples; }
	const rtp::vector< training_sample_type >	&samples			( )const{ return m_samples; }


	void										randomize			( );

	u32											&min_in_leaf_samples( ){ return m_min_in_leaf_samples ;}

public:
	void										save				( xray::configs::lua_config_value cfg )const;
	void										load				( const xray::configs::lua_config_value &cfg );

public:
	void										synchronize			();

private:
	u32											m_min_in_leaf_samples;							
	rtp::vector< training_sample_type >			m_samples;
	rtp::vector< training_sample_type >			m_add_samples;

	static const u32							min_in_leaf_samples_default = 5;

}; // class training_set

} // namespace rtp
} // namespace xray

#include "training_set_inline.h"

#endif // #ifndef TRAINING_SET_H_INCLUDED