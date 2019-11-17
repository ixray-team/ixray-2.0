////////////////////////////////////////////////////////////////////////////
//	Created 	: 13.10.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_TIMING_TIMER_H_INCLUDED
#define XRAY_TIMING_TIMER_H_INCLUDED

namespace xray {
namespace timing {

class timer {
private:
	u64				m_start_time;

public:
	inline			timer				();
	inline	void	start				();
	inline	u64		get_elapsed_ticks	() const;
	inline	u32		get_elapsed_ms		() const;
	inline	float	get_elapsed_sec		() const;
}; // class timer

} // namespace timing
} // namespace xray

#include <xray/timing_timer_inline.h>

#endif // #ifndef XRAY_TIMING_TIMER_H_INCLUDED