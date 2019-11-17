////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include <xray/resources.h>
#include <xray/resources_cook_classes.h>
#include <xray/resources_fs.h>
#include <xray/fs_path_string.h>
#include <xray/fs_utils.h>
#include "fs_helper.h"
#include "memory.h"
#include <xray/core_test_suite.h>
#include "resources_manager.h"
#include "resources_impl.h"
#include "resources_managed_allocator.h"

#include "resources_test_managed.h"
#include "resources_test_inplace_managed.h"
#include "resources_test_unmanaged.h"
#include "resources_test_inplace_unmanaged.h"
#include "resources_test_translate_query.h"
#include "resources_test_configuration.h"

// If this code works, it was written by Lain. If not, I don't know who wrote it

DECLARE_LINKAGE_ID(resources_test)

namespace xray {
namespace resources {

fixed_vector<fs::path_string, 16>	s_created_files;

class resource_tester
{
public:

public:
	void   on_mount_completed (bool result)
	{
		TEST_ASSERT					(result);
	}

	void   resources_callback (xray::resources::queries_result & queries)
	{
		TEST_CURE_ASSERT			(queries.is_successful(), return);
		for ( u32 i=0; i<queries.size(); ++i )
			TEST_CURE_ASSERT		(queries[i].is_success(), return);
		LOGI_INFO					("resources:test", "callback called with success");
	}

	void   work_with_specific_cooker	(cook_base * cook, test_configuration_type const & test_configuration)
	{
		fs::path_string	db_path		=	m_resource_path;
		fs::path_string	disk_path	=	m_resource_path;

		db_path.appendf				("/test/test%s.db", platform::big_endian() ? "-be" : ""); 
		disk_path					+=	"/test/disk";

		query_mount_db				("test", 
									 db_path.c_str(), 
									 NULL, 
									 false, 
									 boost::bind(&resource_tester::on_mount_completed, this, _1), 
									 &::xray::memory::g_mt_allocator);
		query_mount_disk			("test/disk", disk_path.c_str(), 
									 boost::bind(&resource_tester::on_mount_completed, this, _1), 
									 &::xray::memory::g_mt_allocator);

		if ( test_configuration.wait_after_mount )
			threading::yield		(50);

		request normal_request[] = 
		{
   			{ "test/folder1/2-866.no_inline",		test_resource_class },
      		{ "test/folder2/1-1260.no_inline",		test_resource_class },
     		{ "test/1-80.data",						test_resource_class },
    		{ "test/2-119.data",					test_resource_class },
     		{ "test/3-1215.test",					test_resource_class },
   			{ "test/4-4.test",						test_resource_class },
     		{ "test/5-5.test",						test_resource_class },
     		{ "test/6-6.test",						test_resource_class },
 	  		{ "test/disk/folder1/2-866.no_inline",	test_resource_class },
 			{ "test/disk/folder2/1-1260.no_inline",	test_resource_class },
  			{ "test/disk/1-80.data",				test_resource_class },
  			{ "test/disk/2-119.data",				test_resource_class },
  			{ "test/disk/3-1215.test",				test_resource_class },
  			{ "test/disk/4-4.test",					test_resource_class },
  			{ "test/disk/5-5.test",					test_resource_class },
  			{ "test/disk/6-6.test",					test_resource_class },
		};

// 		if ( test_configuration.iterations_todo % 2 )
// 			normal_request[0].path		=	"test/disk/4-4.test";

		request inplace_inline_request[] = 
		{
   			{ "test/inplace_inline.inplace_inline",		test_resource_class },
		};

		request request_for_file_that_dont_exist_reuse_true[]	=	
		{
			{ "test/disk/this_file_dont_exist1",	test_resource_class },
			{ "test/disk/this_file_dont_exist2",	test_resource_class },
			{ "test/disk/this_file_dont_exist1",	test_resource_class },
		};

		request request_for_file_that_dont_exist_reuse_false[]	=
		{
			{ "test/disk/this_file_dont_exist1",	test_resource_class },
			{ "test/disk/this_file_dont_exist2",	test_resource_class },
			{ "test/disk/this_file_dont_exist3",	test_resource_class },
		};

		request request_for_translate_query[]	=
		{
			{ "translate_query_request1",			test_resource_class },
			{ "translate_query_request2",			test_resource_class },
			{ "translate_query_request1",			test_resource_class },
		};

		if ( test_configuration.do_query_create_resource )
		{
			u32	src_buffer_size				=	creation_data_size;
			u32 src_buffer_offs_to_raw_data	=	0;
			if ( test_configuration.inplace_in_creation_or_inline_data )
			{
				R_ASSERT						(test_configuration.cook_type == cook_type_inplace_unmanaged);
				src_buffer_size				+=	sizeof(test_unmanaged_resource);
				src_buffer_offs_to_raw_data	=	sizeof(test_unmanaged_resource);
			}

			pstr src_buffer_data			=	test_configuration.inplace_in_creation_or_inline_data ? 
												(pstr)MT_ALLOC(char, src_buffer_size) : (pstr)ALLOCA(src_buffer_size);

			for ( u32 i=0; i<creation_data_size; ++i )
				src_buffer_data[src_buffer_offs_to_raw_data + i]	=	(i % 2) ? '0' : '1';

			const_buffer	 src_buffer			(src_buffer_data, src_buffer_size);
			creation_request creation_requests[]	=	
			{
				{ "create_resource_request1", src_buffer,	test_resource_class },
				{ "create_resource_request2", src_buffer,	test_resource_class },
				{ "create_resource_request1", src_buffer,	test_resource_class },
			};

			u32 const creation_requests_count	=	test_configuration.inplace_in_creation_or_inline_data ? 1 : array_size(creation_requests);

			query_create_resources_and_wait		(creation_requests, 
												 creation_requests_count,
												 boost::bind(&resource_tester::resources_callback, this, _1), 
												 & memory::g_mt_allocator);
		}
		else
		{
			request * requests				=	test_configuration.inplace_in_creation_or_inline_data ? 
												inplace_inline_request : normal_request;
			
			if ( test_configuration.inplace_in_creation_or_inline_data )
				R_ASSERT						(test_configuration.cook_type == cook_type_inplace_unmanaged);

			u32 requests_count				=	test_configuration.inplace_in_creation_or_inline_data ? 
												array_size(inplace_inline_request) : array_size(normal_request);

			if ( test_configuration.cook_type == cook_type_translate_query )
			{
				requests					=	request_for_translate_query;
				requests_count				=	array_size(request_for_translate_query);
			}

			if ( cook->does_create_resource_if_no_file() )
			{
				if ( test_configuration.reuse_type == cook_base::reuse_false )
				{
					requests				=	request_for_file_that_dont_exist_reuse_false;
					requests_count			=	array_size(request_for_file_that_dont_exist_reuse_false);
				}
				else
				{
					requests				=	request_for_file_that_dont_exist_reuse_true;
					requests_count			=	array_size(request_for_file_that_dont_exist_reuse_true);
				}
			}

			query_resources_and_wait			( requests, 
												  requests_count,
												  boost::bind(&resource_tester::resources_callback, this, _1), 
												  & memory::g_mt_allocator	);
		}
			
		resources::wait_and_dispatch_callbacks	(true);

		query_unmount_disk					("test/disk", disk_path.c_str(), 
 											 boost::bind(&resource_tester::on_mount_completed, this, _1), 
											 &::xray::memory::g_mt_allocator); 
		query_unmount_db					("test", db_path.c_str(), 
											 boost::bind(&resource_tester::on_mount_completed, this, _1), 
											 &::xray::memory::g_mt_allocator);

	}

