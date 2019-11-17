////////////////////////////////////////////////////////////////////////////
//	Created		: 04.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "mixing_n_ary_tree_comparer.h"
#include "mixing_n_ary_tree_animation_node.h"
#include "mixing_n_ary_tree_weight_node.h"
#include "mixing_n_ary_tree_addition_node.h"
#include "mixing_n_ary_tree_subtraction_node.h"
#include "mixing_n_ary_tree_multiplication_node.h"
#include "mixing_n_ary_tree_transition_node.h"
#include "mixing_n_ary_tree.h"
#include "mixing_n_ary_tree_transition_constructor.h"
#include "mixing_animation_clip.h"
#include "mixing_n_ary_tree_node_comparer.h"
#include "mixing_n_ary_tree_size_calculator.h"
#include "mixing_n_ary_tree_interpolator_selector.h"
#include "interpolator_size_calculator.h"

using xray::animation::mixing::n_ary_tree_comparer;
using xray::animation::mixing::n_ary_tree_base_node;
using xray::animation::mixing::n_ary_tree_animation_node;
using xray::animation::mixing::n_ary_tree_weight_node;
using xray::animation::mixing::n_ary_tree_addition_node;
using xray::animation::mixing::n_ary_tree_subtraction_node;
using xray::animation::mixing::n_ary_tree_multiplication_node;
using xray::animation::mixing::n_ary_tree_transition_node;
using xray::animation::mixing::n_ary_tree_n_ary_operation_node;
using xray::animation::mixing::n_ary_tree;
using xray::animation::base_interpolator;
using xray::animation::mixing::n_ary_tree_interpolator_selector;
using xray::animation::mixing::binary_tree_base_node;
using xray::animation::bone_mixer;

static inline void on_new_interpolator					( base_interpolator const*& result, n_ary_tree_base_node& node )
{
	n_ary_tree_interpolator_selector	selector( result );
	node.accept				( selector );
	result					= selector.selected( );
}

template < typename T >
static inline void on_new_interpolator					( base_interpolator const*& result, T& node )
{
	n_ary_tree_interpolator_selector	selector( result );
	selector.visit			( node );
	result					= selector.selected( );
}

void n_ary_tree_comparer::process_interpolators		(
		n_ary_tree const& from,
		n_ary_tree const& to
	)
{
	u32 const from_interpolators_count				= from.interpolators_count();
	u32 const to_interpolators_count				= to.interpolators_count();
	base_interpolator const* const* const from_interpolators_begin = from.interpolators( );
	base_interpolator const* const* const to_interpolators_begin = to.interpolators( );

	u32 const total_interpolators_count				= from_interpolators_count + to_interpolators_count;
	base_interpolator const** const interpolators_begin	= static_cast<base_interpolator const**>( ALLOCA( total_interpolators_count*sizeof(base_interpolator const*) ) );
	base_interpolator const** const interpolators_end		=
		std::unique(
			interpolators_begin,
			std::merge(
				from_interpolators_begin,
				from_interpolators_begin + from_interpolators_count,
				to_interpolators_begin,
				to_interpolators_begin + to_interpolators_count,
				interpolators_begin,
				merge_interpolators_predicate()
			),
			unique_interpolators_predicate()
		);
	u32 const unique_interpolators_count	= u32(interpolators_end - interpolators_begin);

	interpolator_size_calculator	size_calculator;
	for ( base_interpolator const** i = interpolators_begin; i != interpolators_end; ++i )
		(*i)->accept		( size_calculator );

	m_needed_buffer_size	+= unique_interpolators_count*sizeof(base_interpolator const*) + size_calculator.calculated_size();
}

