////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef OBJECT_SOLID_VISUAL_H_INCLUDED
#define OBJECT_SOLID_VISUAL_H_INCLUDED

#include "object.h"
#include <xray/render/base/world.h>
#include <xray/render/base/visual.h>

namespace stalker2{

class object_solid_visual :public game_object
{
	typedef game_object		super;
public:
						object_solid_visual		( game& game );
	virtual				~object_solid_visual	( );
	virtual void		load					( xray::configs::lua_config_value const& t );

	virtual void		decrease_quality		( );
	virtual void		make_quality_optimal	( );

protected:
	void				on_visual_ready			( resources::queries_result& data );
	fs::path_string		m_visual_name;
	u32					m_visual_id;
	render::visual_ptr	m_visual;
}; // class object_solid_visual

} // namespace xray

#endif // #ifndef OBJECT_SOLID_VISUAL_H_INCLUDED