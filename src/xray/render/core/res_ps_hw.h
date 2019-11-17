////////////////////////////////////////////////////////////////////////////
//	Created		: 06.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RES_PS_HW_H_INCLUDED
#define RES_PS_HW_H_INCLUDED

#include "res_declaration.h"
#include "res_xs_hw.h"

namespace xray {
namespace render_dx10 {
	

typedef res_xs_hw<ps_data>	res_ps_hw;
typedef xray::intrusive_ptr<res_ps_hw, res_base, threading::single_threading_policy>	ref_ps_hw;

} // namespace render
} // namespace xray

#endif // #ifndef RES_PS_H_INCLUDED