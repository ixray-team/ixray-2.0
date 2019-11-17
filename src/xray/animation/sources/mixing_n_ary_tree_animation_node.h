////////////////////////////////////////////////////////////////////////////
//	Created		: 03.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef N_ARY_TREE_ANIMATION_NODE_H_INCLUDED
#define N_ARY_TREE_ANIMATION_NODE_H_INCLUDED

#include "mixing_n_ary_tree_n_ary_operation_node.h"
#include "mixing_animation_clip.h"
#include "bone_mixer_data.h"

namespace xray {
namespace animation {
namespace mixing {

class n_ary_tree_animation_node : public n_ary_tree_n_ary_operation_node {
public:
	inline			n_ary_tree_animation_node	(
						animation_clip const& animation,
						float const time_scale,
						u32 const operands_count,
						u32 const start_time_in_ms,
						bool const is_transiting_to_zero
					);
	inline			n_ary_tree_animation_node	(
						animation_clip const& animation,
						bone_mixer_data const& bone_mixer_data,
						float const time_scale,
						u32 const operands_count,
						u32 const start_time_in_ms,
						bool const is_transiting_to_zero
					);
	inline			n_ary_tree_animation_node	( n_ary_tree_animation_node const& other );
	inline	animation_clip const&	animation	( ) const;
	inline	bone_mixer_data& bone_mixer_data	( );
	inline	float	time_scale					( ) const;
	inline	u32		start_time_in_ms			( ) const;
	inline	bool	is_transiting_to_zero		( ) const;
	inline	bool	operator ==					( n_ary_tree_animation_node const& other ) const;
	inline	bool	operator !=					( n_ary_tree_animation_node const& other ) const;
	virtual	void	accept						( n_ary_tree_visitor& visitor );
	virtual void	accept						( n_ary_tree_double_dispatcher& dispatcher, n_ary_tree_base_node& node );

private:
	n_ary_tree_animation_node& operator=		( n_ary_tree_animation_node const& other);
	virtual void	visit						( n_ary_tree_double_dispatcher& dispatcher, n_ary_tree_animation_node& node );
	virtual void	visit						( n_ary_tree_double_dispatcher& dispatcher, n_ary_tree_weight_node& node );
	virtual void	visit						( n_ary_tree_double_dispatcher& dispatcher, n_ary_tree_addition_node& node );
	virtual void	visit						( n_ary_tree_double_dispatcher& dispatcher, n_ary_tree_subtraction_node& node );
	virtual void	visit						( n_ary_tree_double_dispatcher& dispatcher, n_ary_tree_multiplication_node& node );
	virtual void	visit						( n_ary_tree_double_dispatcher& dispatcher, n_ary_tree_transition_node& node );

private:
	animation::bone_mixer_data	m_bone_mixer_data;
	animation_clip const&		m_animation;

public:
	n_ary_tree_animation_node*	m_next_animation;

private:
	float const					m_time_scale;
	u32	const					m_start_time_in_ms;
	bool const					m_is_transiting_to_zero;
private:
#ifdef DEBUG
#pragma warning(push)
#pragma warning(disable:4200)
	n_ary_tree_base_node*		m_operands[];
#pragma warning(pop)
#endif // #ifdef DEBUG
}; // class n_ary_tree_animation_node

	inline	bool	operator <					( n_ary_tree_animation_node const& left, n_ary_tree_animation_node const& right );

} // namespace mixing
} // namespace animation
} // namespace xray

#include "mixing_n_ary_tree_animation_node_inline.h"

#endif // #ifndef N_ARY_TREE_ANIMATION_NODE_H_INCLUDED