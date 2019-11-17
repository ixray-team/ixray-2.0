////////////////////////////////////////////////////////////////////////////
//	Created		: 10.02.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef OBJECT_LIGHT_H_INCLUDED
#define OBJECT_LIGHT_H_INCLUDED

#include "object.h"

namespace stalker2{

class object_light :public game_object
{
	typedef game_object		super;
public:
					object_light			( game& game );
	virtual			~object_light			( );
	virtual void	load					( xray::configs::lua_config_value const& t );

protected:
	xray::math::color	m_color;
	float				m_brightnes;
	float				m_range;
	bool				m_b_cast_shadow;
}; // class object_light

} // namespace stalker2

#endif // #ifndef OBJECT_LIGHT_H_INCLUDED