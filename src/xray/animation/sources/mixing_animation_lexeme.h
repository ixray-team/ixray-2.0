////////////////////////////////////////////////////////////////////////////
//	Created		: 19.02.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef ANIMATION_LEXEME_H_INCLUDED
#define ANIMATION_LEXEME_H_INCLUDED

#include "mixing_binary_tree_animation_node.h"
#include "mixing_base_lexeme.h"

namespace xray {
namespace animation {
namespace mixing {

class animation_lexeme :
	public binary_tree_animation_node,
	public base_lexeme
{
public:
	inline					animation_lexeme	( mutable_buffer& buffer, animation_clip* const animation, float const time_scale = 1.f );
	inline					animation_lexeme	( animation_lexeme const& other, bool );
	inline	animation_lexeme* cloned_in_buffer	( );
}; // class animation_lexeme

} // namespace mixing
} // namespace animation
} // namespace xray

#include "mixing_animation_lexeme_inline.h"

#endif // #ifndef ANIMATION_LEXEME_H_INCLUDED