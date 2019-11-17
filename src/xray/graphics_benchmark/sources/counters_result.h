////////////////////////////////////////////////////////////////////////////
//	Created		: 17.06.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef COUNTERS_RESULT_H_INCLUDED
#define COUNTERS_RESULT_H_INCLUDED

#include "benchmark_renderer.h"

namespace xray {
namespace graphics_benchmark {


struct counters_result
{
	double elapsed_time;		// in microseconds
	float fps;					// 1/sec
	float clear_pass_duration;  // in microseconds
	double pixels_per_tick;
	u64 elapsed_ticks;
	u32 triangles_count;
	u32 batch_count;

	std::string as_text(counters_result& base, bool use_base)
	{
		std::string result;
		char buffer[1024];

		if (use_base)
		{
			::sprintf(buffer,"%10f, %10d", 
				base.pixels_per_tick - pixels_per_tick, (s64)elapsed_ticks - (s64)base.elapsed_ticks
			);
		}
		else
			::sprintf(buffer,"%10f, %10d", pixels_per_tick, elapsed_ticks);

		result = buffer;
		return result;
	}
	// more
}; // struct counters_result

} // namespace graphics_benchmark
} // namespace xray

#endif // #ifndef COUNTERS_RESULT_H_INCLUDED