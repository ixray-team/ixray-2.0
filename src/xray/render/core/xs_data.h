////////////////////////////////////////////////////////////////////////////
//	Created		: 13.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XS_DATA_H_INCLUDED
#define XS_DATA_H_INCLUDED

#include "res_signature.h"
#include "res_constant_table.h"
#include "res_texture_list.h"
#include "res_sampler_list.h"

namespace xray {
namespace render_dx10 {

//////////////////////////////////////////////////////////////////////////
// vs_data
struct vs_data
{
	vs_data() : hw_shader(NULL)	{}

	typedef   ID3DVertexShader				hw_interface;
	enum { type = enum_shader_type_vertex };

	ID3DVertexShader*		hw_shader;

	res_constant_table		constants;
	sampler_slots			samplers;
	texture_slots			textures;

	ref_signature_const		signature;	
};

//////////////////////////////////////////////////////////////////////////
// gs_data
struct gs_data
{
	gs_data() : hw_shader(NULL)	{}

	typedef   ID3DGeometryShader			hw_interface;
	enum { type = enum_shader_type_geometry };

	ID3DGeometryShader*		hw_shader;

	res_constant_table		constants;
	sampler_slots			samplers;
	texture_slots			textures;
};


//////////////////////////////////////////////////////////////////////////
// ps_data
struct ps_data
{
	ps_data() : hw_shader(NULL)	{}

	typedef   ID3DPixelShader				hw_interface;
	enum { type = enum_shader_type_pixel };

	ID3DPixelShader*		hw_shader;

	res_constant_table		constants;
	sampler_slots			samplers;
	texture_slots			textures;
};


} // namespace render
} // namespace xray

#endif // #ifndef XS_DATA_H_INCLUDED