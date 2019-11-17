////////////////////////////////////////////////////////////////////////////
//	Created		: 29.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "mixing_n_ary_tree_interpolator_selector.h"
#include "mixing_n_ary_tree_animation_node.h"
#include "mixing_n_ary_tree_transition_node.h"
#include "mixing_n_ary_tree_weight_node.h"
#include "mixing_n_ary_tree_addition_node.h"
#include "mixing_n_ary_tree_subtraction_node.h"
#include "mixing_n_ary_tree_multiplication_node.h"
#include "mixing_animation_clip.h"

using xray::animation::mixing::n_ary_tree_interpolator_selector;
using xray::animation::mixing::n_ary_tree_animation_node;
using xray::animation::mixing::n_ary_tree_transition_node;
using xray::animation::mixing::n_ary_tree_weight_node;
using xray::animation::mixing::n_ary_tree_addition_node;
using xray::animation::mixing::n_ary_tree_multiplication_node;
using xray::animation::mixing::n_ary_tree_subtraction_node;
using xray::animation::base_interpolator;

void n_ary_tree_interpolator_selector::update_interpolator	( base_interpolator const& interpolator )
{
	if ( !m_result ) {
		m_result		= &interpolator;
		return;
	}

	if ( m_result->transition_time() < interpolator.transition_time() )
		m_result		= &interpolator;
}

void n_ary_tree_interpolator_selector::visit				( n_ary_tree_animation_node& node )
{
	update_interpolator	( node.animation().interpolator() );
}

void n_ary_tree_interpolator_selector::visit				( n_ary_tree_transition_node& node )
{
	node.from().accept	( *this );
	node.to().accept	( *this );
}

void n_ary_tree_interpolator_selector::visit				( n_ary_tree_weight_node& node )
{
	update_interpolator	( node.interpolator() );
}

void n_ary_tree_interpolator_selector::visit				( n_ary_tree_addition_node& node )
{
	propagate			( node );
}

void n_ary_tree_interpolator_selector::visit				( n_ary_tree_subtraction_node& node )
{
	propagate			( node );
}

void n_ary_tree_interpolator_selector::visit				( n_ary_tree_multiplication_node& node )
{
	propagate			( node );
}

template < typename T >
inline void n_ary_tree_interpolator_selector::propagate		( T& node )
{
	n_ary_tree_base_node* const* i			= node.operands( sizeof(T) );
	n_ary_tree_base_node* const* const e	= i + node.operands_count( );
	for ( ; i != e; ++i )
		(*i)->accept	( *this );
}