	void unregister_cook (cook_base * cook)
	{
		u32 const time_for_cleanup	=	1000;

		timing::timer				timer;
		timer.start					();
		while ( cook->cook_users_count() && (timer.get_elapsed_ms() < time_for_cleanup || debug::is_debugger_present()) )
			resources::dispatch_callbacks	();

		CURE_ASSERT					(timer.get_elapsed_ms() < time_for_cleanup || debug::is_debugger_present(), return, "cooker has failed to delete all resources/buffers, though it was given %d ms of time", time_for_cleanup);

		resources::unregister_cook	(cook->get_class_id());
	}

	void test (core_test_suite * suite)
	{
		m_resource_path			=	suite->get_resource_path();

		if ( debug::is_debugger_present() )
		{
			//logging::push_rule				( "core:file_system", logging::silent );
			
			logging::push_rule				( "core:resources:allocator", logging::silent );
			logging::push_rule				( "core:resources:test", logging::trace );
			logging::push_rule				( "core:resources:device_manager", logging::trace );
			logging::push_rule				( "core:resources:detail", logging::trace );
		}

		m_test_iteration_ordinal		=	1;
		test_configuration_type				test_configuration(1);

		do {

			cook_base * cook			=	NULL;
			if ( test_configuration.cook_type == cook_type_managed )
			{ 
				cook					=	MT_NEW(test_managed_cook)
											(test_configuration.reuse_type,
											 test_configuration.create_in_current_thread,
											 test_configuration.generate_if_no_file_test);
			}
			else if ( test_configuration.cook_type == cook_type_inplace_managed )
			{
				cook					=	MT_NEW(test_inplace_managed_cook)
											(test_configuration.reuse_type,
											 test_configuration.create_in_current_thread,
											 test_configuration.generate_if_no_file_test);
			}
			else if ( test_configuration.cook_type == cook_type_unmanaged )
			{
				cook					=	MT_NEW(test_unmanaged_cook)
											(test_configuration.reuse_type,
											 test_configuration.create_in_current_thread,
											 test_configuration.allocate_in_current_thread,
											 test_configuration.generate_if_no_file_test);
			}
			else if ( test_configuration.cook_type == cook_type_inplace_unmanaged )
			{
				cook					=	MT_NEW(test_inplace_unmanaged_cook)
											(test_configuration.reuse_type,
											 test_configuration.create_in_current_thread,
											 test_configuration.allocate_in_current_thread || test_configuration.inplace_in_creation_or_inline_data,
											 test_configuration.generate_if_no_file_test,
											 test_configuration.inplace_in_creation_or_inline_data);
			}
			else if ( test_configuration.cook_type == cook_type_translate_query )
			{
				cook					=	MT_NEW(test_translate_query_cook)
											(test_configuration.reuse_type,
											 test_configuration.translate_in_current_thread);
			}
			else
				NOT_IMPLEMENTED();

			register_cook					(cook);

			fixed_string512					cook_string;
			cook->to_string					(& cook_string);

			if ( test_configuration.generate_if_no_file_test == generate_if_no_file_test_requery )
				cook_string				+=	"+requery";
			else if ( test_configuration.generate_if_no_file_test == generate_if_no_file_test_save_generated )
				cook_string				+=	"+save_generated";
			else if ( test_configuration.do_query_create_resource )
				cook_string				+=	"+query_create_resource";

			if ( test_configuration.inplace_in_creation_or_inline_data )
				cook_string				+=	"+inplace_inline";

			LOGI_INFO("resources:test",	"TEST ITERATION %d | cook: %s, cook_reuse: %s, create_in_current_thread: %d, allocate_in_current_thread: %d, sleep_after_mount: %d", 
										m_test_iteration_ordinal++, cook_string.c_str(), 
										convert_cook_reuse_value_to_string(test_configuration.reuse_type), 
										test_configuration.create_in_current_thread, 
										test_configuration.allocate_in_current_thread, 
										test_configuration.wait_after_mount);

			work_with_specific_cooker	(cook, test_configuration);

			unregister_cook				(cook);

			delete_files_created_by_cook	();		

			MT_DELETE					(cook);

		} while ( test_configuration.move_next() );

		//threading::yield				(100);
		wait_and_dispatch_callbacks		(true);
	}

