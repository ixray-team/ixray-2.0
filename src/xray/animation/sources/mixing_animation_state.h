////////////////////////////////////////////////////////////////////////////
//	Created		: 21.04.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef MIXING_ANIMATION_STATE_H_INCLUDED
#define MIXING_ANIMATION_STATE_H_INCLUDED

namespace xray {
namespace animation {

struct bone_mixer_data;

namespace mixing {

class animation_clip;

struct animation_state : private boost::noncopyable {
	inline					animation_state	(
								animation_clip const& animation,
								bone_mixer_data& bone_mixer_data,
								float const time,
								float const weight
							);

	animation_clip const*	animation;
	bone_mixer_data*		bone_mixer_data;
	float					time;
	float					weight;
}; // struct animation_state

} // namespace mixing
} // namespace animation
} // namespace xray

#include "mixing_animation_state_inline.h"

#endif // #ifndef MIXING_ANIMATION_STATE_H_INCLUDED