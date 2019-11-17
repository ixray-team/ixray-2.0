////////////////////////////////////////////////////////////////////////////
//	Created		: 28.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_UNMANAGED_RESOURCE_H_INCLUDED
#define XRAY_RESOURCES_UNMANAGED_RESOURCE_H_INCLUDED

#include <xray/resources_resource_base.h>
#include <xray/resources_intrusive_base.h>
#include <xray/resources_managed_resource.h>
#include <xray/resources_resource_ptr.h>

namespace xray {
namespace resources {

class cook_base;

class XRAY_CORE_API unmanaged_resource :	public resource_base, 
											public unmanaged_intrusive_base
{
public:
							unmanaged_resource		();
	virtual				   ~unmanaged_resource 		();

	void					set_creation_source		(creation_source_enum creation_source, pcstr request_path, u32 resource_size);
	managed_resource_ptr	raw_resource_ptr		() const { return m_raw_resource_ptr; }

	void					set_deleter_object		(cook_base * cook, u32 deallocation_thread_id);
	bool					has_deleter_object		() const { return !!m_deleter; }

	u32						deallocate_thread_id	() const { R_ASSERT(m_deallocation_thread_id); return m_deallocation_thread_id; }
	u32						get_size				() const { return m_size; }

private:
	unmanaged_resource *	get_next_delay_delete	() const { return m_next_delay_delete; }
	class_id				get_class_id			() const { return m_class_id; }
	cook_base *				get_deleter_object		() const { return m_deleter; }
	void					late_set_fat_it			(fat_it_wrapper it);
	fixed_string512			log_string				() const;

	void					set_as_inlined_in_fat	();
	bool					is_inlined_in_fat		() const { return m_inlined_in_fat; }
	void					unset_as_inlined_in_fat	();
	void					set_size				(u32 size) { m_size = size; }
	fat_it_wrapper			get_fat_it				() const { return m_fat_it; }

private:
	fat_it_wrapper			m_fat_it;
	class_id				m_class_id;
	cook_base *				m_deleter;
	managed_resource_ptr	m_raw_resource_ptr;
	unmanaged_resource *	m_next_delay_delete;
	unmanaged_resource *	m_next_in_global_list;
	unmanaged_resource *	m_prev_in_global_list;
	unmanaged_resource *	m_next_in_global_delay_delete_list;
	unmanaged_resource *	m_prev_in_global_delay_delete_list;
	u32						m_deallocation_thread_id;
	u32						m_size;
#ifndef MASTER_GOLD
	fs::path_string			m_request_path;
#endif
	bool					m_inlined_in_fat;

	friend class			resources_manager;
	friend class			thread_local_data;
	friend class			unmanaged_intrusive_base;
	friend class			base_of_intrusive_base;
	friend class			resource_base;
	friend class			query_result;
	friend class			query_result_for_cook;
	friend class			game_resources_manager;
	friend class			::xray::fs::file_system_impl;
};

typedef	resource_ptr		<unmanaged_resource, unmanaged_intrusive_base>	unmanaged_resource_ptr;
typedef	child_resource_ptr	<unmanaged_resource, unmanaged_intrusive_base>	child_unmanaged_resource_ptr;

namespace resources_detail {
	typedef resources::query_callback	query_callback;

	#pragma warning( push )
	#pragma warning( disable : 4231 )
	template class XRAY_CORE_API xray::resources::child_resource_ptr <	xray::resources::unmanaged_resource, 	
																		xray::resources::unmanaged_intrusive_base	>;	

	template class XRAY_CORE_API xray::resources::resource_ptr < xray::resources::unmanaged_resource, 	
																 xray::resources::unmanaged_intrusive_base>;	

	template class XRAY_CORE_API xray::intrusive_ptr< xray::resources::unmanaged_resource, 	
													  xray::resources::unmanaged_intrusive_base,
													  xray::threading::multi_threading_mutex_policy>;	
		
	template class XRAY_CORE_API boost::function< void ( xray::resources::queries_result& data ) >;

	#pragma warning( pop )
}

} // namespace resources
} // namespace xray

#endif // #ifndef XRAY_RESOURCES_UNMANAGED_RESOURCE_H_INCLUDED