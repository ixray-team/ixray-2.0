////////////////////////////////////////////////////////////////////////////
//	Created		: 06.11.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef MEMORY_POOL_ALLOCATOR_INLINE_H_INCLUDED
#define MEMORY_POOL_ALLOCATOR_INLINE_H_INCLUDED

namespace xray {
namespace memory {

template <class T, class ThreadPolicy>
fixed_size_allocator<T, ThreadPolicy>::fixed_size_allocator (u32 const granularity) 
	: m_arena_id(NULL), m_allocated_count(0)
{
	m_granularity					=	math::align_up((u32)sizeof(node), granularity);
	R_ASSERT_CMP						(m_granularity, >=, sizeof(node));
	m_free_list.whole				=	0;
}

template <class T, class ThreadPolicy>
inline void   fixed_size_allocator<T, ThreadPolicy>::initialize_impl (pvoid arena, u64 size, pcstr arena_id)
{
	m_arena_id						=	arena_id;
	R_ASSERT							(size <= u32(-1));
	pbyte const aligned_arena		=	(pbyte)math::align_up((size_t)arena, (size_t)XRAY_DEFAULT_ALIGN_SIZE);
	m_max_count						=	((u32)size - u32(aligned_arena - (pbyte)arena)) / m_granularity;

	for ( u32 i=0; i<m_max_count; ++i )
	{
		node * const current		=	(node *)(aligned_arena + (i * m_granularity));
		current->next				=	(i == m_max_count - 1) ? NULL : (node *)((pbyte)current + m_granularity);
	}

	m_free_list.pointer				=	(node *)aligned_arena;
	m_free_list.counter				=	0;
}

template <class T, class ThreadPolicy>
T *   fixed_size_allocator<T, ThreadPolicy>::allocate ()
{
	pointer_and_counter allocated_node;
	pointer_and_counter new_head;
	do 
	{
		allocated_node				=	m_free_list;
		if ( !allocated_node.pointer )
			break;

		new_head.pointer			=	allocated_node.pointer->next;
		new_head.counter			=	allocated_node.counter + 1;

	} while (
		threading::interlocked_compare_exchange	(
			m_free_list.whole,
			new_head.whole,
			allocated_node.whole
		) != allocated_node.whole
	);

	CURE_ASSERT							(allocated_node.pointer, return NULL, "fixed_size_allocator - out of memory!");

	threading::interlocked_increment	(m_allocated_count);
	return								& allocated_node.pointer->data;
}

template <class T, class ThreadPolicy>
inline pvoid   fixed_size_allocator<T, ThreadPolicy>::call_malloc (size_t size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_DECLARATION XRAY_CORE_DEBUG_PARAMETERS_DECLARATION)
{
	XRAY_CORE_DEBUG_PARAMETERS_UNREFERENCED_GUARD;
	XRAY_UNREFERENCED_PARAMETER			( XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_PARAMETER );
	R_ASSERT							(size == sizeof(T));
	return								allocate();
}

template <class T, class ThreadPolicy>
inline pvoid   fixed_size_allocator<T, ThreadPolicy>::call_realloc (pvoid pointer, size_t new_size XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_DECLARATION XRAY_CORE_DEBUG_PARAMETERS_DECLARATION)
{
	XRAY_CORE_DEBUG_PARAMETERS_UNREFERENCED_GUARD;
	XRAY_UNREFERENCED_PARAMETER			( XRAY_CORE_DEBUG_PARAMETERS_DESCRIPTION_PARAMETER );
	XRAY_UNREFERENCED_PARAMETERS		(new_size, pointer);
	NOT_IMPLEMENTED						(return NULL);
}

template <class T, class ThreadPolicy>
inline void   fixed_size_allocator<T, ThreadPolicy>::call_free (pvoid pointer XRAY_CORE_DEBUG_PARAMETERS_DECLARATION)
{
	XRAY_CORE_DEBUG_PARAMETERS_UNREFERENCED_GUARD;
	deallocate							((T *)pointer);
}

template <class T, class ThreadPolicy>
void   fixed_size_allocator<T, ThreadPolicy>::deallocate (T * data)
{
	u32 const data_offset_in_node	=	(u32)&((node *)NULL)->data;
	node * freeing_node				=	(node *)((pbyte)data - data_offset_in_node);

	pointer_and_counter 				head;
	pointer_and_counter 				new_head;
	new_head.pointer				=	freeing_node;

	do {
		head										=	m_free_list;
		new_head.pointer->next						=	head.pointer;
		new_head.counter							=	head.counter;
	} while (
		threading::interlocked_compare_exchange	(
			m_free_list.whole,
			new_head.whole,
			head.whole
		) != head.whole
	);

	threading::interlocked_decrement	(m_allocated_count);
}

} // namespace memory
} // namespace xray

#endif // #ifndef MEMORY_POOL_ALLOCATOR_INLINE_H_INCLUDED