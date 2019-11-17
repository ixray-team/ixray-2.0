#include "pch.h"
#include "test_application.h"

#include "xray/buffer_string.h"
#include "xray/engine/api.h"
#include <xray/core/sources/fs_file_system.h>

#include <xray/core/core.h>
#include <xray/intrusive_list.h>

#include <stdio.h>

#include <xray/os_preinclude.h>
#undef NOUSER
#undef NOMSG
#undef NOWINMESSAGES
#undef NOCTLMGR
#include <xray/os_include.h>
using namespace xray;

#include <xray/core/simple_engine.h>
#include <xray/uninitialized_reference.h>
#include <xray/type_variant.h>

xray::memory::doug_lea_allocator_type		g_test_allocator;

using namespace xray::resources;
using namespace xray::fs;

static xray::core::simple_engine		core_engine;

void   application::initialize (u32 , pstr const* )
{
	xray::core::preinitialize					(& core_engine, 
												 GetCommandLine(), 
												 command_line::contains_application_true,
												 "test",
												 __DATE__);
	g_test_allocator.user_current_thread_id		();
	g_test_allocator.do_register				(1 * 1024 * 1024,	"test allocator");
	xray::memory::allocate_region				();
	xray::core::initialize						("test", core::create_log, core::perform_debug_initialization);
}

void   application::finalize ()
{

	xray::core::finalize						();
}


struct sub_resource : public unmanaged_resource
{
	//virtual void recalculate_memory_usage_impl() { }
};

struct resource_with_children : public unmanaged_resource
{
	child_unmanaged_resource_ptr	m_child;

	void set_child (unmanaged_resource_ptr child)
	{
		m_child.initialize_and_set_parent	(this, & resource_with_children::m_child, child.c_ptr());
	}

	//virtual void recalculate_memory_usage_impl() { }
};

void   application::execute ()
{
	resource_with_children	c;
	sub_resource			a;
	c.set_child				(& a);

	
	

	m_exit_code								=	0;
}

