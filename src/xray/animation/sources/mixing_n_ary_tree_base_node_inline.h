////////////////////////////////////////////////////////////////////////////
//	Created		: 20.04.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef MIXING_N_ARY_TREE_BASE_NODE_INLINE_H_INCLUDED
#define MIXING_N_ARY_TREE_BASE_NODE_INLINE_H_INCLUDED

namespace xray {
namespace animation {
namespace mixing {

inline n_ary_tree_base_node::n_ary_tree_base_node	( )
{
#ifdef XRAY_ANIMATION_CHECK_CONSTRUCTORS
	++g_n_ary_tree_node_constructors;
#endif // #ifdef XRAY_ANIMATION_CHECK_CONSTRUCTORS
}

#ifdef XRAY_ANIMATION_CHECK_CONSTRUCTORS
inline n_ary_tree_base_node::n_ary_tree_base_node	( n_ary_tree_base_node const& )
{
	++g_n_ary_tree_node_constructors;
}
#endif // #ifdef XRAY_ANIMATION_CHECK_CONSTRUCTORS

inline n_ary_tree_base_node::~n_ary_tree_base_node	( )
{
#ifdef XRAY_ANIMATION_CHECK_CONSTRUCTORS
	ASSERT	( g_n_ary_tree_node_destructors < g_n_ary_tree_node_constructors );
	++g_n_ary_tree_node_destructors;
#endif // #ifdef XRAY_ANIMATION_CHECK_CONSTRUCTORS
}

} // namespace mixing
} // namespace animation
} // namespace xray

#endif // #ifndef MIXING_N_ARY_TREE_BASE_NODE_INLINE_H_INCLUDED