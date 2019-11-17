////////////////////////////////////////////////////////////////////////////
//	Created		: 15.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef BACKEND_LAYERS_H_INCLUDED
#define BACKEND_LAYERS_H_INCLUDED

#include "res_constant_table.h"
#include "res_texture_list.h"
#include "res_sampler_list.h"

namespace xray {
namespace render_dx10 {

//////////////////////////////////////////////////////////////////////////
// constants_handler
template <enum_shader_type shader_type>
class constants_handler
{
	constants_handler() : m_diff_range_start (0), m_diff_range_end (0) {}
	friend class backend;

// 	constant const * get( LPCSTR const name)	{ return m_current->get(name);}		// slow search ???
// 	constant const * get( shared_string& name)	{ return m_current->get(name);}		// fast search ???

	template <typename T>
	void set_constant( constant_host const & c, T const & arg);

	void			assign			( res_constant_table const * ctable);
	void			gather_data		();
	void			update_buffers	();

	void			apply			();
	
	struct  change_ragne
	{
		change_ragne(): start(0), end(0){}

		u32 start;
		u32 end;
	};

	void  fill_changes_buffer( ID3DConstantBuffer** buffer);
	u32							m_diff_range_start;
	u32							m_diff_range_end;
	ref_constant_table_const	m_current;

#ifdef DEBUG
	render::vector<constant_host*>		m_unset_constants;
	void							check_for_unset_constants();
#else
	void							check_for_unset_constants(){}
#endif

};

template <enum_shader_type shader_type>
class textures_handler
{
	friend class backend;

	textures_handler() : m_diff_range_start (0), m_diff_range_end (0) {}

	void			assign		( res_texture_list const * ctable);

	void			apply		();

	void			fill_changes_buffer( ID3DShaderResourceView** buffer);

	ref_texture_list_const	m_current;

	u32						m_diff_range_start;
	u32						m_diff_range_end;

#ifdef DEBUG
	void					check_for_unset_textures( texture_slots const & slots);
#else
	void					check_for_unset_textures( texture_slots const & ){}
#endif

};

template <enum_shader_type shader_type>
class samplers_handler
{
	friend class backend;

	samplers_handler() : m_diff_range_start (0), m_diff_range_end (0) {}

	void			assign		( res_sampler_list const* ctable);

	void			apply		();

	void			fill_changes_buffer( ID3DSamplerState ** buffer);

	u32						m_diff_range_start;
	u32						m_diff_range_end;
	ref_sampler_list_const	m_current;

#ifdef DEBUG
	void					check_for_unset_samplers( sampler_slots const & slots);
#else
	void					check_for_unset_samplers( sampler_slots const & ){}
#endif

};


// template <typename resource, hw_resource>
// class resource_handler
// {
// 	friend class backend;
// 
// 	void			assign		( resource& ctable);
// 	void			apply		();
// 
// 	void			fill_changes_buffer( hw_resource** buffer);
// 
// 	u32				m_diff_range_start;
// 	u32				m_diff_range_end;
// 	ref_constant_table	m_current;
// };


} // namespace render
} // namespace xray

#include "backend_handlers_inline.h"

#endif // #ifndef BACKEND_LAYERS_H_INCLUDED