////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources_hdd_manager.h"
#include <xray/resources_query_result.h>

namespace xray {
namespace resources {

bool   hdd_manager::priority_pred::operator () (query_result* r1, query_result* r2)
{
	fat_iterator const fat_it1	=	wrapper_to_fat_it( r1->get_fat_it() );
	fat_iterator const fat_it2	=	wrapper_to_fat_it( r2->get_fat_it() );

	if ( fat_it1.is_end() || fat_it2.is_end() )
		return						false;

	if ( fat_it1.is_db() && fat_it2.is_db() && fs::g_fat->equal_db(fat_it1, fat_it2) )
	{
		return						fat_it1.get_file_offs() > fat_it2.get_file_offs();
	}

	return							r1->get_priority() < r2->get_priority();
}

bool   hdd_manager::can_handle_query (pcstr path) 
{
	XRAY_UNREFERENCED_PARAMETER		(path);
	return							true; 
}

void   hdd_manager::push_query_impl (query_result* res)
{
	m_queue.push					(res);
}

query_result*   hdd_manager::pop_next_query ()
{
	if ( !m_queue.size() )
		return						NULL;

	query_result* const result	=	m_queue.top();
	m_queue.pop						();
	return							result;
}

} // namespace resources
} // namespace xray