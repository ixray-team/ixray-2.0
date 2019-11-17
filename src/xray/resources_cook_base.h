////////////////////////////////////////////////////////////////////////////
//	Created 	: 08.10.2008
//	Author		: Sergey Chechin, Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_COOK_H_INCLUDED
#define XRAY_RESOURCES_COOK_H_INCLUDED

#include <xray/resources_classes.h>
#include <xray/fs_path_string.h>

namespace xray {
namespace resources {

class unmanaged_resource;
class query_result_for_cook;
class unmanaged_cook;
class managed_cook;
class inplace_unmanaged_cook;
class inplace_managed_cook;	
class translate_query_cook;	

class XRAY_CORE_API cook_base
{
public:
	static u32  const					thread_id_unset					=	(u32)-1;
	static u32	const 					use_cooker_thread_id			=	(u32)-2;
	static u32	const 					use_current_thread_id			=	(u32)-3;
	static u32	const 					use_resource_manager_thread_id	=	(u32)-4;
	static u32	const 					use_user_thread_id				=	(u32)-5;

	enum reuse_enum					{	reuse_false, reuse_true, reuse_raw	};

	enum result_enum				{	result_undefined,	// do not return this from cook
										result_error, 
										result_postponed,
										result_success, 
										result_requery,
										result_out_of_memory	};

protected:
	enum flags_enum					{	flag_does_translate_query			=	1 << 1,
										flag_does_inplace_cook				=	1 << 2,
										flag_creates_managed_resource		=	1 << 3,	
									};

public:
			cook_base					(class_id					resource_class, 
										 reuse_enum					reuse_type,
										 enum_flags<flags_enum>		flags,
										 u32						allocate_thread_it,
										 u32 						creation_thread_id);

	virtual	void						translate_request_path		(pcstr request, fs::path_string & new_request) const { new_request = request; }

			class_id					get_class_id				() const;
			bool						has_flag					(flags_enum flag) const { return m_flags & flag; }

			unmanaged_cook *			cast_unmanaged_cook			();
			managed_cook *				cast_managed_cook			();
			inplace_unmanaged_cook *	cast_inplace_unmanaged_cook	();
			inplace_managed_cook *		cast_inplace_managed_cook	();
			translate_query_cook *		cast_translate_query_cook	();
	
	static	cook_base *					find_cook					(class_id resource_class);
	static	unmanaged_cook *			find_unmanaged_cook			(class_id resource_class);
	static	managed_cook *				find_managed_cook			(class_id resource_class);
	static	inplace_unmanaged_cook *	find_inplace_unmanaged_cook	(class_id resource_class);
	static	inplace_managed_cook *		find_inplace_managed_cook	(class_id resource_class);
	static	translate_query_cook *		find_translate_query_cook	(class_id resource_class);

			bool						cooks_managed_resource		() const;
			bool						cooks_unmanaged_resource	() const { return !cooks_managed_resource(); }

	static	bool						cooks_managed_resource		(class_id resource_class);
	static	bool						cooks_unmanaged_resource	(class_id resource_class) { return !cooks_managed_resource(resource_class); }

			bool						cooks_inplace				() const;
	static	bool						cooks_inplace				(class_id resource_class);

			bool						does_create_resource				() const;
	static	bool						does_create_resource				(class_id resource_class);

			bool						does_create_resource_if_no_file		();
	static	bool						does_create_resource_if_no_file		(class_id resource_class);
	
			reuse_enum					reuse_type					() const;
	static	reuse_enum					reuse_type					(class_id resource_class);

			u32							cook_users_count			() const;

	virtual bool						allow_sync_load_from_inline	() const { return true; }
	static  bool						allow_sync_load_from_inline	(class_id resource_class);
			void						call_destroy_resource		(unmanaged_resource * resource);
			void						call_destroy_resource		(managed_resource * resource);
	virtual void						deallocate_resource			(pvoid buffer) = 0;

			void						to_string					(buffer_string * out_string);
			u32							allocate_thread_id			();
	static	u32							allocate_thread_id			(class_id resource_class);
			u32							creation_thread_id			();

			void						register_object_to_delete	(unmanaged_resource * resource, u32 deallocation_thread_id);

	virtual								~cook_base					() {}

protected:
			void						set_flag					(flags_enum flag) { m_flags |= flag; }

			template <class T>
			pinned_ptr_mutable<T>		pin_for_write				(managed_resource_ptr resource) { return pinned_ptr_mutable<T>(resource); }
private:
			cook_base *					get_next					() const		{ return m_next; }
			void						set_next					(cook_base * next)	{ m_next = next; }

			void						change_cook_users_count		(int change)	{ m_cook_users_count.change_cook_users_count(change);	}

private:
	class XRAY_CORE_API cook_users_count_functionality
	{
	public:
										cook_users_count_functionality	() : m_count(0) {}
										~cook_users_count_functionality	()			{ R_ASSERT(!m_count); }
		u32								cook_users_count				() const	{ return m_count; }
		void							change_cook_users_count			(int change);
	private:
		threading::atomic32_type		m_count;
	};

private:
	cook_users_count_functionality		m_cook_users_count;

	class_id							m_class_id;
	reuse_enum							m_reuse_type;
	u32									m_creation_thread_id;
	u32									m_allocate_thread_id;
	enum_flags<flags_enum>				m_flags;

	cook_base *							m_next;

	friend class						resources_manager;
	friend class						unmanaged_resource;
	friend class						query_result;

}; // cook_base

XRAY_CORE_API void					register_cook			(cook_base *	processor);
XRAY_CORE_API cook_base *			unregister_cook			(class_id		resource_class);

pcstr   convert_cook_reuse_value_to_string					(cook_base::reuse_enum value);

} // namespace xray
} // namespace resources

#endif // #ifndef XRAY_RESOURCES_POST_PROCESSOR_H_INCLUDED