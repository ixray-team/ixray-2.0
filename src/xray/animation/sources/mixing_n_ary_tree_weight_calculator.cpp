////////////////////////////////////////////////////////////////////////////
//	Unit		: n_ary_tree_weight_calculator.cpp
//	Created		: 04.04.2010
//	Author		: Dmitriy Iassenev
//	Copyright(C) Challenge Solutions(tm) - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "mixing_n_ary_tree_weight_calculator.h"
#include "mixing_n_ary_tree_animation_node.h"
#include "mixing_n_ary_tree_transition_node.h"
#include "mixing_n_ary_tree_weight_node.h"
#include "mixing_n_ary_tree_addition_node.h"
#include "mixing_n_ary_tree_subtraction_node.h"
#include "mixing_n_ary_tree_multiplication_node.h"
#include "mixing_n_ary_tree_destroyer.h"

using xray::animation::mixing::n_ary_tree_weight_calculator;
using xray::animation::mixing::n_ary_tree_animation_node;
using xray::animation::mixing::n_ary_tree_transition_node;
using xray::animation::mixing::n_ary_tree_weight_node;
using xray::animation::mixing::n_ary_tree_addition_node;
using xray::animation::mixing::n_ary_tree_subtraction_node;
using xray::animation::mixing::n_ary_tree_multiplication_node;

void n_ary_tree_weight_calculator::visit		( n_ary_tree_animation_node& node )
{
	float weight		= 1.f;

	u32 const operands_count				= node.operands_count( );
	n_ary_tree_base_node** i			= node.operands( sizeof(n_ary_tree_animation_node) );
	n_ary_tree_base_node** const e	= i + operands_count;
	for ( ; i != e; ++i ) {
		(*i)->accept	( *this );
		weight			*= m_weight;
		if ( weight == 0.f )
			break;

		if ( !m_result )
			continue;

		*i				= m_result;
	}

	m_weight			= weight;
	m_result			= 0;
}

void n_ary_tree_weight_calculator::visit		( n_ary_tree_transition_node& node )
{
	float const transition_time		= float(m_current_time_in_ms - node.start_time( ))/1000.f;
	float const interpolated_value	= (transition_time >= node.interpolator().transition_time()) ? 1.f : node.interpolator().interpolated_value( transition_time );
	R_ASSERT_CMP		( interpolated_value, >=, 0.f );
	R_ASSERT_CMP		( interpolated_value, <=, 1.f );
	if ( interpolated_value == 1.f ) {
		node.to().accept( *this );
		if ( m_weight == 0.f ) {
			m_null_weight_found	= true;
			return;
		}

		m_result		= &node.to();
		n_ary_tree_destroyer	destroyer;
		node.from().accept	( destroyer );
		node.~n_ary_tree_transition_node	( );
		return;
	}

	node.from().accept	( *this );
	float const weight_from	= m_weight;

	node.to().accept	( *this );
	float const weight_to	= m_weight;

	m_weight			= weight_from*(1.f - interpolated_value) + weight_to*interpolated_value;
}

void n_ary_tree_weight_calculator::visit		( n_ary_tree_weight_node& node )
{
	m_weight			= node.weight( );
	m_result			= 0;
}

void n_ary_tree_weight_calculator::visit		( n_ary_tree_addition_node& node )
{
	float weight		= 0.f;

	u32 const operands_count				= node.operands_count( );
	n_ary_tree_base_node** i			= node.operands( sizeof(n_ary_tree_addition_node) );
	n_ary_tree_base_node** const e	= i + operands_count;
	for ( ; i != e; ++i ) {
		(*i)->accept	( *this );
		weight			+= m_weight;

		if ( !m_result )
			continue;

		*i				= m_result;
	}

	m_weight			= weight;
	m_result			= 0;
}

void n_ary_tree_weight_calculator::visit		( n_ary_tree_subtraction_node& node )
{
	bool first_operand	= true;
	float weight		= 0.f;

	u32 const operands_count				= node.operands_count( );
	n_ary_tree_base_node** i			= node.operands( sizeof(n_ary_tree_subtraction_node) );
	n_ary_tree_base_node** const e	= i + operands_count;
	for ( ; i != e; ++i ) {
		(*i)->accept	( *this );
		if ( first_operand ) {
			first_operand	= false;
			weight		= m_weight;
		}
		else
			weight		-= m_weight;

		if ( !m_result )
			continue;

		*i				= m_result;
	}

	m_weight			= weight;
	m_result			= 0;
}

void n_ary_tree_weight_calculator::visit		( n_ary_tree_multiplication_node& node )
{
	float weight		= 1.f;

	u32 const operands_count				= node.operands_count( );
	n_ary_tree_base_node** i			= node.operands( sizeof(n_ary_tree_multiplication_node) );
	n_ary_tree_base_node** const e	= i + operands_count;
	for ( ; i != e; ++i ) {
		(*i)->accept	( *this );
		weight			*= m_weight;
		if ( weight == 0.f )
			break;

		if ( !m_result )
			continue;

		*i				= m_result;
	}

	m_weight			= weight;
	m_result			= 0;
}