////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCES_TEST_BASE_COOK_H_INCLUDED
#define RESOURCES_TEST_BASE_COOK_H_INCLUDED

#include "resources_test_resource.h"

namespace xray {
namespace resources {

extern	fixed_vector<fs::path_string, 16>	s_created_files;

enum generate_if_no_file_test_enum	{ generate_if_no_file_test_unset, generate_if_no_file_test_requery, generate_if_no_file_test_save_generated };

class test_cook_base
{
public:
	test_cook_base (generate_if_no_file_test_enum generate_if_no_file_test) : m_generate_if_no_file_test(generate_if_no_file_test) {}

	generate_if_no_file_test_enum	generate_if_no_file_test () const { return m_generate_if_no_file_test; }

	void   on_generate_helper_resource_loaded (queries_result & result)
	{
		query_result_for_cook * const	parent	=	result.get_parent_query();
		fs::path_string					disk_path;

		if ( !parent->select_disk_path_from_request_path(& disk_path) )
		{
			parent->finish_query	(cook_base::result_error);
			return;
		}

		FILE * file					=	NULL;
		if ( !fs::open_file(& file, fs::open_file_create | fs::open_file_truncate | fs::open_file_write, disk_path.c_str()) )
		{
			parent->finish_query	(cook_base::result_error);
			return;
		}

		mutable_buffer					generated_file_data(ALLOCA(generate_file_data_size), generate_file_data_size);
		fill_with_generated_file_data	(generated_file_data);		
		fwrite							(generated_file_data.c_ptr(), generated_file_data.size(), 1, file);
		fclose							(file);

		s_created_files.push_back		(disk_path.c_str());
		parent->finish_query		(cook_base::result_requery);
	}

private:
	generate_if_no_file_test_enum	m_generate_if_no_file_test;
};

} // namespace resources
} // namespace xray

#endif // #ifndef RESOURCES_TEST_BASE_COOK_H_INCLUDED
