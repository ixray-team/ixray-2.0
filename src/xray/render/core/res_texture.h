////////////////////////////////////////////////////////////////////////////
//	Created		: 07.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef RES_TEXTURE_DX11_H_INCLUDED
#define RES_TEXTURE_DX11_H_INCLUDED

#include "res_common.h"

namespace xray {
namespace render_dx10 {

class res_texture :
	public res_named,
	public boost::noncopyable
{
	friend class resource_manager;
	friend class res_rt;
	friend class res_base;

	res_texture	( bool pool_texture = false);
	virtual ~res_texture();
	void _free		() const;

	// For friend classes calls only.
	void set_hw_texture( ID3DBaseTexture* surface, u32 mip_level_cut = 0, bool staging = false);

public:

	ID3DBaseTexture *			get_hw_texture	();
	ID3DShaderResourceView *	get_view		()		{ return m_sh_res_view;}
	D3D_TEXTURE2D_DESC const &	get_desc		()		{ return m_desc;}

	u32				width			()		{ desc_enshure(); return m_desc.Width;}
	u32				height			()		{ desc_enshure(); return m_desc.Height;}
	u32				mips_count		()		{ desc_enshure(); return m_desc.MipLevels;}
	u32				array_size		()		{ desc_enshure(); return m_desc.ArraySize;}
	DXGI_FORMAT		format			()		{ desc_enshure(); return m_desc.Format;}
	bool			pool_texture	()		{ return	m_pool_texture;}

private:
//	bool			desc_valid		();   {return m_surface == m_desc_cache_surface;}
	void			desc_enshure	()	{}// {if (!desc_valid()) desc_update();}
	void			desc_update		();

	D3D_USAGE		get_usage		();

private:
	u32				m_loaded	 : 1;	//bLoaded
	u32				m_user		 : 1;	//bUser
	u32				m_seq_cycles : 1;	//seqCycles
	u32				m_mem_usage	 : 28;	//MemoryUsage

#	ifdef	USE_DX10_OLD
	u32					m_is_loaded_as_staging: 1;//bLoadedAsStaging
#	endif	//	USE_DX10

	fastdelegate::FastDelegate1<u32>	m_bind;	

	ID3DBaseTexture*					m_surface;		
	ID3DBaseTexture*					m_desc_cache_surface;	
	ID3DShaderResourceView*				m_sh_res_view;		
	D3D_TEXTURE2D_DESC					m_desc;			
	u32									m_mip_level_cut;
	bool								m_desc_valid;
	bool								m_pool_texture;
};
typedef intrusive_ptr<res_texture, res_base, threading::single_threading_policy> ref_texture;


class texture_slot
{
public:
	texture_slot(): slot_id( enum_slot_ind_null), texture( NULL){}

	name_string		name;
	u32				slot_id;
	ref_texture		texture;
};

} // namespace render 
} // namespace xray 


#endif // #ifndef RES_TEXTURE_DX11_H_INCLUDED

