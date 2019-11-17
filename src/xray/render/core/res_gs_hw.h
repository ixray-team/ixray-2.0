////////////////////////////////////////////////////////////////////////////
//	Created		: 06.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RES_GS_HW_H_INCLUDED
#define RES_GS_HW_H_INCLUDED

#include "res_declaration.h"
#include "res_xs_hw.h"

namespace xray {
namespace render_dx10 {
	
typedef res_xs_hw<gs_data>	res_gs_hw;
typedef xray::intrusive_ptr<res_gs_hw, res_base, threading::single_threading_policy>	ref_gs_hw;

} // namespace render
} // namespace xray

#endif // #ifndef RES_GS_H_INCLUDED