////////////////////////////////////////////////////////////////////////////
//	Created		: 23.03.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "collision_object_dynamic.h"
#include "object_base.h"

using xray::editor::object_base;
using xray::editor::collision_object_dynamic;

object_base^ collision_object_dynamic::get_owner( ) const 
{ 
	return m_owner; 
}
