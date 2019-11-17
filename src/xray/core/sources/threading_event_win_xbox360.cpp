////////////////////////////////////////////////////////////////////////////
//	Created 	: 09.04.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/threading_event.h>
#include <xray/os_include.h>

namespace xray { 
namespace threading { 

u32 const event::wait_time_infinite	= INFINITE;

event::event			( bool const initial_state )
{
	COMPILE_ASSERT		( sizeof(m_event) == sizeof(HANDLE), please_change_buffer_size );
	(HANDLE&)m_event	= CreateEvent( NULL, FALSE, initial_state, NULL );
	R_ASSERT			( (HANDLE&)m_event );
}

event::~event			( )
{
	CloseHandle			( (HANDLE&)m_event );
}

void event::set			( bool const value )
{
	if ( value )
		SetEvent		( (HANDLE&)m_event );
	else
		ResetEvent		( (HANDLE&)m_event );
}

event::wait_result_enum event::wait	( u32 const max_wait_time_ms )
{
	DWORD const result	= WaitForSingleObject( (HANDLE&)m_event, max_wait_time_ms );
	if ( result == WAIT_OBJECT_0 )
		return			event::wait_result_signaled;

	if ( result == WAIT_TIMEOUT )
		return			event::wait_result_still_nonsignaled;

	if ( result == WAIT_FAILED )
		return			event::wait_result_failed;

	UNREACHABLE_CODE	( return event::wait_result_failed );
}

pvoid event::get_handle	( ) const
{
	return				(HANDLE&)m_event;
}

} // namespace threading
} // namespace xray