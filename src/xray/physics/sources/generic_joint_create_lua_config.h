////////////////////////////////////////////////////////////////////////////
//	Created		: 10.06.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef GENERIC_JOINT_CREATE_LUA_CONFIG_H_INCLUDED
#define GENERIC_JOINT_CREATE_LUA_CONFIG_H_INCLUDED

class body;
class joint;

joint* create_generic_joint(const xray::configs::lua_config_value &cfg, body* b0, body* b1, const float4x4 &transform  );

#endif // #ifndef GENERIC_JOINT_CREATE_LUA_CONFIG_H_INCLUDED