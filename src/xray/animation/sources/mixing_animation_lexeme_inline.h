////////////////////////////////////////////////////////////////////////////
//	Created		: 19.02.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef ANIMATION_LEXEME_INLINE_H_INCLUDED
#define ANIMATION_LEXEME_INLINE_H_INCLUDED

namespace xray {
namespace animation {
namespace mixing {

inline animation_lexeme::animation_lexeme					(
		mutable_buffer& buffer,
		animation_clip* const animation,
		float const time_scale
	) :
	binary_tree_animation_node	( animation, time_scale ),
	base_lexeme			( buffer )
{
	ASSERT_CMP			( time_scale, >, 0.f );
}

inline animation_lexeme::animation_lexeme					( animation_lexeme const& other, bool ) :
	binary_tree_animation_node	( other ),
	base_lexeme			( other, true )
{
}

inline animation_lexeme* animation_lexeme::cloned_in_buffer	( )
{
	return				base_lexeme::cloned_in_buffer< animation_lexeme >( );
}

} // namespace mixing
} // namespace animation
} // namespace xray

#endif // #ifndef ANIMATION_LEXEME_INLINE_H_INCLUDED