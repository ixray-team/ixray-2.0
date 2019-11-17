////////////////////////////////////////////////////////////////////////////
//	Created		: 06.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RES_XS_HW_H_INCLUDED
#define RES_XS_HW_H_INCLUDED

#include "sampler_slot.h"
#include "res_texture.h"

namespace xray {
namespace render_dx10 {

template <typename shader_data>
class res_xs_hw:
		public res_named,
		public res_id,
		public boost::noncopyable
{
	friend class resource_manager;
	friend class res_base;

	res_xs_hw		();
	~res_xs_hw		();
	void _free		() const;


public:

	HRESULT create_hw_shader( ID3DBlob* shader_code);

	typename shader_data::hw_interface* hw_shader() { return m_shader_data.hw_shader;}

	shader_data const & data() const { return m_shader_data; }

// 	res_constant_table const &	constants	() const	{ return m_constants;}
// 	sampler_slots const &		samplers	() const	{ return m_samplers; }
// 	texture_slots const &		textures	() const	{ return m_textures; }

private:
	HRESULT create_hw_shader( ID3DBlob* shader_code, typename shader_data::hw_interface **hw_shader);
	void	parse_resources	( ID3DShaderReflection* shader_reflection, 
								sampler_slot	(&samplers)[D3D_COMMONSHADER_SAMPLER_SLOT_COUNT], 
								texture_slot	(&textures)[D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] );
 
private:
	shader_data				m_shader_data;

}; // class res_xs_hw

} // namespace render
} // namespace xray

#endif // #ifndef RES_XS_HW_H_INCLUDED