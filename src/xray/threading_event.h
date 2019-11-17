////////////////////////////////////////////////////////////////////////////
//	Created 	: 09.04.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_THREADING_EVENT_H_INCLUDED
#define XRAY_THREADING_EVENT_H_INCLUDED

namespace xray {
namespace threading {

class XRAY_CORE_API event : detail::noncopyable 
{
public:
	enum wait_result_enum			{ wait_result_failed, wait_result_signaled, wait_result_still_nonsignaled };
	static u32 const				wait_time_infinite;

public:
						event		( bool initial_state = false );
					   ~event		( );
public:

	void				set			( bool value);
	wait_result_enum	wait		( u32 max_wait_time_ms );

public:
	pvoid				get_handle	( ) const;

private:
#if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
	char XRAY_DEFAULT_ALIGN	m_event[sizeof(pvoid)]; // sizeof(CRITICAL_SECTION);
#elif XRAY_PLATFORM_PS3 // #if XRAY_PLATFORM_WINDOWS | XRAY_PLATFORM_XBOX_360
	char XRAY_DEFAULT_ALIGN	m_event[4];				// sizeof(sys_event_flag_t);
#endif // #elif XRAY_PLATFORM_XBOX_360
}; // class event

} // namespace threading
} // namespace xray

#endif // #ifndef XRAY_THREADING_EVENT_H_INCLUDED