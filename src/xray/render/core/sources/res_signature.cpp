////////////////////////////////////////////////////////////////////////////
//	Created		: 28.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/core/res_signature.h>
#include <xray/render/core/resource_manager.h>

namespace xray {
namespace render_dx10 {

void res_signature::_free() const
{
	resource_manager::ref().release( const_cast<res_signature*>(this) );
}

} // namespace render
} // namespace xray
