////////////////////////////////////////////////////////////////////////////
//	Created		: 09.11.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TASK_ALLOCATOR_H
#define TASK_ALLOCATOR_H

#include <xray/tasks_task.h>

namespace xray {
namespace tasks {

#define XRAY_TASK_ALLOCATOR_GRANULARITY	16

class XRAY_ALIGN(XRAY_MAX_CACHE_LINE_SIZE) task_allocator
{
public:
						task_allocator		();

	task *				allocate			();
	void				deallocate			(task *);
	void				check_all_free		();

private:
	pbyte				get_base			() const { return (pbyte)m_task_buffer; }

private:
	template < int granularity >
	struct alignment {
		enum { value = ((sizeof(task) - 1)/granularity + 1)*granularity, };
	};

	enum {
		max_task_count	=	u32(950000),
		granularity		=	alignment<XRAY_TASK_ALLOCATOR_GRANULARITY>::value,
	};

	XRAY_ALIGN(XRAY_MAX_CACHE_LINE_SIZE)
	u8					m_task_buffer	[max_task_count * granularity ];

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4201)
	union pointer_and_counter
	{
		threading::atomic64_type	whole;
#if !XRAY_PLATFORM_WINDOWS_64
		struct {
			task *		pointer;
			u32			counter;
		};
#else // #if XRAY_PLATFORM_WINDOWS_64
		task *			pointer;
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
			task *		pointer;
			u32			counter;
		};
	};
#endif // #if defined(_MSC_VER)

	pointer_and_counter	m_free_list;

	friend class		task_manager;
};

} // namespace tasks
} // namespace xray

#endif // #ifndef TASK_ALLOCATOR_H