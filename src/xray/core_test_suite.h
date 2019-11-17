////////////////////////////////////////////////////////////////////////////
//	Created		: 22.06.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_CORE_TEST_SUITE_H_INCLUDED
#define XRAY_CORE_TEST_SUITE_H_INCLUDED

#include <xray/testing.h>
#include <xray/linkage_helper.h>

INCLUDE_TO_LINKAGE(resources_test);
INCLUDE_TO_LINKAGE(compressor_test);
INCLUDE_TO_LINKAGE(threading_test);
INCLUDE_TO_LINKAGE(math_tests);

namespace xray {

class XRAY_CORE_API core_test_suite : public testing::suite_base<core_test_suite>
{
public:
	DEFINE_SUITE_HELPERS

public:
	void	set_resource_path	(pcstr resource_path)	{ m_resource_path = resource_path; }
	pcstr	get_resource_path	() const				{ return m_resource_path.c_str(); }

private:
	fs::path_string		m_resource_path;
}; // class core_test_suite

} // namespace xray

#endif // #ifndef XRAY_CORE_TEST_SUITE_H_INCLUDED