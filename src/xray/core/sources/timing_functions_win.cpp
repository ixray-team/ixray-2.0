////////////////////////////////////////////////////////////////////////////
//	Created 	: 23.01.2009
//	Author		: Dmitriy Iassenev
//  Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "timing.h"
#include "platform_extensions_win.h"
#include <xray/os_include.h>

#pragma warning(push)
#pragma warning(disable: 4201)
#include <mmsystem.h>
#pragma warning(pop)

#pragma comment(lib, "winmm.lib")

u64	xray::timing::g_qpc_per_second	= 0;

void xray::timing::initialize					( )
{
	timeBeginPeriod				( 1 );

	bool const result			= platform::unload_delay_loaded_library( "winmm.dll" );
	XRAY_UNREFERENCED_PARAMETER	( result );
	R_ASSERT					( result );

	QueryPerformanceFrequency	( ( PLARGE_INTEGER )&g_qpc_per_second );

	errno_t const error			= _controlfp_s( 0, _PC_24, MCW_PC );
	R_ASSERT_U					( !error );
}

u64 xray::timing::query_performance_counter	( )
{
	u64							result;
	QueryPerformanceCounter		( ( PLARGE_INTEGER )&result );
	return						(result);
}