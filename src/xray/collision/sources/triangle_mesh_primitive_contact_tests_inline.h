////////////////////////////////////////////////////////////////////////////
//	Created		: 13.03.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TRIANGLE_MESH_PRIMITIVE_CONTACT_TESTS_INLINE_H_INCLUDED
#define TRIANGLE_MESH_PRIMITIVE_CONTACT_TESTS_INLINE_H_INCLUDED

#ifdef DEBUG
#	include <xray/render/base/debug_renderer.h>
#endif // #ifdef DEBUG

namespace xray {
namespace collision {
	struct on_contact;

namespace detail {

template < class T >
inline	triangle_mesh_primitive_contact_tests<T>::triangle_mesh_primitive_contact_tests(	const float4x4 &transform,
																 buffer_vector<result>& result_buff, on_contact& c ):
	 m_transform(transform), res_vec( result_buff ), cb(c), back_res(c)
{
}
template<class T>
inline	bool	triangle_mesh_primitive_contact_tests<T>::trash_back( const result &back_tri, float3 const* verts )
{
	return detail::trash_back_by_all_res( back_tri, res_vec, verts, m_transform.c.xyz() );
}
template<class T>
inline void	triangle_mesh_primitive_contact_tests<T>::back_test			( result &back_tri, float3 const* verts )
{
	triangle_sphere_back_test( back_res, back_tri, verts, m_transform.c.xyz(), m_radius );
}
template<class T>
void	triangle_mesh_primitive_contact_tests<T>::test( buffer_vector<u32> &backs, const u32 * ind, float3 const* verts )
{
	XRAY_UNREFERENCED_PARAMETERS (verts, &backs, ind);

#ifdef DEBUG
	result &res = res_vec.back();
	const float3	&v0 = verts[ res.v0 ], &v1 = verts[ res.v1 ], &v2 = verts[ res.v2 ];
	math::color positiv_color( 0u, 0u, 255u, 255u ) ;
	math::color back_color( 0u, 0u, 255u, 255u ) ;
	math::color color		= res.b_in_plane ? back_color: positiv_color;
	//cb_r.cb.render_world()->debug().draw_triangle( v0, v1, v2, color );
	cb.renderer().draw_line( v0, v1, color );
	cb.renderer().draw_line( v1, v2, color );
	cb.renderer().draw_line( v2, v0, color );
#endif // #ifdef DEBUG
}



}//detail
}//collision
}//xray
#endif // #ifndef TRIANGLE_MESH_PRIMITIVE_CONTACT_TESTS_INLINE_H_INCLUDED