	struct on_test_file_erased_predicate_type
	{
		on_test_file_erased_predicate_type (u32 files_left) : files_left(files_left) {}
		void   callback (bool result)
		{
			TEST_ASSERT	(result);
			R_ASSERT	(files_left);
			--files_left;
		}

		u32				files_left;
	};

	void   delete_files_created_by_cook ()
	{
		on_test_file_erased_predicate_type on_test_file_erased_predicate((u32)s_created_files.size());

		for ( u32 i=0; i<s_created_files.size(); ++i )
		{
			if ( s_created_files[i].length() )
			{
				resources::query_erase_file	(s_created_files[i].c_str(),
											 boost::bind(& on_test_file_erased_predicate_type::callback, & on_test_file_erased_predicate, _1),
											 & memory::g_mt_allocator);
			}
		}

		u32 const time_for_cleanup	=	1000;
		timing::timer	timer;
		timer.start					();
		while ( on_test_file_erased_predicate.files_left && 
			  (timer.get_elapsed_ms() < time_for_cleanup || debug::is_debugger_present()) )
		{
			if ( threading::g_debug_single_thread )
				resources::tick				();

			resources::dispatch_callbacks	();
		}

		s_created_files.clear		();

		CURE_ASSERT					(timer.get_elapsed_ms() < time_for_cleanup || debug::is_debugger_present(), 
									 return, 
									 "test has failed to delete all temporary files, though it was given %d ms of time", 
									 time_for_cleanup);
	}

private:
// 	void   fs_callback (fs_iterator it)
// 	{
// 		log_fs_iterator					(it);
// 	}
// 	query_fs_iterator_and_wait	("", boost::bind(& resource_tester::fs_callback, this, _1), & memory::g_mt_allocator);

	fs::path_string		m_resource_path;
	u32					m_test_iteration_ordinal;
	bool				m_stop_dispatch;
};

REGISTER_TEST_CLASS(resource_tester, core_test_suite);



} // namespace xray
} // namespace resources











































/*@                                    /\  /\
 * @                                  /  \/  \                        ----- |   | ----      |---\ |    | /--\  --- |   |  ---- /--\ /--\
 *  @                                /        --                        |   |   | |         |   / |    | |      |  |\  |  |    |    |
 *   \---\                          /           \                       |   |---| ----      |--/  |    |  \     |  | \ |  ----  \    \
 *    |   \------------------------/       /-\    \                     |   |   | |         |  \  |    |   -\   |  |  \|  |      -\   -\
 *    |                                    \-/     \                    |   |   | ----      |---/  \--/  \--/  --- |   \  ---- \--/ \--/
 *     \                                             ------O
 *      \                                                 /                 --- |   | ----  /--\        |--\   /--\   /--\
 *       |    |                    |    |                /                   |  |\  | |    |    |       |   | |    | |
 *       |    |                    |    |-----    -------                    |  | \ | ---- |    |       |   | |    | | /-\
 *       |    |\                  /|    |     \  WWWWWW/                     |  |  \| |    |    |       |   | |    | |    |
 *       |    | \                / |    |      \-------                     --- |   \ |     \--/        |--/   \--/   \--/
 *       |    |  \--------------/  |    |
 *      /     |                   /     |
 *      \      \                  \      \
 *       \-----/                   \-----/
 */

