////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_INTRUSIVE_BASE_H_INCLUDED
#define XRAY_RESOURCES_INTRUSIVE_BASE_H_INCLUDED

#include <xray/debug_signalling_bool.h>

namespace xray {
namespace resources {

class managed_resource;
class unmanaged_resource;

class XRAY_CORE_API base_of_intrusive_base
{
public:
	base_of_intrusive_base	() : m_reference_count (0), m_flags(0) {}
	long					reference_count			()	const	{ return m_reference_count; }

	template				<class Resource>
	signalling_bool			try_unregister_from_fat_or_from_name_registry	(Resource * const object, u32 zero_reference_count = 0) const;

	bool					pinned_by_game_resource_manager	() const { return m_flags & flag_pinned_by_game_resource_manager; }

private:
	bool					is_associated_with_fat	(managed_resource * const object) const;
	bool					is_associated_with_fat	(unmanaged_resource * const object) const;

	void					pin_reference_count_for_query_finished_callback		();
	void					unpin_reference_count_for_query_finished_callback	();
	void					pin_reference_count_for_game_resource_manager		();
	void					unpin_reference_count_for_game_resource_manager		();

private:
	threading::atomic32_type	m_reference_count;
	
	enum flag_pinned_by_enum {
		flag_pinned_by_game_resource_manager	=	1 << 0,
		flag_pinned_by_query_finished_callback	=	1 << 1,
	};

	u8						m_flags;

	friend class			threading::multi_threading_policy_base;
	friend class			threading::single_threading_policy;
	friend class			managed_resource;
	friend class			unmanaged_resource;
	friend class			resources_manager;
	friend class			resource_base;
	friend class			game_resources_manager;
};

class resource;

class XRAY_CORE_API managed_intrusive_base : public base_of_intrusive_base
{
public:
	void		destroy		(managed_resource * object)	const;
};

class unmanaged_resource;

class XRAY_CORE_API unmanaged_intrusive_base : public base_of_intrusive_base
{
public:
	void		destroy		(unmanaged_resource* object)	const;
};

} // namespace resources
} // namespace xray

#endif // #ifndef XRAY_RESOURCES_INTRUSIVE_BASE_H_INCLUDED