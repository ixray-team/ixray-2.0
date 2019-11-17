////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef N_ARY_TREE_COMPARER_H_INCLUDED
#define N_ARY_TREE_COMPARER_H_INCLUDED

#include "mixing_n_ary_tree_double_dispatcher.h"

namespace xray {
namespace animation {

struct base_interpolator;
class bone_mixer;
	
namespace mixing {

class n_ary_tree;
struct binary_tree_base_node;

class n_ary_tree_comparer : public n_ary_tree_double_dispatcher {
public:
					n_ary_tree_comparer		(
						n_ary_tree const& from,
						n_ary_tree const& to
					);
			bool	equal					( ) const;
			u32		needed_buffer_size		( ) const;
	n_ary_tree computed_tree				( mutable_buffer& buffer, xray::memory::base_allocator* allocator, bone_mixer& bone_mixer, u32 const current_time_in_ms );

private:
					n_ary_tree_comparer		( n_ary_tree_comparer const& );
	n_ary_tree_comparer& operator =			( n_ary_tree_comparer const& );

private:
	virtual	void	dispatch				( n_ary_tree_animation_node& left,		n_ary_tree_animation_node& right );
	virtual	void	dispatch				( n_ary_tree_animation_node& left,		n_ary_tree_transition_node& right );
	virtual	void	dispatch				( n_ary_tree_animation_node& left,		n_ary_tree_weight_node& right );
	virtual	void	dispatch				( n_ary_tree_animation_node& left,		n_ary_tree_addition_node& right );
	virtual	void	dispatch				( n_ary_tree_animation_node& left,		n_ary_tree_subtraction_node& right );
	virtual	void	dispatch				( n_ary_tree_animation_node& left,		n_ary_tree_multiplication_node& right );

	virtual	void	dispatch				( n_ary_tree_transition_node& left,		n_ary_tree_animation_node& right );
	virtual	void	dispatch				( n_ary_tree_transition_node& left,		n_ary_tree_transition_node& right );
	virtual	void	dispatch				( n_ary_tree_transition_node& left,		n_ary_tree_weight_node& right );
	virtual	void	dispatch				( n_ary_tree_transition_node& left,		n_ary_tree_addition_node& right );
	virtual	void	dispatch				( n_ary_tree_transition_node& left,		n_ary_tree_subtraction_node& right );
	virtual	void	dispatch				( n_ary_tree_transition_node& left,		n_ary_tree_multiplication_node& right );

	virtual	void	dispatch				( n_ary_tree_weight_node& left,			n_ary_tree_animation_node& right );
	virtual	void	dispatch				( n_ary_tree_weight_node& left,			n_ary_tree_transition_node& right );
	virtual	void	dispatch				( n_ary_tree_weight_node& left,			n_ary_tree_weight_node& right );
	virtual	void	dispatch				( n_ary_tree_weight_node& left,			n_ary_tree_addition_node& right );
	virtual	void	dispatch				( n_ary_tree_weight_node& left,			n_ary_tree_subtraction_node& right );
	virtual	void	dispatch				( n_ary_tree_weight_node& left,			n_ary_tree_multiplication_node& right );

	virtual	void	dispatch				( n_ary_tree_addition_node& left,		n_ary_tree_animation_node& right );
	virtual	void	dispatch				( n_ary_tree_addition_node& left,		n_ary_tree_transition_node& right );
	virtual	void	dispatch				( n_ary_tree_addition_node& left,		n_ary_tree_weight_node& right );
	virtual	void	dispatch				( n_ary_tree_addition_node& left,		n_ary_tree_addition_node& right );
	virtual	void	dispatch				( n_ary_tree_addition_node& left,		n_ary_tree_subtraction_node& right );
	virtual	void	dispatch				( n_ary_tree_addition_node& left,		n_ary_tree_multiplication_node& right );

	virtual	void	dispatch				( n_ary_tree_subtraction_node& left,		n_ary_tree_animation_node& right );
	virtual	void	dispatch				( n_ary_tree_subtraction_node& left,		n_ary_tree_transition_node& right );
	virtual	void	dispatch				( n_ary_tree_subtraction_node& left,		n_ary_tree_weight_node& right );
	virtual	void	dispatch				( n_ary_tree_subtraction_node& left,		n_ary_tree_addition_node& right );
	virtual	void	dispatch				( n_ary_tree_subtraction_node& left,		n_ary_tree_subtraction_node& right );
	virtual	void	dispatch				( n_ary_tree_subtraction_node& left,		n_ary_tree_multiplication_node& right );

	virtual	void	dispatch				( n_ary_tree_multiplication_node& left,	n_ary_tree_animation_node& right );
	virtual	void	dispatch				( n_ary_tree_multiplication_node& left,	n_ary_tree_transition_node& right );
	virtual	void	dispatch				( n_ary_tree_multiplication_node& left,	n_ary_tree_weight_node& right );
	virtual	void	dispatch				( n_ary_tree_multiplication_node& left,	n_ary_tree_addition_node& right );
	virtual	void	dispatch				( n_ary_tree_multiplication_node& left,	n_ary_tree_subtraction_node& right );
	virtual	void	dispatch				( n_ary_tree_multiplication_node& left,	n_ary_tree_multiplication_node& right );

private:
			void	process_interpolators	(
						n_ary_tree const& from,
						n_ary_tree const& to
					);
			void	add_transition_node		(
						u32 const left_unique_multipliers,
						u32 const right_unique_multipliers
					);
	template < typename T >
	inline	void	propagate				( T& left, T& right );

	template < typename T >
	inline	void	increase_buffer_size	( T& node );
			void	increase_buffer_size	( n_ary_tree_base_node& node );

private:
	n_ary_tree const&	m_from;
	n_ary_tree const&	m_to;
	base_interpolator const*	m_additive_interpolator;
	base_interpolator const*	m_non_additive_interpolator;
	base_interpolator const**	m_current_interpolator;
	u32							m_animations_count;
	u32							m_needed_buffer_size;
	bool						m_equal;
}; // class n_ary_tree_comparer

} // namespace mixing
} // namespace animation
} // namespace xray

#endif // #ifndef N_ARY_TREE_COMPARER_H_INCLUDED