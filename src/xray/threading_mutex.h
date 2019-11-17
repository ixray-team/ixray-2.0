////////////////////////////////////////////////////////////////////////////
//	Created 	: 25.09.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_THREADING_MUTEX_H_INCLUDED
#define XRAY_THREADING_MUTEX_H_INCLUDED

namespace xray {
namespace threading {

class XRAY_CORE_API mutex : private detail::noncopyable {
public:
					mutex		( );
					~mutex		( );

public:
			void	lock		( );
			void	unlock		( );
			bool	try_lock	( );

private:
#if XRAY_PLATFORM_WINDOWS_32
	char XRAY_DEFAULT_ALIGN	m_mutex[24]; // sizeof(CRITICAL_SECTION);
#elif XRAY_PLATFORM_WINDOWS_64 // #if XRAY_PLATFORM_WINDOWS_32
	char XRAY_DEFAULT_ALIGN	m_mutex[40]; // sizeof(CRITICAL_SECTION);
#elif XRAY_PLATFORM_XBOX_360 // #elif XRAY_PLATFORM_WINDOWS_64
	char XRAY_DEFAULT_ALIGN	m_mutex[28]; // sizeof(CRITICAL_SECTION);
#elif XRAY_PLATFORM_PS3 // #elif XRAY_PLATFORM_XBOX_360
	char XRAY_DEFAULT_ALIGN	m_mutex[24]; // sizeof(sys_lwmutex_t);
#endif // #elif XRAY_PLATFORM_XBOX_360
};

class mutex_raii : private boost::noncopyable {
public:
	inline			mutex_raii	( mutex& mutex ) : m_mutex( mutex )	{ m_mutex.lock		( ); }
					~mutex_raii	( )									{ m_mutex.unlock	( ); }

private:
	mutex&			m_mutex;
};

} // namespace threading
} // namespace xray

#endif // #ifndef XRAY_THREADING_MUTEX_H_INCLUDED