////////////////////////////////////////////////////////////////////////////
//	Created		: 06.04.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "res_xs_hw.h"
#include "backend.h"
#include <xray/render/core/resource_manager.h>

namespace xray {
namespace render_dx10 {

template<typename shader_data>
res_xs_hw<shader_data>::res_xs_hw()
{

}

template<typename shader_data>
res_xs_hw<shader_data>::~res_xs_hw()
{
	safe_release( m_shader_data.hw_shader);
}

template<typename shader_data>
void res_xs_hw<shader_data>::_free	() const
{
	resource_manager::ref().release( const_cast<res_xs_hw<shader_data>*> (this));
}

template<typename shader_data>
HRESULT res_xs_hw<shader_data>::create_hw_shader( ID3DBlob* shader_code, typename shader_data::hw_interface **hw_shader)
{
	XRAY_UNREFERENCED_PARAMETERS( shader_code, hw_shader);
	NODEFAULT;
	return 0;
}

// template<typename shader_data>
// void res_xs_hw<shader_data>::apply()
// {
// 	NODEFAULT;
// }

template<typename shader_data>
HRESULT res_xs_hw<shader_data>::create_hw_shader( ID3DBlob* shader_code)
{
	ASSERT( shader_code);

	void const* buffer	= shader_code->GetBufferPointer();
	u32 buffer_size		= (u32)shader_code->GetBufferSize();

	HRESULT hr = create_hw_shader( shader_code, &m_shader_data.hw_shader);

	if ( SUCCEEDED( hr))	
	{
		ID3DShaderReflection* shader_reflection;
#if USE_DX10
		hr			= D3D10ReflectShader( buffer, buffer_size, &shader_reflection);
#else
		hr			= D3DReflect( buffer, buffer_size, IID_ID3D11ShaderReflection, (void**)&shader_reflection);
#endif

		m_shader_data.constants.parse	( shader_reflection, (enum_shader_type)shader_data::type);
		parse_resources		( shader_reflection, m_shader_data.samplers, m_shader_data.textures);

		safe_release( shader_reflection);
		//LPD3DXSHADER_CONSTANTTABLE	constants_table	= NULL;
		////LPCVOID data = NULL;
		//hr = D3DXFindShaderComment( ( DWORD*)shader_code->GetBufferPointer(),
		//	MAKEFOURCC( 'C','T','A','B'), ( LPCVOID*)&constants_table, NULL);

		//if ( SUCCEEDED( hr) && constants_table)
		//{
		//	//constants_table = LPD3DXSHADER_CONSTANTTABLE( data);
		//	//m_constants.parse	( pConstants,0x2);
		//} 
		//else
		//{
		//	LOG_ERROR( "! D3DXFindShaderComment hr == %08x", ( int)hr);
		//	hr = E_FAIL;
		//}
	}
	else
	{
		LOG_ERROR( "! CreateVertexShader hr == %08x", ( int)hr);
	}

	return hr;
}

template<typename shader_data>
void res_xs_hw<shader_data>::parse_resources	( ID3DShaderReflection* shader_reflection, 
												 sampler_slot (&samplers) [D3D_COMMONSHADER_SAMPLER_SLOT_COUNT], 
												 texture_slot (&textures) [D3D_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT])
{
// 	ZeroMemory( samplers, sizeof(samplers));
// 	ZeroMemory( textures, sizeof(textures));

	D3D_SHADER_DESC	shader_desc;
	shader_reflection->GetDesc( &shader_desc);

	for( u32 i=0; i<shader_desc.BoundResources; ++i)
	{
		D3D_SHADER_INPUT_BIND_DESC	ResDesc;
		shader_reflection->GetResourceBindingDesc(i, &ResDesc);

		ASSERT(ResDesc.BindCount==1);

		if( ResDesc.Type == D3D10_SIT_TEXTURE)
		{
			ASSERT( textures[ResDesc.BindPoint].name.length() == 0);

			texture_slot tex_slot;
			tex_slot.name		= ResDesc.Name;
			tex_slot.slot_id	= ResDesc.BindPoint; // ??? may be removed

			textures[ResDesc.BindPoint] = tex_slot;

		}
		else if( ResDesc.Type == D3D10_SIT_SAMPLER)
		{
			ASSERT( samplers[ResDesc.BindPoint].name.length() == 0);
			
			sampler_slot smp;
			smp.name		= ResDesc.Name;
			smp.slot_id		= ResDesc.BindPoint; // ??? may be removed
			smp.state		= backend::ref().find_registered_sampler( ResDesc.Name);

			//ASSERT( smp.state, "The sampler_slot hasn't been registared!");

			samplers[ResDesc.BindPoint] = smp;
		}
		else if( ResDesc.Type == D3D11_SIT_UAV_RWTYPED)
		{
			NOT_IMPLEMENTED();
		}
		else
		{
			continue;
		}
	}
	return;
}


} // namespace render
} // namespace xray

