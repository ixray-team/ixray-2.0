////////////////////////////////////////////////////////////////////////////
//	Created		: 18.01.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RAW_BI_SPLINE_H_INCLUDED
#define RAW_BI_SPLINE_H_INCLUDED

#include <xray/animation/i_bi_spline_data.h>

namespace xray {
namespace maya_animation {

template<typename Point_type, int Dimension>
class raw_bi_spline {
public:
	virtual void evaluate			( float time, Point_type &point )const								= 0;
	virtual void get_data			( animation::i_bi_spline_data<Point_type> &data )const				= 0;
	virtual void build_approximation( const float* points, u32 number, const Point_type &max_epsilon  )	= 0;
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( raw_bi_spline )
}; // class raw_bi_spline

} // namespace maya_animation
} // namespace xray
#endif // #ifndef RAW_BI_SPLINE_H_INCLUDED