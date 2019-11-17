////////////////////////////////////////////////////////////////////////////
//	Created		: 27.04.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef OBJECT_TRACK_H_INCLUDED
#define OBJECT_TRACK_H_INCLUDED

#include "object_base.h"

namespace xray{
namespace editor{

class tool_utils;

class object_track :public object_base 
{
	typedef object_base		super;
public:
									object_track	(tool_utils& t);
	virtual							~object_track	();
	virtual void					load			(xray::configs::lua_config_value const& t);
	virtual void					save			(xray::configs::lua_config_value const& t);
	virtual void					fill_ui			(tree_node^ node, u32& count);
	virtual void					render			(render::base::world& world, float4x4 const& tranform);
	virtual void					initialize_property_holder	();
	
private:
			tool_utils&				m_tool_utils;
}; // class object_track

} // namespace editor
} // namespace xray

#endif // #ifndef OBJECT_TRACK_H_INCLUDED