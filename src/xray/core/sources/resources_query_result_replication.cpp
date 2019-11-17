////////////////////////////////////////////////////////////////////////////
//	Created		: 28.12.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/resources_query_result.h>
#include "resources_manager.h"
#include "resources_managed_allocator.h"

namespace xray {
namespace resources {

using namespace fs;

void   query_result::init_replication (fat_it_wrapper fat_it, managed_resource_ptr resource, int priority) 
{
	R_ASSERT							(resource);
	unset_flag							(flag_load | flag_save);
	set_flag							(flag_replication);
	m_fat_it						=	fat_it;
	m_raw_managed_resource			=	resource;
	m_priority						=	priority;
}

void   query_result::init_save (pcstr physical_path, mutable_buffer const & data, query_result * data_to_save_generator, int priority) 
{
	R_ASSERT							(data.size());
	m_data_to_save_generator		=	data_to_save_generator;
	unset_flag							(flag_load | flag_replication);
	set_flag							(flag_save);
	strings::copy						(m_request_path, m_request_path_max_size, physical_path);
	m_generated_data_to_save		=	data;
	m_priority						=	priority;
}

void   query_result::replicate ()
{
	R_ASSERT							( is_load_type() && is_success() );
	fat_iterator fat_it				=	wrapper_to_fat_it(m_fat_it);

	managed_resource_ptr						replication_resource;

	if ( !cook_base::does_create_resource(m_class_id) )
	{
		replication_resource		=	m_raw_managed_resource;
	}
	else
	{
		replication_resource		=	g_resources_manager->allocate_managed_resource(m_raw_managed_resource->get_size());

		pinned_ptr_const<u8>	src_data		(m_raw_managed_resource);

		if ( replication_resource )
		{
			pinned_ptr_const<u8>	dest_data	(replication_resource);
			memory::copy				((pvoid)dest_data.c_ptr(), dest_data.size(), (pvoid)src_data.c_ptr(), src_data.size());
		}
		else
		{
			// no memory, synchronous write
			fs::g_fat->replicate_file	(fat_it, src_data.c_ptr());

			LOGI_INFO("resources:resource", "synchronously replicated due to low memory %s", 
											m_raw_managed_resource->log_string ().c_str());
			return;
		}
	}

	g_resources_manager->replicate_resource (fat_it, replication_resource, NULL, low_priority);
}

} // namespace resources
} // namespace xray

