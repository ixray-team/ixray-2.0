////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCES_HDD_MANAGER_H_INCLUDED
#define RESOURCES_HDD_MANAGER_H_INCLUDED

#include "resources_helper.h"
#include "resources_device_manager.h"
#include <queue>

namespace xray {
namespace resources {

class hdd_manager : public device_manager
{
public:
							hdd_manager				(pcstr thread_desc, u32 sector_size) : 
													device_manager(thread_desc, sector_size) {}

	virtual bool			can_handle_query		(pcstr path);
	virtual void			push_query_impl			(query_result* res);
	virtual query_result*	pop_next_query			();

private:
	struct priority_pred	{	
		bool operator () (query_result* r1, query_result* r2);
	};

	typedef	std::priority_queue<	query_result*, 
									vector<query_result*>, 
									priority_pred	>			queue;

	queue					m_queue;
};

} // namespace xray
} // namespace resources

#endif // RESOURCES_HDD_MANAGER_H_INCLUDED