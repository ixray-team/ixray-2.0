////////////////////////////////////////////////////////////////////////////
//	Created		: 03.02.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef TERRAIN_MODIFIER_BUMP_H_INCLUDED
#define TERRAIN_MODIFIER_BUMP_H_INCLUDED

#include "terrain_modifier_control.h"

namespace xray {
namespace editor {

public ref class terrain_modifier_bump : public terrain_modifier_height
{
	typedef terrain_modifier_height	super;
public:
								terrain_modifier_bump	( level_editor^ le, tool_terrain^ tool);
	virtual						~terrain_modifier_bump	( );
	virtual		void			start_input				( ) override;
	virtual		void			execute_input			( ) override;
	virtual		void			do_work					( ) override;
	virtual		void			load_settings			(RegistryKey^ key)	override;
	virtual		void			save_settings			(RegistryKey^ key)	override;
public:
	property bool				grab_mode;
protected:

	xray::timing::timer*		m_timer;
}; // class terrain_modifier_bump

} // namespace editor
} // namespace xray

#endif // #ifndef TERRAIN_MODIFIER_BUMP_H_INCLUDED