////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "mixing_n_ary_tree_destroyer.h"
#include "mixing_n_ary_tree_animation_node.h"
#include "mixing_n_ary_tree_weight_node.h"
#include "mixing_n_ary_tree_addition_node.h"
#include "mixing_n_ary_tree_subtraction_node.h"
#include "mixing_n_ary_tree_multiplication_node.h"
#include "mixing_n_ary_tree_transition_node.h"

using xray::animation::mixing::n_ary_tree_destroyer;
using xray::animation::mixing::n_ary_tree_animation_node;
using xray::animation::mixing::n_ary_tree_weight_node;
using xray::animation::mixing::n_ary_tree_addition_node;
using xray::animation::mixing::n_ary_tree_subtraction_node;
using xray::animation::mixing::n_ary_tree_multiplication_node;
using xray::animation::mixing::n_ary_tree_transition_node;
using xray::animation::mixing::n_ary_tree_n_ary_operation_node;

template < typename T >
static inline void destroy					( T& node )
{
	node.~T			( );
}

void n_ary_tree_destroyer::visit			( n_ary_tree_animation_node& node )
{
	propagate		( node );
}

void n_ary_tree_destroyer::visit			( n_ary_tree_weight_node& node )
{
	destroy			( node );
}

void n_ary_tree_destroyer::visit			( n_ary_tree_transition_node& node )
{
	node.from().accept	( *this );
	node.to().accept	( *this );
	destroy			( node );
}

void n_ary_tree_destroyer::visit			( n_ary_tree_addition_node& node )
{
	propagate		( node );
}

void n_ary_tree_destroyer::visit			( n_ary_tree_subtraction_node& node )
{
	propagate		( node );
}

void n_ary_tree_destroyer::visit			( n_ary_tree_multiplication_node& node )
{
	propagate		( node );
}

template < typename T >
inline void n_ary_tree_destroyer::propagate	( T& node )
{
	n_ary_tree_base_node** i			= node.operands( sizeof(T) );
	n_ary_tree_base_node** const e	= i + node.operands_count( );
	for ( ; i != e; ++i )
		(*i)->accept( *this );

	destroy			( node );
}