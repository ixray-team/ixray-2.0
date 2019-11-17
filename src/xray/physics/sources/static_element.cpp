////////////////////////////////////////////////////////////////////////////
//	Created 	: 16.02.2008
//	Author		: Konstantin Slipchenko
//	Description : static element
////////////////////////////////////////////////////////////////////////////
#include "pch.h"

#include "static_element.h"

bool	static_element::collide( island& , const xray::collision::geometry_instance & )const
{
	UNREACHABLE_CODE(return false);
}

bool	static_element::collide( island& i, const collide_element_interface& o )const
{
	return o.collide( i, m_geometry_instance );
}

bool	static_element::collide( island& , const xray::collision::geometry_instance& , body& )const
{
	UNREACHABLE_CODE(return false);
}

bool	static_element::integrate( const step_counter_ref& sr, float time_delta )
{
	XRAY_UNREFERENCED_PARAMETERS( &sr, time_delta );
	UNREACHABLE_CODE(return false);
}