n_ary_tree_comparer::n_ary_tree_comparer				(
		n_ary_tree const& from,
		n_ary_tree const& to
	) :
	m_from						( from ),
	m_to						( to ),
	m_additive_interpolator		( 0 ),
	m_non_additive_interpolator	( 0 ),
	m_current_interpolator		( 0 ),
	m_animations_count			( 0 ),
	m_needed_buffer_size		( 0 ),
	m_equal						( true )
{
	m_needed_buffer_size		+= sizeof(n_ary_tree_intrusive_base);

	process_interpolators		( from, to );

	n_ary_tree_animation_node* i	= &*m_from.root();
	n_ary_tree_animation_node* j	= &*m_to.root();
	for ( ; i && j; ++m_animations_count ) {
		if ( *i < *j ) {
			increase_buffer_size( *i );
			m_needed_buffer_size += sizeof(n_ary_tree_base_node*);
			m_needed_buffer_size += 2*sizeof(n_ary_tree_weight_node) + sizeof(n_ary_tree_transition_node);
			if ( !(*i).is_transiting_to_zero() )
				m_equal			= false;
			i					= i->m_next_animation;
			continue;
		}

		if ( *j < *i ) {
			on_new_interpolator	( j->animation().type() == animation_clip::additive ? m_additive_interpolator : m_non_additive_interpolator, *j );
			increase_buffer_size( *j );
			m_needed_buffer_size += sizeof(n_ary_tree_base_node*);
			m_needed_buffer_size += 2*sizeof(n_ary_tree_weight_node) + sizeof(n_ary_tree_transition_node);
			j					= j->m_next_animation;
			m_equal				= false;
			continue;
		}

		m_current_interpolator	= &(j->animation().type() == animation_clip::additive ? m_additive_interpolator : m_non_additive_interpolator);
		i->accept				( *this, *j );
		m_current_interpolator	= 0;
		i						= i->m_next_animation;
		j						= j->m_next_animation;
	}

	for ( ; i; i = i->m_next_animation, ++m_animations_count ) {
		increase_buffer_size	( *i );
		m_needed_buffer_size	+= sizeof(n_ary_tree_base_node*);
		m_needed_buffer_size	+= 2*sizeof(n_ary_tree_weight_node) + sizeof(n_ary_tree_transition_node);
		if ( !(*i).is_transiting_to_zero() )
			m_equal				= false;
	}

	for ( ; j; j = j->m_next_animation, ++m_animations_count ) {
		on_new_interpolator		( j->animation().type() == animation_clip::additive ? m_additive_interpolator : m_non_additive_interpolator, *j );
		increase_buffer_size	( *j );
		m_needed_buffer_size	+= sizeof(n_ary_tree_base_node*);
		m_needed_buffer_size	+= 2*sizeof(n_ary_tree_weight_node) + sizeof(n_ary_tree_transition_node);
		m_equal					= false;
	}

	if ( m_additive_interpolator && (*m_additive_interpolator == *m_non_additive_interpolator) )
		m_non_additive_interpolator	= m_additive_interpolator;

	m_needed_buffer_size		+= m_animations_count*sizeof(animation_state);
}

void n_ary_tree_comparer::increase_buffer_size			( n_ary_tree_base_node& node )
{
	n_ary_tree_size_calculator	calculator;
	node.accept					( calculator );
	m_needed_buffer_size		+= calculator.calculated_size( );
}

template < typename T >
inline void n_ary_tree_comparer::increase_buffer_size	( T& node )
{
	n_ary_tree_size_calculator	calculator;
	calculator.visit			( node );
	m_needed_buffer_size		+= calculator.calculated_size( );
}

bool n_ary_tree_comparer::equal							( ) const
{
	return				m_equal;
}

u32 n_ary_tree_comparer::needed_buffer_size				( ) const
{
	return				m_needed_buffer_size;
}

n_ary_tree n_ary_tree_comparer::computed_tree			( xray::mutable_buffer& buffer, xray::memory::base_allocator* allocator, bone_mixer& bone_mixer, u32 const current_time_in_ms )
{
	ASSERT				( !equal() );
	n_ary_tree_transition_constructor transition_constructor( buffer, bone_mixer, m_from, m_to, m_additive_interpolator, m_non_additive_interpolator, m_animations_count, current_time_in_ms );
	return				transition_constructor.computed_tree( allocator );
}

