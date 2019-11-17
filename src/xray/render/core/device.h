////////////////////////////////////////////////////////////////////////////
//	Created		: 09.02.2009
//	Author		: Igor Lobanchikov
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef RENDER_DEVICE_H_INCLUDED
#define RENDER_DEVICE_H_INCLUDED

#include <xray/render/core/render_options.h>
#include <xray/render/core/device_caps.h>
#include <xray/render/core/quasi_singleton.h>



namespace xray {

namespace render{
namespace engine{

	struct wrapper;

} // namespace engine
} // namespace render

namespace render_dx10 {


class device: public quasi_singleton<device>
{
public:
		device( xray::render::engine::wrapper * wrapper, HWND hwnd, bool windowed);
		~device();

		void		create_d3d		();	
		void		destroy_d3d		();	
		void		create			(HWND hwnd, bool move_window);	
		void		create			();	//	hwnd = m_hwnd, move_window = false
		void		destroy			();	
		void		reset			();

		bool		begin_frame		();
		void		end_frame		();
		u32			get_frame		() {return m_frame;}

		// Need to recheck if we really need this in DX10 or DX11.
		void		test_corrporate_level();

		ID3DDevice*			d3d_device	() const	{ return m_device; }
		ID3DDeviceContext*	d3d_context	() const	{ return m_context; }

			//	Return current back-buffer width and height.
			//	Desired with and height in global options can be different!
inline	u32			get_width	() const;
inline	u32			get_height	() const;
inline	xray::math::uint2	get_size	() const;

		ID3DRenderTargetView * get_base_render_target();
		ID3DDepthStencilView * get_depth_stencil_target();

		bool		support		(DXGI_FORMAT fmt, DWORD type);

const	device_caps&	get_caps			() const { return m_caps;}
		u32				video_memory_size	();

public:
	render_options	options;

private:
	enum device_state
	{
		ds_ok = 0,
		ds_lost,
		ds_need_reset
	};

private:
	//	This method will try to use width/height from global options.
	//	If width/height are not supported it will try to find a valid combination.
	//	It will return selected width/height.
	void					select_resolution	( u32 &width, u32 &height, bool windowed, HWND window) const;	//	selectResolution
	u32						select_presentation_interval();
	DXGI_RATIONAL			select_refresh		( u32 width, u32 height, DXGI_FORMAT fmt) const;	//	selectRefresh
	void					update_targets			();

	// Not sure if we need this 
	//u32					select_gpu() const;	//	selectGPU
	void					update_window_props	( HWND hw) const;	//	updateWindowProps
	void					setup_states			() const;
	void					update_caps				()			{ m_caps.update();}
	DXGI_ADAPTER_DESC const	adapter_desc			() const	{ return m_adapter_desc;}
	device_state			get_device_state		() const;
	ID3DRenderTargetView*	get_render_target		() const 	{ return m_base_rt;}
	ID3DDepthStencilView*	get_depth_stencil		() const 	{ return m_base_zb;}

private:
	IDXGIAdapter*			m_adapter;	//	pD3D equivalent
	ID3DDevice*				m_device;	//	pDevice	//	render device
	ID3DDevice1*			m_device1;	//	DX10.1
	IDXGISwapChain*         m_swap_chain;
	ID3DDeviceContext*		m_context;	//	pDevice	//	render device

	// Render targets
	ID3DRenderTargetView*	m_base_rt;	//	base render target view
	ID3DDepthStencilView*	m_base_zb;	//	depth/stencil render target view

	DXGI_SWAP_CHAIN_DESC	m_chain_desc;	
	D3D_FEATURE_LEVEL		m_feature_level;
	DXGI_ADAPTER_DESC		m_adapter_desc;

private:
	xray::render::engine::wrapper *	m_wrapper;
	HWND					m_hwnd;
	bool					m_windowed;

	device_caps				m_caps;

	bool					m_move_window;
	bool					m_frame_started;
	u32						m_frame;
	device_state			m_device_state;
	bool					m_use_perfhud;
	
	u32						m_present_sync_mode;

}; // class device

} // namespace render 
} // namespace xray 

#include "device_inline.h"

#endif // #ifndef RENDER_DEVICE_H_INCLUDED