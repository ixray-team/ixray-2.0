////////////////////////////////////////////////////////////////////////////
//	Created		: 13.01.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_TERRAIN_H_INCLUDED
#define TOOL_TERRAIN_H_INCLUDED

#include "tool_base.h"

namespace xray {
namespace editor {

ref class terrain_modifier_bump;
ref class terrain_modifier_raise_lower;
ref class terrain_modifier_flatten;
ref class terrain_modifier_smooth;	
ref class terrain_modifier_detail_painter;
ref class terrain_modifier_diffuse_painter;
ref class terrain_tool_tab;
ref class terrain_core;


public ref class tool_terrain :public tool_base
{
	typedef tool_base	super;
public:
						tool_terrain			(level_editor^ le);
	virtual				~tool_terrain			();

	virtual void		tick					() override;

	virtual object_base^ create_object			(System::String^ name)						override;
	virtual object_base^ load_object			(xray::configs::lua_config_value const& t)	override;
	virtual object_base^ create_raw_object		(System::String^ id)						override;
	virtual void		destroy_object			(object_base^ o)							override;

	void				load_library			();
	virtual tool_tab^	ui						() override;

	virtual void		load_settings			(RegistryKey^ key)	override;
	virtual void		save_settings			(RegistryKey^ key)	override;
	
	terrain_core^		get_terrain_core		();

private:
	terrain_tool_tab^							m_tool_tab;
	terrain_core^								m_terrain_core;
public: // tmp
	terrain_modifier_bump^				m_terrain_modifier_bump;
	terrain_modifier_raise_lower^		m_terrain_modifier_raise_lower;
	terrain_modifier_flatten^			m_terrain_modifier_flatten;
	terrain_modifier_smooth^			m_terrain_modifier_smooth;
	terrain_modifier_detail_painter^	m_terrain_modifier_detail_painter;
	terrain_modifier_diffuse_painter^	m_terrain_modifier_diffuse_painter;
}; // class tool_terrain

} // namespace editor
} // namespace xray

#endif // #ifndef TOOL_TERRAIN_H_INCLUDED