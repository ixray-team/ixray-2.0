////////////////////////////////////////////////////////////////////////////
//	Created		: 28.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/resources_managed_resource.h>
#include "resources_allocators.h"
#include "resources_manager.h"

namespace xray {
namespace resources {

//----------------------------------------------------------
// managed_resource
//----------------------------------------------------------

managed_resource::managed_resource (u32	const size, class_id const class_id) 
					: resource_base			(resource_base::is_resource_flag),
					  m_node				(NULL),
					  m_next_delay_delete	(NULL),
					  m_size				(size),
					  m_class_id			(class_id),
					  m_fat_it				(NULL)
{
	m_memory_usage_self.type			=	& memory_type_managed_resource;
}

managed_resource::~managed_resource ()
{
	R_ASSERT							(creation_source() != creation_source_unset);
	R_ASSERT							(m_node);
}

void   managed_resource::resize_down (u32 const new_size)				
{
	R_ASSERT								(new_size);
	R_ASSERT								(new_size <= m_size);
	m_size								=	new_size;

	u32 const node_new_size				=	sizeof(managed_node) + new_size;

	memory::g_resources_managed_allocator.resize_down(m_node, node_new_size);
#ifdef DEBUG
	memory::g_resources_managed_allocator.check_consistency();
#endif // #ifdef DEBUG
}

void   managed_resource::set_is_unmovable (bool const is_unmovable)
{
	ASSERT									(m_node);
	managed_node*	node				=	m_node;

	if ( node->is_unmovable() == is_unmovable )
		return;

	threading::mutex & mutex			=	memory::g_resources_managed_allocator.get_defragmentation_mutex ();
	if ( is_unmovable )
		mutex.lock							();

	node->set_is_unmovable					(is_unmovable);
	R_ASSERT								(& memory::g_resources_managed_allocator.get_node_arena(node) 
												== 
											 & memory::g_resources_managed_allocator);
	memory::g_resources_managed_allocator.notify_unmovable_changed		(node);

	if ( is_unmovable )
		mutex.unlock						();
}

bool   managed_resource::is_unmovable () const
{
	ASSERT									(m_node);
	return									m_node->is_unmovable();
}

pcbyte   managed_resource::pin ()
{
	// copy of m_node to work with, because m_node can be changed at any moment
	managed_node*	node				=	m_node;
	ASSERT									(node);
 	pbyte			data				=	(pbyte)node + sizeof(managed_node);

// 	fs::path_string							full_path;
// 	get_full_path							(full_path);
// 	LOGI_INFO("resources:managed_resource", "pinned managed_resource \"%s\", addr = 0x%08x", 
// 											 full_path.c_str(), 
// 											 data);

	threading::interlocked_increment		(node->m_pin_count);
	return									data;
}

void   managed_resource::unpin (pcbyte const_pinned_data)
{
	pbyte			pinned_data			=	const_cast<pbyte>(const_pinned_data);
	managed_node*	node				=	(managed_node*)
											(pinned_data - sizeof(managed_node));

	threading::interlocked_decrement		(node->m_pin_count);

	if ( node->m_unpin_notify_allocator && !node->m_pin_count )
	{
		node->m_unpin_notify_allocator->notify_unpinned_object();
		node->m_unpin_notify_allocator	=	NULL;
	}

// 	fs::path_string							full_path;
// 	get_full_path							(full_path);
// 	LOGI_INFO("resources:managed_resource", "unpinned managed_resource \"%s\", addr = 0x%08x", 
// 											 full_path.c_str(), 
// 											 (pbyte)pinned_data);
}

u32   managed_resource::get_buffer_size() const
{
	ASSERT									(m_node);
	return									(u32)m_node->m_size;
}

void   managed_resource::get_full_path (fs::path_string & dest) const
{
	ASSERT									(m_node);
	fat_iterator	fat_it				=	wrapper_to_fat_it(m_fat_it);
	fat_it.get_full_path					(dest);
}

void   managed_resource::late_set_fat_it (fat_it_wrapper it)
{
	fat_iterator const old_it			=	wrapper_to_fat_it(m_fat_it);
	fat_iterator const new_it			=	wrapper_to_fat_it(it);
	if ( new_it == old_it )
		return;
	R_ASSERT								(old_it.is_end());
	m_fat_it							=	it;
	m_node->m_fat_it					=	new_it;
}

fixed_string512   managed_resource::log_string () const
{
#ifndef MASTER_GOLD
	pcstr request_path				=		m_request_path.c_str();
#else
	pcstr request_path				=		"<was not saved>";
#endif

	fixed_string512							out_result;
	if ( creation_source() == creation_source_physical_path )
	{
		out_result.assignf					("managed managed_resource with physical path: '%s', size = %d", request_path, m_size);
		return								out_result;
	}
	if ( creation_source() == creation_source_user_data )
	{
		out_result.assignf					("managed managed_resource created from user-data with name '%s', size = %d", request_path, m_size);
		return								out_result;
	}

	fs::path_string							full_path;
	get_full_path							(full_path);

	fat_iterator	fat_it				=	wrapper_to_fat_it(m_fat_it);
	if ( fat_it.is_end() )
	{
		if ( creation_source() == creation_source_translate_query )
			out_result.assignf				("managed managed_resource via translate_query: '%s', size = %d", request_path, m_size);
		else
			out_result.assignf				("managed managed_resource generated because file was not found: '%s', size = %d", request_path, m_size);
		return								out_result;
	}

	R_ASSERT								(creation_source() == creation_source_fat ||
											 creation_source() == creation_source_translate_query);

	if ( !fat_it.is_end() && fat_it.is_compressed() && m_size == fat_it.get_raw_file_size() )
	{
		out_result.appendf					("managed compressed_res '%s', uncompressed = %d, compressed = %d", 
											 full_path.c_str(),
											 fat_it.get_file_size(),
											 m_size);
	}
	else
	{
		out_result.appendf					("managed managed_resource '%s', size = %d, raw_file_size = %d", 
											 full_path.c_str(),
											 m_size,
											 fat_it.is_end() ? 0 : fat_it.get_file_size());
	}

	return									out_result;
}

void   managed_resource::set_creation_source (creation_source_enum creation_source, pcstr request_path)
{ 
	R_ASSERT								(m_creation_source == creation_source_unset); 
	m_creation_source					=	creation_source;
#ifndef MASTER_GOLD
	m_request_path						=	request_path;
#else // #ifndef MASTER_GOLD
	XRAY_UNREFERENCED_PARAMETER				( request_path );
#endif // #ifndef MASTER_GOLD
}

} // namespace resources
} // namespace xray