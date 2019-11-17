////////////////////////////////////////////////////////////////////////////
//	Created		: 04.02.2009
//	Author		: Igor Lobanchikov
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef D3D_DEINES_H_INCLUDED
#define D3D_DEINES_H_INCLUDED

typedef xray::fixed_string<64>	name_string;
const u32 enum_slot_ind_null = 0xFFFFFFFF;

#if USE_DX10

#	include "DX10_defines.h"

#	define DX10_ONLY(expr)			expr

#else

#	include "DX11_defines.h"

#endif	//	USE_DX10

#endif // #ifndef D3D_DEINES_H_INCLUDED
