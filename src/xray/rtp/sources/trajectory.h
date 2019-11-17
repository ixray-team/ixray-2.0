////////////////////////////////////////////////////////////////////////////
//	Created		: 18.05.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef TRAJECTORY_H_INCLUDED
#define TRAJECTORY_H_INCLUDED

#include "training_set.h"

namespace xray {

namespace render {
namespace debug {
	struct renderer;
} // namespace debug
} // namespace render


namespace rtp {

template< class action >
class trajectory {

private:
typedef	action												action_type;
typedef	typename action::space_param_type					space_param_type;
typedef	training_regression_tree< space_param_type >		regression_tree_type;

public:
typedef	typename regression_tree_type::training_sample_type	training_sample_type;

private:
typedef	std::pair<training_sample_type, const action* >		item_type;

public:
			trajectory		( u32 self_id );
	void	add				( const training_sample_type& sample, const action* a );
	void	add_to_sets		( vector< training_set< training_sample_type > > &sets );
	void	clear			( ){ m_samples.clear(); }
	u32		size			( ){ return m_samples.size(); }
	void	render			( xray::render::debug::renderer& renderer ) const;
	void	set_is_in_taget	( bool v ){ m_in_taget = v;}

private:
	vector< item_type > m_samples;
	u32					m_id;
	u32					m_in_taget;

}; // class trajectory

} // namespace rtp
} // namespace xray

#include "trajectory_inline.h"

#endif // #ifndef TRAJECTORY_H_INCLUDED