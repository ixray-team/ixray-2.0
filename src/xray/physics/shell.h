////////////////////////////////////////////////////////////////////////////
//	Created 	: 17.02.2008
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_PHYSICS_SHELL_H_INCLUDED
#define XRAY_PHYSICS_SHELL_H_INCLUDED

namespace xray {
namespace physics {

struct XRAY_NOVTABLE shell {
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( shell )
}; // struct shell

} // namespace physics
} // namespace xray

#endif // #ifndef XRAY_PHYSICS_SHELL_H_INCLUDED