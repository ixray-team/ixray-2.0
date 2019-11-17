////////////////////////////////////////////////////////////////////////////
//	Created		: 26.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "mixing_n_ary_tree.h"
#include "mixing_n_ary_tree_destroyer.h"
#include "mixing_n_ary_tree_animation_node.h"
#include "base_interpolator.h"
#include "mixing_n_ary_tree_weight_calculator.h"

using xray::animation::mixing::n_ary_tree;

n_ary_tree::~n_ary_tree				( )
{
	destroy						( );
}

void n_ary_tree::destroy			( )
{
	if ( !m_root )
		return;

	if ( m_reference_counter->reference_count() > 1 ) {
		m_reference_counter		= 0;
		return;
	}

	ASSERT						( static_cast<pcvoid>(&*m_reference_counter) <= m_interpolators );
	ASSERT						( static_cast<pcvoid>(m_interpolators) <= m_root );

	n_ary_tree_destroyer		tree_destroyer;
	for (n_ary_tree_animation_node* i = m_root; i; i=i->m_next_animation )
		i->accept				( tree_destroyer );

	for (base_interpolator const* const* i = m_interpolators, * const*	const e = i + m_interpolators_count; i != e; ++i )
		(*i)->~base_interpolator( );

	animation_state* i			= m_animation_states;
	animation_state* const e	= m_animation_states + m_animations_count;
	for ( ; i != e; ++i )
		(*i).~animation_state	( );

	if ( m_allocator ) {
		n_ary_tree_intrusive_base* buffer	= &*m_reference_counter;
		m_reference_counter		= 0;
		XRAY_FREE_IMPL			( m_allocator, buffer );
	}
}

void n_ary_tree::tick				( u32 current_time_in_ms )
{
	m_significant_animations_count	= m_animations_count;
	u32 const animations_count	= m_animations_count;
	n_ary_tree_animation_node* i	= &*m_root;
	n_ary_tree_animation_node* j	= 0;
	animation_state* k			= m_animation_states;
	for ( ; i; ) {
		n_ary_tree_weight_calculator	calculator(current_time_in_ms);
		calculator.visit		( *i );
		float const weight		= calculator.weight	( );
		if ( calculator.null_weight_found() ) {
			// remove completed transition
			if ( j ) {
				j->m_next_animation	= i->m_next_animation;
			}
			else {
				m_root			= i->m_next_animation;
				R_ASSERT		( m_root );
			}

			n_ary_tree_destroyer	destroyer;
			i->accept			( destroyer );

			if ( j )
				i				= j->m_next_animation;
			else
				i				= &*m_root;

			--m_animations_count;
			--m_significant_animations_count;
			continue;
		}

		//if ( weight < epsilon ) {
		//	i					= i->m_next_animation;
		//	--m_significant_animations_count;
		//	continue;
		//}

		(*k).animation			= &(*i).animation();
		(*k).bone_mixer_data	= &(*i).bone_mixer_data();

//		R_ASSERT_CMP			( current_time_in_ms, >=, (*i).start_time_in_ms() );
		math::clamp				( current_time_in_ms, (*i).start_time_in_ms(), current_time_in_ms );
		(*k).time				= (*i).time_scale() * float( current_time_in_ms - (*i).start_time_in_ms() )/1000.f;

#pragma message( XRAY_TODO( "(*k).time must be  < (*k).animation->time() no clamp needed " ) )
		math::clamp				( (*k).time , 0.f, (*k).animation->time() );
		ASSERT					( (*k).time <= (*k).animation->time() );

		(*k++).weight			= weight;
		j						= i;
		i						= i->m_next_animation;
	}

	R_ASSERT_CMP				( animations_count, >=, m_animations_count );
	for (animation_state* const e = k + animations_count - m_animations_count; k != e; ++k )
		k->~animation_state		( );
}

void n_ary_tree::dump				( ) const
{
	animation_state const* i			= m_animation_states;
	animation_state const* const e	= i + m_significant_animations_count;
	for ( ; i != e; ++i ) {
		LOG_INFO				( "%s : weight=%f, time=%f", (*i).animation->identifier(), (*i).weight, (*i).time );
	}
	
	LOG_INFO					( "================" );
}