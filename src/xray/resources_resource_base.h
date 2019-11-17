////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_RESOURCE_BASE_H_INCLUDED
#define XRAY_RESOURCES_RESOURCE_BASE_H_INCLUDED

#include <xray/fs_path_string.h>
#include <xray/resources_resource_link.h>
#include <xray/resources_callbacks.h>
#include <xray/fs_platform_configuration.h>
#include <boost/intrusive/rbtree.hpp>

template struct XRAY_CORE_API boost::intrusive::make_set_member_hook< boost::intrusive::link_mode<boost::intrusive::auto_unlink> >;
template class	XRAY_CORE_API boost::intrusive::set_member_hook< boost::intrusive::link_mode<boost::intrusive::auto_unlink> >;
template struct XRAY_CORE_API boost::intrusive::link_mode<boost::intrusive::auto_unlink>;

namespace xray {

namespace fs {
class file_system_impl;
} // namespace fs

namespace resources {

class					queries_result;
typedef					boost::function< void ( queries_result & data ) >	query_callback;

typedef void *			fat_it_wrapper;

class 					managed_resource;
class					query_result_for_cook;
class 					query_result;
class					unmanaged_resource;
class					managed_intrusive_base;
class					unmanaged_intrusive_base;
class					managed_node;
class					inlined_resource;
class					name_registry_entry;
class					base_of_intrusive_base;
class					child_resource_ptr_base;

typedef boost::intrusive::set_member_hook< boost::intrusive::link_mode<boost::intrusive::auto_unlink> >		resource_tree_member_hook;

class XRAY_CORE_API resource_base
{
public:
	enum						creation_source_enum			{	creation_source_unset,
																	creation_source_fat, 
																	creation_source_user_data,
																	creation_source_physical_path,
																	creation_source_created_by_user,
																	creation_source_translate_query,
																	creation_source_deallocate_buffer_helper	};


	fs::path_string				reusable_request_name			();
	class memory_usage const &	memory_usage					() const { return m_memory_usage_self; }

protected:
	enum						flag_t							{	is_resource_flag			=	1 << 0,
																	is_query_result_flag		=	1 << 1,
																	is_unmanaged_resource_flag	=	1 << 2,
																	needs_cook_flag				=	1 << 3,
																};


								resource_base					(flag_t flag);
	virtual					   ~resource_base					();

	void						set_need_cook					();
	bool						needs_cook						() { return !!(m_flag & needs_cook_flag); }
	creation_source_enum		creation_source					() const { return m_creation_source; }
	void						set_name_registry_entry			(name_registry_entry * entry) { m_name_registry_entry = entry; }
	name_registry_entry *		get_name_registry_entry			() const { return m_name_registry_entry; }
	
public:
	resource_tree_member_hook	tree_hook; // need to be public

private:
	managed_resource *			cast_managed_resource			();
	unmanaged_resource *		cast_unmanaged_resource 		();
	query_result *				cast_query_result				();
	base_of_intrusive_base *	cast_base_of_intrusive_base		();
	void						set_memory_usage				(class memory_usage const & usage) { m_memory_usage_self = usage; }
	u32							get_reference_count				() const;

protected:
	class memory_usage			m_memory_usage_self;
	name_registry_entry *		m_name_registry_entry;
	resource_base *				m_next_for_query_finished_callback;
	
	u32							m_recalculate_memory_usage_last_frame;
	creation_source_enum		m_creation_source;

	void						link_parent_resource	(resource_base *			parent);
	void						link_child_resource		(child_resource_ptr_base *	child);
	void						unlink_parent_resource	(resource_base *			parent);
	void						unlink_child_resource	(child_resource_ptr_base *	child);
	
private:
	resource_link_list			m_children_resources;
	resource_link_list			m_parents_resources;
	u32							m_flag;

 	template < ::xray::fs::pointer_for_fat_size_enum pointer_for_fat_size>
	friend	class				::xray::fs::fat_node;
	
	friend	class				::xray::fs::file_system;
	friend	class				::xray::fs::file_system_impl;
	friend	class				query_result;
	friend	class				query_result_for_cook;												  
	friend	class				managed_intrusive_base;
	friend	class				unmanaged_intrusive_base;
	friend	class				base_of_intrusive_base;
	friend	class				resources_manager;
	friend	class				game_resources_manager;

	template < typename object_type, typename base_type >
	friend	class				child_resource_ptr;
	friend	class				resource_base_compare;
};

} // namespace resources
} // namespace xray

#endif // #ifndef XRAY_RESOURCES_RESOURCE_BASE_H_INCLUDED