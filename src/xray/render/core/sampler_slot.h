////////////////////////////////////////////////////////////////////////////
//	Created		: 06.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef SAMPLER_SLOT_H_INCLUDED
#define SAMPLER_SLOT_H_INCLUDED

#include "res_sampler_state.h"

namespace xray {
namespace render_dx10 {


class sampler_slot
{
public:
	sampler_slot	( ): slot_id( enum_slot_ind_null), state( NULL)	{ }
	sampler_slot	( name_string const& name, u32 slot_id, res_sampler_state* state): name( name), slot_id( slot_id), state( state)	{ }

// 	name_string const&		name() const		{ return name;}
// 	u32						slot_id() const		{ return slot_id;}

public:
	name_string				name;
	u32						slot_id;
	ref_sampler_state		state;

}; // class sampler_slot



} // namespace render
} // namespace xray

#endif // #ifndef SAMPLER_SLOT_H_INCLUDED