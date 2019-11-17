////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "mixing_n_ary_tree_weaver.h"
#include "mixing_binary_tree_animation_node.h"
#include "mixing_binary_tree_weight_node.h"
#include "mixing_binary_tree_addition_node.h"
#include "mixing_binary_tree_subtraction_node.h"
#include "mixing_binary_tree_multiplication_node.h"
#include "base_interpolator.h"
#include "mixing_animation_clip.h"

using xray::animation::mixing::n_ary_tree_weaver;
using xray::animation::mixing::binary_tree_base_node;
using xray::animation::mixing::binary_tree_animation_node;
using xray::animation::mixing::binary_tree_weight_node;
using xray::animation::mixing::binary_tree_addition_node;
using xray::animation::mixing::binary_tree_multiplication_node;
using xray::animation::mixing::binary_tree_subtraction_node;
using xray::animation::base_interpolator;

n_ary_tree_weaver::n_ary_tree_weaver	( ) :
	m_animations_root			( 0 ),
	m_weights_root				( 0 ),
	m_interpolators_root		( 0 ),
	m_interpolators_count		( 0 )
#ifndef MASTER_GOLD
	,m_animations_count			( 0 )
#endif // #ifndef MASTER_GOLD
{
}

template < typename T >
static inline void clean				( T& node )
{
	node.m_next_weight			= 0;
	node.m_same_weight			= 0;
}

void n_ary_tree_weaver::add_interpolator( binary_tree_base_node& node, base_interpolator const& interpolator )
{
	XRAY_UNREFERENCED_PARAMETER	( interpolator );
	++m_interpolators_count;
	node.m_next_unique_interpolator	= m_interpolators_root;
	m_interpolators_root		= &node;
}

void n_ary_tree_weaver::visit			( binary_tree_animation_node& node )
{
	clean						( node );

	node.m_unique_weights_count	= 0;

	node.m_next_animation		= m_animations_root;
	m_animations_root			= &node;

#ifndef MASTER_GOLD
	++m_animations_count;
#endif // #ifndef MASTER_GOLD
}

void n_ary_tree_weaver::visit			( binary_tree_weight_node& node )
{
	node.m_same_weight			= 0;

	node.m_next_weight			= m_weights_root;
	m_weights_root				= &node;

	add_interpolator			( node, node.interpolator_impl() );
}

template < typename T >
inline void n_ary_tree_weaver::propagate( T& node, n_ary_tree_weaver& weaver )
{
	weaver.m_interpolators_root	= m_interpolators_root;
	weaver.m_weights_root		= m_weights_root;

	node.accept					( weaver );

	m_interpolators_root		= weaver.m_interpolators_root;
	m_interpolators_count		+= weaver.m_interpolators_count;
	m_weights_root				= weaver.m_weights_root;
#ifndef MASTER_GOLD
	m_animations_count			+= weaver.m_animations_count;
#endif // #ifndef MASTER_GOLD
}

void n_ary_tree_weaver::join_animations	( n_ary_tree_weaver const& other )
{
	binary_tree_animation_node* previous = 0;
	for ( binary_tree_animation_node_ptr i = other.m_animations_root; i; previous = &*i, i = i->m_next_animation );

	if ( !previous )
		return;

	previous->m_next_animation	= m_animations_root;
	m_animations_root			= other.m_animations_root;
}

template < typename T >
inline void n_ary_tree_weaver::propagate( T& node, n_ary_tree_weaver& left, n_ary_tree_weaver& right )
{
	propagate					( node.left(), left );
	propagate					( node.right(), right );

	join_animations				( left );
	join_animations				( right );
}

void n_ary_tree_weaver::visit			( binary_tree_addition_node& node )
{
	clean						( node );

	n_ary_tree_weaver			left, right;
	propagate					( node, left, right );

	if ( !left.m_animations_root && !right.m_animations_root )
		m_weights_root			= &node;
}

void n_ary_tree_weaver::visit			( binary_tree_subtraction_node& node )
{
	clean						( node );

	n_ary_tree_weaver			left, right;
	propagate					( node, left, right );

	R_ASSERT					( !left.m_animations_root, "it is impossible to subtract from animation" );
	R_ASSERT					( !right.m_animations_root, "it is impossible to subtract an animation" );

	m_weights_root				= &node;
}

static void update_weights				( binary_tree_animation_node* const animations_root, binary_tree_base_node* const weights_root )
{
	xray::animation::mixing::binary_tree_animation_node_ptr current = animations_root;
	do {
		binary_tree_base_node* j= &*current;
		for (binary_tree_base_node* i=current->m_next_weight; i && (j != weights_root); j=i, i=i->m_next_weight );
		ASSERT					( j );
		if ( j != weights_root ) {
			ASSERT				( !j->m_next_weight );
			j->m_next_weight	= weights_root;
		}
		
		current					= current->m_next_animation;
	} while ( current );
}

void n_ary_tree_weaver::visit			( binary_tree_multiplication_node& node )
{
	clean						( node );

	n_ary_tree_weaver			left, right;
	propagate					( node, left, right );

	if ( left.m_animations_root ) {
		ASSERT					( !right.m_animations_root );
		ASSERT					( right.m_weights_root );
		update_weights			( left.m_animations_root, right.m_weights_root );
		m_weights_root			= 0;
		return;
	}

	if ( !right.m_animations_root )
		return;

	ASSERT						( !left.m_animations_root && right.m_animations_root );

	update_weights				( right.m_animations_root, left.m_weights_root );
	m_weights_root				= 0;
}