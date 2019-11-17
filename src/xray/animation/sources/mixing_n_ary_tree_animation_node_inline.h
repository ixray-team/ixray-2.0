////////////////////////////////////////////////////////////////////////////
//	Created		: 26.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef N_ARY_TREE_ANIMATION_NODE_INLINE_H_INCLUDED
#define N_ARY_TREE_ANIMATION_NODE_INLINE_H_INCLUDED

namespace xray {
namespace animation {
namespace mixing {

inline n_ary_tree_animation_node::n_ary_tree_animation_node	(
		animation_clip const& animation,
		float const time_scale, 
		u32 const operands_count,
		u32 const start_time_in_ms,
		bool const is_transiting_to_zero
	) :
	n_ary_tree_n_ary_operation_node	( operands_count ),
	m_animation			( animation ),
	m_next_animation	( 0 ),
	m_time_scale		( time_scale ),
	m_start_time_in_ms	( start_time_in_ms ),
	m_is_transiting_to_zero	( is_transiting_to_zero )
{
}

inline n_ary_tree_animation_node::n_ary_tree_animation_node	(
		animation_clip const& animation,
		animation::bone_mixer_data const& bone_mixer_data, 
		float const time_scale, 
		u32 const operands_count,
		u32 const start_time_in_ms,
		bool const is_transiting_to_zero
	) :
	n_ary_tree_n_ary_operation_node	( operands_count ),
	m_bone_mixer_data	( bone_mixer_data ),
	m_animation			( animation ),
	m_next_animation	( 0 ),
	m_time_scale		( time_scale ),
	m_start_time_in_ms	( start_time_in_ms ),
	m_is_transiting_to_zero	( is_transiting_to_zero )
{
}

inline n_ary_tree_animation_node::n_ary_tree_animation_node	( n_ary_tree_animation_node const& other ) :
	n_ary_tree_n_ary_operation_node	( other ),
	m_bone_mixer_data	( other.m_bone_mixer_data ),
	m_animation			( other.m_animation ),
	m_next_animation	( 0 ),
	m_time_scale		( other.m_time_scale ),
	m_start_time_in_ms	( other.m_start_time_in_ms ),
	m_is_transiting_to_zero	( other.m_is_transiting_to_zero )
{
}

inline animation_clip const& n_ary_tree_animation_node::animation	( ) const
{
	return				m_animation;
}

inline animation::bone_mixer_data& n_ary_tree_animation_node::bone_mixer_data	( )
{
	return				m_bone_mixer_data;
}

inline float n_ary_tree_animation_node::time_scale					( ) const
{
	return				m_time_scale;
}

inline u32 n_ary_tree_animation_node::start_time_in_ms				( ) const
{
	ASSERT				( m_start_time_in_ms != u32(-1) );
	return				m_start_time_in_ms;
}

inline bool n_ary_tree_animation_node::is_transiting_to_zero			( ) const
{
	return				m_is_transiting_to_zero;
}

inline bool n_ary_tree_animation_node::operator ==					( n_ary_tree_animation_node const& other ) const
{
	return				other.animation() == animation();
}

inline bool	n_ary_tree_animation_node::operator !=					( n_ary_tree_animation_node const& other ) const
{
	return				!(*this == other);
}

inline bool operator <	( n_ary_tree_animation_node const& left, n_ary_tree_animation_node const& right )
{
	return				left.animation() < right.animation();
}

} // namespace mixing
} // namespace animation
} // namespace xray

#endif // #ifndef N_ARY_TREE_ANIMATION_NODE_INLINE_H_INCLUDED