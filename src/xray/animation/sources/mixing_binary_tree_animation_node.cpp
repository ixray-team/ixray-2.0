////////////////////////////////////////////////////////////////////////////
//	Created		: 19.02.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "mixing_binary_tree_animation_node.h"
#include "mixing_animation_clip.h"
#include "base_interpolator.h"
#include "mixing_binary_tree_visitor.h"

using xray::animation::mixing::binary_tree_animation_node;
using xray::animation::mixing::binary_tree_visitor;
using xray::animation::mixing::binary_tree_base_node;
using xray::animation::base_interpolator;

void binary_tree_animation_node::accept								( binary_tree_visitor& visitor )
{
	visitor.visit	( *this );
}

base_interpolator const* binary_tree_animation_node::interpolator	( ) const
{
	return			&animation()->interpolator();
}