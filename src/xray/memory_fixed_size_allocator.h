////////////////////////////////////////////////////////////////////////////
//	Created		: 06.11.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef MEMORY_FIXED_SIZE_ALLOCATOR_H_INCLUDED
#define MEMORY_FIXED_SIZE_ALLOCATOR_H_INCLUDED

#include <xray/memory_debug_parameters.h>

namespace xray {
namespace memory {

template <class T, class ThreadPolicy = threading::multi_threading_mutex_policy>
class fixed_size_allocator : public base_allocator
{
public:
					fixed_size_allocator	(u32 granularity = sizeof(node));

	T *				allocate			();
	void			deallocate			(T * );

	virtual	size_t	total_size			() const { return	m_max_count * sizeof(T); }
	virtual	size_t	allocated_size		() const { return	m_allocated_count * sizeof(T); }

private:
	virtual	void	initialize_impl		(pvoid arena, u64 size, pcstr arena_id);
	virtual	void	finalize_impl		() {}

	virtual	pvoid	call_malloc			(size_t size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_DECLARATION XRAY_CORE_DEBUG_PARAMETERS_DECLARATION);
	virtual	pvoid	call_realloc		(pvoid pointer, size_t new_size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_DECLARATION XRAY_CORE_DEBUG_PARAMETERS_DECLARATION);
	virtual	void	call_free			(pvoid pointer XRAY_CORE_DEBUG_PARAMETERS_DECLARATION);

private:
	struct node
	{
		node *			next;
		T				data;
	};

	pcstr				m_arena_id;
	u32					m_granularity;
	u32					m_max_count;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4201)
	union pointer_and_counter
	{
		threading::atomic64_type	whole;
#if !XRAY_PLATFORM_WINDOWS_64
		struct {
			node *		pointer;
			u32			counter;
		};
#else // #if XRAY_PLATFORM_WINDOWS_64
		node *			pointer;
		struct
		{
			u64			dummy	: 40;
			u64			counter	: 24;
		};
#endif // #if XRAY_PLATFORM_WINDOWS_64
	};
#pragma warning(pop)
#else // #if defined(_MSC_VER)
	union pointer_and_counter
	{
		threading::atomic64_type	whole;
		struct {
			node *		pointer;
			u32			counter;
		};
	};
#endif // #if defined(_MSC_VER)

	pointer_and_counter				m_free_list;
	threading::atomic32_type		m_allocated_count;
};

} // namespace memory
} // namespace xray

#include <xray/memory_fixed_size_allocator_inline.h>

#endif // #ifndef MEMORY_FIXED_SIZE_ALLOCATOR_H_INCLUDED