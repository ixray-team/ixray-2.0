////////////////////////////////////////////////////////////////////////////
//	Created 	: 13.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_TIMING_TIMER_INLINE_H_INCLUDED
#define XRAY_TIMING_TIMER_INLINE_H_INCLUDED

inline xray::timing::timer::timer					( )
{
	m_start_time	= 0;
}
inline void xray::timing::timer::start				( )
{
	m_start_time	= get_QPC( );
}

inline u64 xray::timing::timer::get_elapsed_ticks	( ) const
{
	return			( get_QPC( ) - m_start_time );
}

inline u32 xray::timing::timer::get_elapsed_ms		( ) const
{
	return			( u32( get_elapsed_ticks( )*u64( 1000 ) / g_qpc_per_second ) );
}

inline float xray::timing::timer::get_elapsed_sec	( ) const
{
	return			( float( double(get_elapsed_ticks())/g_qpc_per_second ) );
}

#endif // #ifndef XRAY_TIMING_TIMER_INLINE_H_INCLUDED