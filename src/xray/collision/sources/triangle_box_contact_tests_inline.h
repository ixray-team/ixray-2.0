////////////////////////////////////////////////////////////////////////////
//	Created		: 10.03.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TRIANGLE_BOX_CONTACT_TESTS_INLINE_H_INCLUDED
#define TRIANGLE_BOX_CONTACT_TESTS_INLINE_H_INCLUDED
#include "box_geometry.h"
namespace xray {
namespace collision {
namespace detail{
inline	triangle_box_contact_tests::triangle_box_contact_tests(	const box_geometry &g, const float4x4 &transform,
																 buffer_vector<result>& result_buff, on_contact& c ):
	super( transform,  result_buff, c ),
	m_geometry(g)
{
	m_radius = g.half_sides().magnitude();
}
	

inline void	triangle_box_contact_tests::get_query_aabb(  const float4x4 &transform, const box_geometry& og,   math::aabb	&query_aabb )
{
	

	const float3 size = float3( transform.i.xyz() * og.half_sides().x ).abs() +  
						float3( transform.j.xyz() * og.half_sides().y ).abs() + 
						float3( transform.k.xyz() * og.half_sides().z ).abs() ;

	query_aabb = math::create_min_max( transform.c.xyz() - size, transform.c.xyz()  + size );
}


}//xray{
}//collision{
}//detail{
#endif // #ifndef TRIANGLE_BOC_CONTACT_TESTS_INLINE_H_INCLUDED