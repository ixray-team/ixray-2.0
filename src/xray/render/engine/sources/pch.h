////////////////////////////////////////////////////////////////////////////
//	Created 	: 28.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef PCH_H_INCLUDED
#define PCH_H_INCLUDED

#ifndef XRAY_STATIC_LIBRARIES
#	define XRAY_ENGINE_BUILDING
#endif // #ifndef XRAY_STATIC_LIBRARIES

#define XRAY_LOG_MODULE_INITIATOR	"render_dx10"

#include <xray/extensions.h>

#define XRAY_RENDER_BUILDING

#include <xray/render/core/render_include.h>

#include <xray/render/base/memory.h>

#define R_CHK( expression, ... )											\
	if ( xray::identity(true) ) {											\
	static bool ignore_always			= false;						\
	if ( !ignore_always && FAILED( xray::identity(expression) ) ) {		\
	bool do_debug_break = false;									\
	::xray::debug::on_error	(										\
	& do_debug_break,											\
	xray::process_error_true,									\
	&ignore_always,												\
	xray::assert_untyped,										\
	"assertion_failed",											\
	XRAY_MAKE_STRING( expression ),								\
	__FILE__,													\
	__FUNCTION__,												\
	__LINE__,													\
	__VA_ARGS__);												\
	if ( do_debug_break )											\
	DEBUG_BREAK();												\
	}																	\
	} else (void)0

using xray::math::float2;
using xray::math::float3;
using xray::math::color;
using xray::math::aabb;
using xray::math::int2;
using xray::math::uint2;

using namespace xray;
using xray::detail::noncopyable;

using xray::render::vector;
using xray::render::map;
//using xray::render::fixed_vector;
//#include <xray/render/dx10/sources/common/float4x4b.h>
#include <xray/render/core/utils.h>
#include <xray/render/core/quasi_singleton.h>

//	Include this before any renderer-specific includes
#include <xray/render/engine/namespace_wrapper.h>


#include <bitset>

#include <xray/render/engine/shared_names.h>

// #include <xray/render/dx10/sources/common/device.h>
//#include <xray/render/dx10/sources/common/backend_handlers.h>
//#include <xray/render/dx10/sources/common/backend.h>


// --Porting to DX10_
// //#include <xray/render/dx10/sources/common/fvf.h>
// #include <xray/render/dx10/sources/common/manager_common_inline.h>
// 
// #include <xray/render/dx10/sources/common/res_common.h>
// #include <xray/render/dx10/sources/common/res_constant_table.h>
// #include <xray/render/dx10/sources/common/res_declaration.h>
// #include <xray/render/dx10/sources/common/res_texture.h>
// #include <xray/render/dx10/sources/common/res_rt.h>
// #include <xray/render/dx10/sources/common/res_effect.h>
// 
// #include <xray/render/dx10/sources/common/effect.h>
// #include <xray/render/dx10/sources/common/effect_compiler.h>
// 
// #include <xray/render/dx10/sources/common/resource_manager.h>
// //#include <xray/render/dx10/sources/common/effect_manager.h>
// #include <xray/render/dx10/sources/common/material_manager.h>
// //#include <xray/render/dx10/sources/common/lights_db.h>

#include <xray/render/core/pix_event_wrapper.h>

//#include "visual.h"
#include <xray/render/engine/render_queue.h>
//#include "model_manager.h"

extern bool g_default_mode;

#endif // #ifndef PCH_H_INCLUDED