////////////////////////////////////////////////////////////////////////////
//	Created		: 14.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/core/res_texture_list.h>
#include <xray/render/core/effect_manager.h>
#include <xray/render/core/resource_manager.h>

namespace xray {
namespace render_dx10 {

res_texture_list::res_texture_list	( texture_slots const & slots) 
{
	for (u32 i = 0; i < D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++i)
	{
		if( slots[i].slot_id != enum_slot_ind_null)
		{
			resize(i+1, ref_texture(NULL));
			(*this)[i] = slots[i].texture;
		}
	}
}

void res_texture_list::_free	() const
{
	resource_manager::ref().release( const_cast<res_texture_list*>(this));
}

// u32 res_texture_list::find_texture_stage(const shared_string &tex_name) const
// {
// 	u32	texture_stage = 0;
// 
// 	res_texture_list::const_iterator it  = this->begin();
// 	res_texture_list::const_iterator end = this->end();
// 
// 	for ( int i = 0; it != end; ++it, ++i)
// 	{
// 		const ref_texture loader = *it;
// 
// 		//	Shadowmap texture always uses 0 texture unit
// 		if (loader->m_name == tex_name)
// 		{
// 			//	Assign correct texture
// 			texture_stage	= i;
// 			break;
// 		}
// 	}
// 
// 	ASSERT(it != end);
// 
// 	return texture_stage;
// }

bool res_texture_list::equal(const res_texture_list& base) const
{
	if( size() != base.size())
		return false;

	for (u32 cmp = 0; cmp < size(); ++cmp)
	{
		if ((*this)[cmp] != base[cmp])	return false;
	}

	return true;
}

bool res_texture_list::equal( texture_slots const & base) const
{
	for (u32 i = 0; i < D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++i)
	{
		if( i < size())
		{
			if ((*this)[i] != base[i].texture)	
				return false;
		}
		else if( base[i].slot_id != enum_slot_ind_null)
			return false;
	}

	return true;
}

} // namespace render
} // namespace xray