////////////////////////////////////////////////////////////////////////////
// n_ary_tree_animation_node
////////////////////////////////////////////////////////////////////////////
void n_ary_tree_comparer::dispatch	( n_ary_tree_animation_node& left,		n_ary_tree_animation_node& right )
{
	ASSERT							( left == right );
	on_new_interpolator				( *m_current_interpolator, left );
	propagate						( left, right );
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_animation_node& left,		n_ary_tree_transition_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_animation_node& left,		n_ary_tree_weight_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_animation_node& left,		n_ary_tree_addition_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_animation_node& left,		n_ary_tree_subtraction_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_animation_node& left,		n_ary_tree_multiplication_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

////////////////////////////////////////////////////////////////////////////
// n_ary_tree_transition_node
////////////////////////////////////////////////////////////////////////////
void n_ary_tree_comparer::dispatch	( n_ary_tree_transition_node& left,		n_ary_tree_animation_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_transition_node& left,		n_ary_tree_transition_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_transition_node& left,		n_ary_tree_weight_node& right )
{
#pragma message( XRAY_TODO( "we may omit one virtual function call here since we know exact right node type" ) )
	increase_buffer_size			( left.from() );
	left.to().accept				( *this, right );
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_transition_node& left,		n_ary_tree_addition_node& right )
{
	increase_buffer_size			( left.from() );
	left.to().accept				( *this, right );
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_transition_node& left,		n_ary_tree_subtraction_node& right )
{
	increase_buffer_size			( left.from() );
	left.to().accept				( *this, right );
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_transition_node& left,		n_ary_tree_multiplication_node& right )
{
	increase_buffer_size			( left.from() );
	left.to().accept				( *this, right );
}

////////////////////////////////////////////////////////////////////////////
// n_ary_tree_weight_node
////////////////////////////////////////////////////////////////////////////
void n_ary_tree_comparer::dispatch	( n_ary_tree_weight_node& left,			n_ary_tree_animation_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_weight_node& left,			n_ary_tree_transition_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_weight_node& left,			n_ary_tree_weight_node& right )
{
	increase_buffer_size			( left );
	if ( left == right )
		return;

	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_weight_node& left,			n_ary_tree_addition_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_weight_node& left,			n_ary_tree_subtraction_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_weight_node& left,			n_ary_tree_multiplication_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

////////////////////////////////////////////////////////////////////////////
// n_ary_tree_addition_node
////////////////////////////////////////////////////////////////////////////
void n_ary_tree_comparer::dispatch	( n_ary_tree_addition_node& left,		n_ary_tree_animation_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_addition_node& left,		n_ary_tree_transition_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_addition_node& left,		n_ary_tree_weight_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_addition_node& left,		n_ary_tree_addition_node& right )
{
	propagate						( left, right );
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_addition_node& left,		n_ary_tree_subtraction_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_addition_node& left,		n_ary_tree_multiplication_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

////////////////////////////////////////////////////////////////////////////
// n_ary_tree_subtraction_node
////////////////////////////////////////////////////////////////////////////
void n_ary_tree_comparer::dispatch	( n_ary_tree_subtraction_node& left,		n_ary_tree_animation_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_subtraction_node& left,		n_ary_tree_transition_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_subtraction_node& left,		n_ary_tree_weight_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_subtraction_node& left,		n_ary_tree_addition_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_subtraction_node& left,		n_ary_tree_subtraction_node& right )
{
	propagate						( left, right );
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_subtraction_node& left,		n_ary_tree_multiplication_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

////////////////////////////////////////////////////////////////////////////
// n_ary_tree_multiplication_node
////////////////////////////////////////////////////////////////////////////
void n_ary_tree_comparer::dispatch	( n_ary_tree_multiplication_node& left,	n_ary_tree_animation_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_multiplication_node& left,	n_ary_tree_transition_node& right )
{
	XRAY_UNREFERENCED_PARAMETERS	(&left, &right );
	UNREACHABLE_CODE();
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_multiplication_node& left,	n_ary_tree_weight_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_multiplication_node& left,	n_ary_tree_addition_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_multiplication_node& left,	n_ary_tree_subtraction_node& right )
{
	increase_buffer_size			( left );
	on_new_interpolator				( *m_current_interpolator, right );
	increase_buffer_size			( right );
	m_needed_buffer_size			+= sizeof( n_ary_tree_transition_node );
	m_equal							= false;
}

void n_ary_tree_comparer::dispatch	( n_ary_tree_multiplication_node& left,	n_ary_tree_multiplication_node& right )
{
	propagate						( left, right );
}

void n_ary_tree_comparer::add_transition_node	(
		u32 const left_unique_multipliers,
		u32 const right_unique_multipliers
	)
{
	m_needed_buffer_size				+= sizeof( n_ary_tree_transition_node ) + sizeof(n_ary_tree_base_node*);

	if ( left_unique_multipliers > 1 ) {
		m_needed_buffer_size			+= sizeof( n_ary_tree_multiplication_node );
		m_needed_buffer_size			+= left_unique_multipliers*sizeof(n_ary_tree_base_node*);
	}
	else
		if ( !left_unique_multipliers )
			m_needed_buffer_size		+= sizeof( n_ary_tree_weight_node );

	if ( right_unique_multipliers > 1 ) {
		m_needed_buffer_size			+= sizeof( n_ary_tree_multiplication_node );
		m_needed_buffer_size			+= right_unique_multipliers*sizeof(n_ary_tree_base_node*);
	}
	else
		if ( !right_unique_multipliers )
			m_needed_buffer_size		+= sizeof( n_ary_tree_weight_node );
}

template <typename T>
inline void n_ary_tree_comparer::propagate		( T& left, T& right )
{
	m_needed_buffer_size			+= sizeof( T );

	n_ary_tree_node_comparer	comparer;

	u32 left_unique_multipliers			= 0;
	u32 right_unique_multipliers		= 0;
	n_ary_tree_base_node* const* i			= left.operands( sizeof(T) );
	n_ary_tree_base_node* const* const i_e	= i + left.operands_count( );
	n_ary_tree_base_node* const* j			= right.operands( sizeof(T) );
	n_ary_tree_base_node* const* const j_e	= j + right.operands_count( );
	for ( ; (i != i_e) && (j != j_e); ) {
		switch ( comparer.compare(**i, **j) ) {
			case n_ary_tree_node_comparer::less : {
				increase_buffer_size	( **i );
				++left_unique_multipliers;
				++i;
				m_equal					= false;
				break;
			}
			case n_ary_tree_node_comparer::more : {
				on_new_interpolator		( *m_current_interpolator, **j );
				increase_buffer_size	( **j );
				++right_unique_multipliers;
				++j;
				m_equal					= false;
				break;
			}
			case n_ary_tree_node_comparer::equal : {
				increase_buffer_size	( **i );
				m_needed_buffer_size	+= sizeof(n_ary_tree_base_node*);
				++i;
				++j;
				break;
			}
		}
	}

	for ( ; i != i_e; ++i ) {
		increase_buffer_size			( **i );
		++left_unique_multipliers;
		m_equal							= false;
	}

	for ( ; j != j_e; ++j ) {
		on_new_interpolator				( *m_current_interpolator, **j );
		increase_buffer_size			( **j );
		++right_unique_multipliers;
		m_equal							= false;
	}

	if ( !(left_unique_multipliers + right_unique_multipliers) )
		return;

	add_transition_node					( left_unique_multipliers, right_unique_multipliers );
}