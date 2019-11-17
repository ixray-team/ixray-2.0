////////////////////////////////////////////////////////////////////////////
//	Created		: 13.03.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TRIANGLE_CYLINDER_CONTACT_TESTS_INLINE_H_INCLUDED
#define TRIANGLE_CYLINDER_CONTACT_TESTS_INLINE_H_INCLUDED
#include "cylinder_geometry.h"
namespace xray {
namespace collision {
struct on_contact;
namespace detail{

inline	triangle_cylinder_contact_tests::triangle_cylinder_contact_tests	(	const cylinder_geometry &g, const float4x4 &transform,
																			 buffer_vector<result>& result_buff, on_contact& c ):
	super( transform,  result_buff, c ),
	m_geometry(g)
{

}
inline void	triangle_cylinder_contact_tests::get_query_aabb			(  const float4x4 &transform, const cylinder_geometry& og,   math::aabb	&query_aabb )
{
	og.get_aabb( query_aabb, transform );
}





}//detail {
}//collision{
}//xray{

#endif // #ifndef TRIANGLE_CYLINDER_CONTACT_TESTS_INLINE_H_INCLUDED