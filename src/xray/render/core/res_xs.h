////////////////////////////////////////////////////////////////////////////
//	Created		: 09.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RES_XS_H_INCLUDED
#define RES_XS_H_INCLUDED

#include "xs_descriptor.h"
#include "xs_data.h"
#include "res_sampler_list.h"
#include "res_texture_list.h"

namespace xray {
namespace render_dx10 {

class res_texture;

template <typename shader_data>
class res_xs:
	public res_named,
	public res_id,
	public boost::noncopyable

{
	typedef xray::intrusive_ptr<res_xs_hw<shader_data>, res_base, threading::single_threading_policy>	ref_xs_hw;

	friend class res_base;
	friend class resource_manager;
	res_xs		( xs_descriptor<shader_data> const & binder);
	~res_xs		();
	void _free	() const;

public:

	bool equal	( xs_descriptor<shader_data> const & binder) const;
	void apply	() const;

	res_xs_hw<shader_data> const *	hw_shader	()	{ return &*m_hw_shader; }

private:
	ref_xs_hw			m_hw_shader;
	ref_constant_table	m_constants;
	ref_texture_list	m_textures;
	ref_sampler_list	m_samplers;
};


template <>
class res_xs<vs_data> : 
	public res_named,
	public res_id,
	public boost::noncopyable

{
	typedef xray::intrusive_ptr<res_xs_hw<vs_data>, res_base, threading::single_threading_policy>	ref_xs_hw;

	friend class res_base;
	friend class resource_manager;
	res_xs		( xs_descriptor<vs_data> const & binder);
	~res_xs		();
	void _free	() const;

public:
	bool equal	( xs_descriptor<vs_data> const & binder) const;
	void apply	() const;

	res_xs_hw<vs_data> const *	hw_shader	()	{ return &*m_hw_shader; }

private:
	ref_xs_hw			m_hw_shader;
	ref_constant_table	m_constants;
	ref_texture_list	m_textures;
	ref_sampler_list	m_samplers;
};


typedef  res_xs<vs_data>		res_vs;
typedef  res_xs<gs_data>		res_gs;
typedef  res_xs<ps_data>		res_ps;


typedef xray::intrusive_ptr<res_vs, res_base, threading::single_threading_policy>	ref_vs;
typedef xray::intrusive_ptr<res_gs, res_base, threading::single_threading_policy>	ref_gs;
typedef xray::intrusive_ptr<res_ps, res_base, threading::single_threading_policy>	ref_ps;


} // namespace render
} // namespace xray

#endif // #ifndef RES_XS_H_INCLUDED