////////////////////////////////////////////////////////////////////////////
//	Created		: 20.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RES_GEOMETRY_H_INCLUDED
#define RES_GEOMETRY_H_INCLUDED

#include "res_buffer.h"
#include "res_declaration.h"
#include "res_geometry.h"

namespace xray {
namespace render_dx10 {

class res_geometry : public res_flagged,
	public boost::noncopyable
{
	friend class res_base;
	friend class resource_manager;

	res_geometry( res_buffer * vb, res_buffer * ib, res_declaration * dcl, u32 stride):
		m_vb(vb), m_ib(ib), m_dcl(dcl), m_vb_stride(stride)
	{}
	
	~res_geometry();
	void _free() const;

public:
	
	bool equal(const res_geometry& other) const;
	
	void apply();
	
	u32	get_stride() {return m_vb_stride;}

private:
	ref_buffer				m_vb;			//vb;
	ref_buffer				m_ib;			//ib;
	ref_declaration			m_dcl;			//dcl;
	u32						m_vb_stride;	//vb_stride;
};
typedef	intrusive_ptr<res_geometry, res_base, threading::single_threading_policy>	ref_geometry;

} // namespace render
} // namespace xray

#endif // #ifndef RES_GEOMETRY_H_INCLUDED