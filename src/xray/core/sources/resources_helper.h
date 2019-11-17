////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCES_HELPER_H_INCLUDED
#define RESOURCES_HELPER_H_INCLUDED

#include "fs_file_system.h"

namespace xray {
namespace resources {
	typedef	fs::file_system::iterator						fat_iterator;
} // namespace resources
} // namespace xray

#include "resources_macros.h"
#include <xray/resources.h>
#include <xray/resources_fs.h>
#include <xray/fs_path.h>

namespace xray {
namespace resources {

#define USER_ALLOCATOR	::xray::memory::g_resources_helper_allocator
#include <xray/std_containers.h>
#undef USER_ALLOCATOR

//----------------------------------------------------------
// general
//----------------------------------------------------------
class				queries_result;

typedef	fastdelegate::FastDelegate< void (bool result) >	store_callback;

fat_iterator		wrapper_to_fat_it 				(fat_it_wrapper wrapper);
fat_it_wrapper		fat_it_to_wrapper 				(fat_iterator it);

class				query_result;
fs::path_string		get_native_file_path			(query_result const* query, bool assert_on_fail = true);

class raw_file_buffer_helper
{
public:
	raw_file_buffer_helper *	next;
	class_id					resource_class;
};

} // namespace resources
} // namespace xray

#endif // #ifndef RESOURCES_HELPER_H_INCLUDED