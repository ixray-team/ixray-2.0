////////////////////////////////////////////////////////////////////////////
//	Created		: 24.04.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef OBJECT_CAMERA_H_INCLUDED
#define OBJECT_CAMERA_H_INCLUDED

#include "object_base.h"

namespace xray{
namespace editor{
class tool_utils;

class object_camera :public object_base
{
	typedef object_base	super;
public:
									object_camera	(tool_utils& t);
	virtual							~object_camera	();
	virtual void					load			(xray::configs::lua_config_value const& t);
	virtual void					save			(xray::configs::lua_config_value const& t);
	virtual void					fill_ui			(tree_node^ node, u32& count);
	virtual void					render			(render::base::world& world, float4x4 const& tranform);
	virtual void					initialize_property_holder	();
	
private:
			tool_utils&				m_tool_utils;
			float					m_fov;
			float					m_aspect_ratio;

}; // class object_camera

}// namespace editor
}// namespace xray

#endif // #ifndef OBJECT_CAMERA_H_INCLUDED