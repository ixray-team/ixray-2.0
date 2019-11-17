////////////////////////////////////////////////////////////////////////////
//	Created		: 14.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RES_TEXTURE_LIST_H_INCLUDED
#define RES_TEXTURE_LIST_H_INCLUDED


#include "res_texture.h"

namespace xray {
namespace render_dx10 {

typedef		texture_slot		(texture_slots)	[D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];

class res_texture_list	:
	public res_flagged,
	public render::vector<ref_texture>,
	public res_id
{
	friend class resource_manager;

public:
	res_texture_list	() {}
	res_texture_list	( texture_slots const & slots);
	~res_texture_list	() {}
	void _free			() const; // Not a const function

	bool equal( res_texture_list const & base) const;
	bool equal( texture_slots const & base) const;

// 	void apply();
// 
// 	//	Avoid using this function.
// 	//	If possible use precompiled texture list.
// 	u32 find_texture_stage(const shared_string &tex_name) const;
};
typedef intrusive_ptr<res_texture_list, res_base, threading::single_threading_policy> ref_texture_list;
typedef intrusive_ptr<res_texture_list const, res_base const, threading::single_threading_policy> ref_texture_list_const;

} // namespace render
} // namespace xray

#endif // #ifndef RES_TEXTURE_LIST_H_INCLUDED