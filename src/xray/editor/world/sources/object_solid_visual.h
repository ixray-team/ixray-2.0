////////////////////////////////////////////////////////////////////////////
//	Created		: 13.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef OBJECT_SOLID_VISUAL_H_INCLUDED
#define OBJECT_SOLID_VISUAL_H_INCLUDED

#include "object_base.h"
#include <xray/render/base/world.h>
#include <xray/render/base/visual.h>

namespace xray {
namespace editor {

ref class tool_solid_visual;

ref class object_solid_visual :public object_base
{
	typedef object_base		super;
public:
					object_solid_visual		(tool_solid_visual^ t);
	virtual			~object_solid_visual	();
	virtual void	load					(xray::configs::lua_config_value const& t) override;
	virtual	void	set_transform			(float4x4 const& transform) override;
	virtual	void	set_selected			(bool selected) override;
	virtual	void	set_visible				(bool bvisible) override;
	virtual enum_terrain_interaction get_terrain_interaction()	 override			{return enum_terrain_interaction::tr_vertical_snap;}

			void	set_visual_name_		(System::String^ name);

protected:
			void	initialize_collision	();

protected:
	void			on_visual_ready			(resources::unmanaged_resource_ptr data);
	void			on_resource_loaded		( xray::resources::queries_result& data );

	System::String^					m_visual_name;

public:
	render::visual_ptr*		m_visual;
	tool_solid_visual^				m_tool_solid_visual;
public:
	[DisplayNameAttribute("visual"), DescriptionAttribute("object mesh name"), CategoryAttribute("general"), 
	ValueAttribute(e_def_val, ""), ReadOnlyAttribute(false)]
	property System::String^	visual_name{
		System::String^			get			()						{return m_visual_name;}
		void					set			(System::String^ n)		{set_visual_name_(n);on_property_changed("visual");}
	}
}; // class object_solid_visual

} // namespace editor
} // namespace xray

#endif // #ifndef OBJECT_SOLID_VISUAL_H_INCLUDED