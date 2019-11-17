#include "pch.h"
#include <xray/render/core/device.h>
#include <xray/render/core/res_texture.h>
#include <xray/render/core/resource_manager.h>
//
//#ifndef _EDITOR
//#include "../../xrEngine/render.h"
//#endif
//
//#include "../../xrEngine/tntQAVI.h"
//#include "../../xrEngine/xrTheora_Surface.h"
//
//#include "dxRenderDeviceRender.h"

namespace xray {
namespace render_dx10 {


#define		PRIORITY_HIGH	12
#define		PRIORITY_NORMAL	8
#define		PRIORITY_LOW	4

//void ref_texture::create(LPCSTR name)
//{
//	resource_manager::ref().create_texture(name);
//}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
res_texture::res_texture( bool pool_texture):
m_surface		(NULL),
m_mip_level_cut	(0),
m_desc_valid	(false),
m_sh_res_view	(NULL),
m_pool_texture	(pool_texture)
{
	ZeroMemory( &m_desc, sizeof(m_desc));

	//pAVI				= NULL;
	//pTheora				= NULL;
	//desc_cache			= 0;
	//seqMSPF				= 0;
	//flags.MemoryUsage	= 0;
	//flags.bLoaded		= false;
	//flags.bUser			= false;
	//flags.seqCycles		= FALSE;
	//m_material			= 1.0f;
	//m_bind = fastdelegate::FastDelegate1<u32>(this, &res_texture::apply_load);
}

res_texture::~res_texture()
{
	safe_release(m_surface);
	safe_release(m_sh_res_view);

	//unload				();
	// release external reference
}

void res_texture::_free		() const
{
	resource_manager::ref().release( const_cast<res_texture*>(this) );
}

void res_texture::desc_update()
{
	m_desc_cache_surface	= m_surface;
	if (m_desc_cache_surface)
	{
		D3D_RESOURCE_DIMENSION	type;
		m_desc_cache_surface->GetType(&type);
		if (D3D_RESOURCE_DIMENSION_TEXTURE2D == type)
		{
			ID3DTexture2D*	T	= (ID3DTexture2D*)m_desc_cache_surface;
			T->GetDesc(&m_desc);
			m_desc_valid = true;
		}
	}
}

void res_texture::set_hw_texture(ID3DBaseTexture* surface, u32 mip_level_cut, bool staging)
{
	if (surface)
		surface->AddRef();

	m_mip_level_cut = mip_level_cut;

	safe_release( m_surface);

	safe_release( m_sh_res_view);

	m_surface = surface;
	m_desc_valid = false;

	if (m_surface)
	{
		desc_update();

		ASSERT( mip_level_cut < m_desc.MipLevels || !m_desc_valid);

		D3D_RESOURCE_DIMENSION	type;
		m_surface->GetType(&type);
		
		if (D3D_RESOURCE_DIMENSION_TEXTURE2D == type )
		{
			D3D_SHADER_RESOURCE_VIEW_DESC	view_desc;

			if (m_desc.MiscFlags&D3D_RESOURCE_MISC_TEXTURECUBE)
			{
				view_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
				view_desc.TextureCube.MostDetailedMip = 0;
				view_desc.TextureCube.MipLevels = m_desc.MipLevels;
			}
			else
			{
				if(m_desc.SampleDesc.Count <= 1 )
				{
					if( m_desc.ArraySize > 1)
					{
						view_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
						view_desc.Texture2DArray.FirstArraySlice = 0;
						view_desc.Texture2DArray.ArraySize = m_desc.ArraySize;
						view_desc.Texture2DArray.MostDetailedMip = 0;
						view_desc.Texture2DArray.MipLevels = m_desc.MipLevels;
					}
					else
					{
						view_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
						view_desc.Texture2D.MostDetailedMip = 0;
						view_desc.Texture2D.MipLevels = m_desc.MipLevels;
					}
				}
				else
				{
					ASSERT( m_desc.ArraySize == 1, "Multisampler Array textures support are not implemented.");
					view_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DMS;
					view_desc.Texture2D.MostDetailedMip = 0;
					view_desc.Texture2D.MipLevels = m_desc.MipLevels;
				}
			}			

			view_desc.Format = DXGI_FORMAT_UNKNOWN;

			switch(m_desc.Format)
			{
			case DXGI_FORMAT_R24G8_TYPELESS:
				view_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				break;
			case DXGI_FORMAT_R32_TYPELESS:
				view_desc.Format = DXGI_FORMAT_R32_FLOAT;
				break;
			}

			// this would be supported by DX10.1 but is not needed for stalker
			// if( view_desc.Format != DXGI_FORMAT_R24_UNORM_X8_TYPELESS )
			if( !staging && ((m_desc.SampleDesc.Count <= 1) || (view_desc.Format != DXGI_FORMAT_R24_UNORM_X8_TYPELESS)) )
				R_CHK(device::ref().d3d_device()->CreateShaderResourceView(m_surface, &view_desc, &m_sh_res_view));
			else
				m_sh_res_view = 0;
		}
		else
			R_CHK(device::ref().d3d_device()->CreateShaderResourceView(m_surface, NULL, &m_sh_res_view));
	}	
}

ID3DBaseTexture* res_texture::get_hw_texture()
{
// 	if (m_surface)
// 		m_surface->AddRef();

	return m_surface;
}

} // namespace render 
} // namespace xray 
