////////////////////////////////////////////////////////////////////////////
//	Created		: 08.02.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef CHECK_ANIMATION_DATA_H_INCLUDED
#define CHECK_ANIMATION_DATA_H_INCLUDED

namespace xray {

namespace configs{
	class lua_config_value;
} // namespace configs

namespace animation {


void	test_data(	configs::lua_config_value	const &animation_data, 
					configs::lua_config_value	const &check_data,
					world &world );

} // namespace animation
} // namespace xray

#endif // #ifndef CHECK_ANIMATION_DATA_H_INCLUDED