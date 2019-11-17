////////////////////////////////////////////////////////////////////////////
//	Created		: 09.02.2009
//	Author		: Igor Lobanchikov
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/core/device.h>
#include <xray/render/core/backend.h>
#include <xray/render/base/engine_wrapper.h>
#include <boost/bind.hpp>

namespace xray {
namespace render_dx10 {

inline void	fill_vid_mode_list			( const device* const) {;}
inline void	free_vid_mode_list			() {;}

inline void	fill_render_mode_list		() {;}
inline void	free_render_mode_list		() {;}


device::device		( xray::render::engine::wrapper * wrapper, HWND hwnd, bool windowed):
// --Porting to DX10_
//	m_d3d			( 0),
m_device			( 0),
m_context			( 0),
m_base_rt			( 0),
m_base_zb			( 0),
m_wrapper			( wrapper),
m_hwnd				( hwnd),
m_windowed			( windowed),
m_frame_started		( false), 
m_frame				( 0),
m_present_sync_mode	( D3DPRESENT_INTERVAL_DEFAULT)
{
	//	Just check if initialization order is correct
	ASSERT( device::ptr());

	if( m_wrapper)
	// This form DX9 code. Needed to recheck if we need to do this for DX10 DX11.
		m_wrapper->run_in_window_thread( boost::bind( &device::create, this));
	else
		create();

	//	need device to be already created.
	m_caps.update();
}

device::~device()
{
	if( m_wrapper)
		m_wrapper->run_in_window_thread( boost::bind( &device::destroy, this));
	else
		destroy();
}

void device::create_d3d()
{
	IDXGIFactory * factory;
	HRESULT res = CreateDXGIFactory( __uuidof( IDXGIFactory), ( void**)( &factory));
	R_CHK(res);
	
	m_adapter = 0;
	m_use_perfhud = false;

#ifndef	MASTER_GOLD
	// Look for 'NVIDIA NVPerfHUD' adapter
	// If it is present, override default settings
	UINT i = 0;
	while( factory->EnumAdapters( i, &m_adapter) != DXGI_ERROR_NOT_FOUND)
	{
		m_adapter->GetDesc( &m_adapter_desc);
		if( !wcscmp( m_adapter_desc.Description,L"NVIDIA PerfHUD"))
		{
			m_use_perfhud = true;
			break;
		}
		else
		{
			m_adapter->Release();
			m_adapter = 0;
		}
		++i;
	}
#endif	//	MASTER_GOLD

	if ( !m_adapter)
		factory->EnumAdapters( 0, &m_adapter);

	factory->Release();
}

void device::destroy_d3d()
{
	log_ref_count	( "m_Adapter", m_adapter);
	safe_release	( m_adapter);
}

void device::update_targets()
{
	HRESULT res;

	// Create a render target view
	ID3DTexture2D *buffer;
	res =  m_swap_chain->GetBuffer( 0, __uuidof( ID3DTexture2D), ( void**)&buffer);
	R_CHK(res);

	res = m_device->CreateRenderTargetView( buffer, NULL, &m_base_rt);
	buffer->Release();
	R_CHK(res);

	//	Create Depth/stencil buffer
	ID3DTexture2D*	depth_stencil	= NULL;

	//	HACK: DX10: hard depth buffer format
	D3D_TEXTURE2D_DESC desc_depth;
	desc_depth.Width				= m_chain_desc.BufferDesc.Width;
	desc_depth.Height				= m_chain_desc.BufferDesc.Height;
	desc_depth.MipLevels			= 1;
	desc_depth.ArraySize			= 1;
	desc_depth.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;// m_caps.depth_fmt
	desc_depth.SampleDesc.Count		= 1;
	desc_depth.SampleDesc.Quality	= 0;
	desc_depth.Usage				= D3D_USAGE_DEFAULT;
	desc_depth.BindFlags			= D3D_BIND_DEPTH_STENCIL;
	desc_depth.CPUAccessFlags		= 0;
	desc_depth.MiscFlags			= 0;

	res = m_device->CreateTexture2D( &desc_depth,		// Texture desc
									NULL,				// Initial data
									&depth_stencil);	// [out] Texture
	R_CHK( res);

	//	Create Depth/stencil view
	res = m_device->CreateDepthStencilView( depth_stencil, NULL, &m_base_zb);
	R_CHK( res);

	depth_stencil->Release();
}

void device::create()
{
	create( m_hwnd, false);
}

void device::create( HWND hwnd, bool move_window)
{
	// temporary !

	m_move_window			= move_window;
	create_d3d();

	// TODO: DX10: Create appropriate initialization
	// General - select adapter and device

	// Display the name of video board
	DXGI_ADAPTER_DESC Desc;
	R_CHK( m_adapter->GetDesc( &Desc));

	//	Warning: Desc.Description is wide string
	LOG_INFO ( "* GPU [vendor:%X]-[device:%X]: %S", Desc.VendorId, Desc.DeviceId, Desc.Description);

	/*
	// Display the name of video board
	D3DADAPTER_IDENTIFIER9	adapterID;
	R_CHK	( pD3D->GetAdapterIdentifier( DevAdapter,0,&adapterID));
	Msg		( "* GPU [vendor:%X]-[device:%X]: %s",adapterID.VendorId,adapterID.DeviceId,adapterID.Description);

	u16	drv_Product		= HIWORD( adapterID.DriverVersion.HighPart);
	u16	drv_Version		= LOWORD( adapterID.DriverVersion.HighPart);
	u16	drv_SubVersion	= HIWORD( adapterID.DriverVersion.LowPart);
	u16	drv_Build		= LOWORD( adapterID.DriverVersion.LowPart);
	Msg		( "* GPU driver: %d.%d.%d.%d",u32( drv_Product),u32( drv_Version),u32( drv_SubVersion), u32( drv_Build));
	*/

	/*
	m_caps.id_vendor	= adapterID.VendorId;
	m_caps.id_device	= adapterID.DeviceId;
	*/

	m_caps.id_vendor	= Desc.VendorId;
	m_caps.id_device	= Desc.DeviceId;

	/*
	// Retreive windowed mode
	D3DDISPLAYMODE mWindowed;
	R_CHK( pD3D->GetAdapterDisplayMode( DevAdapter, &mWindowed));

	*/

	// Need to specify depth format depend on render options.
/////////////////////////////////////////////////////////////////////////////	fDepth = D3DFMT_D24S8;

	// Set up the presentation parameters
	ZeroMemory			( &m_chain_desc, sizeof( m_chain_desc));
	select_resolution	( m_chain_desc.BufferDesc.Width, m_chain_desc.BufferDesc.Height, m_windowed, hwnd);

	//	TODO: DX10: implement dynamic format selection
	//m_chain_desc.BufferDesc.Format		= fTarget;
	m_chain_desc.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;
	m_chain_desc.BufferCount		= 1;
	m_chain_desc.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;

	if( m_windowed)
	{
		m_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
		m_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	}
	else
	{
		m_chain_desc.BufferDesc.RefreshRate = select_refresh( m_chain_desc.BufferDesc.Width, m_chain_desc.BufferDesc.Height, m_chain_desc.BufferDesc.Format);
	}

	// Multisample
	m_chain_desc.SampleDesc.Count = 1;
	m_chain_desc.SampleDesc.Quality = 0;

	// Windoze
	//P.SwapEffect			= bWindowed?D3DSWAPEFFECT_COPY:D3DSWAPEFFECT_DISCARD;
	//P.hDeviceWindow			= m_hWnd;
	//P.Windowed				= bWindowed;
	m_chain_desc.SwapEffect		= DXGI_SWAP_EFFECT_DISCARD;
	m_chain_desc.OutputWindow	= hwnd;
	m_chain_desc.Windowed		= m_windowed;

	// This may be usefull !
	m_chain_desc.Flags			= 0;

	UINT createDeviceFlags = 0;
#ifdef DEBUG
	//createDeviceFlags |= D3D_CREATE_DEVICE_DEBUG;
#endif

	HRESULT R;
	// Create the device
	//	DX10 don't need it?
	//u32 GPU		= selectGPU();
#if USE_DX10

	D3D10_DRIVER_TYPE driver_type = D3D10_DRIVER_TYPE_HARDWARE;
	// driver_type = m_caps.bForceGPU_REF ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_HARDWARE;

	if ( m_use_perfhud)
		driver_type =  D3D10_DRIVER_TYPE_REFERENCE;

	R =  D3DX10CreateDeviceAndSwapChain
		(   m_adapter,
		driver_type,
		NULL,
		createDeviceFlags,
		&m_chain_desc,
		&m_swap_chain,
		&m_device);
	
	m_context = m_device;

	m_feature_level = D3D_FEATURE_LEVEL_10_0;
	if( !FAILED( R))
	{
		D3DX10GetFeatureLevel1( m_device, &m_device1);
		if( m_device1)
			m_feature_level = D3D_FEATURE_LEVEL_10_1;
	}
#else

	D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_HARDWARE;
	// driver_type = m_caps.bForceGPU_REF ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_HARDWARE;

	if ( m_use_perfhud)
		driver_type =  D3D_DRIVER_TYPE_REFERENCE;


	D3D_FEATURE_LEVEL feature_levels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	R =  D3D11CreateDeviceAndSwapChain( 0,//m_adapter,//What wrong with adapter??? We should use another version of DXGI?????
		driver_type,
		NULL,
		createDeviceFlags,
		feature_levels,
		sizeof( feature_levels)/sizeof( feature_levels[0]),
		D3D11_SDK_VERSION,
		&m_chain_desc,
		&m_swap_chain,
		&m_device,
		&m_feature_level,		
		&m_context);
#endif

	if ( FAILED( R))
	{
		// Fatal error! Cannot create rendering device AT STARTUP !!!
		LOG_INFO	( 	"Failed to initialize graphics hardware.\n"
						"Please try to restart the game.\n"
						"CreateDevice returned 0x%08x", R);

		//MessageBox			( NULL,"Failed to initialize graphics hardware.\nPlease try to restart the game.","Error!",MB_OK|MB_ICONERROR);
		TerminateProcess	( GetCurrentProcess(),0);
	};
	R_CHK( R);

	log_ref_count	( "* CREATE: DeviceREF:",m_device);

	/*
	switch ( GPU)
	{
	case D3DCREATE_SOFTWARE_VERTEXPROCESSING:
	Log	( "* Vertex Processor: SOFTWARE");
	break;
	case D3DCREATE_MIXED_VERTEXPROCESSING:
	Log	( "* Vertex Processor: MIXED");
	break;
	case D3DCREATE_HARDWARE_VERTEXPROCESSING:
	Log	( "* Vertex Processor: HARDWARE");
	break;
	case D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE:
	Log	( "* Vertex Processor: PURE HARDWARE");
	break;
	}
	*/


	//	Create render target and depth-stencil views here
	update_targets();

	size_t	memory		= Desc.DedicatedVideoMemory;
	LOG_INFO			( "*     Texture memory: %d M",		memory/( 1024*1024));
#ifndef _EDITOR
	update_window_props							( hwnd);
	fill_vid_mode_list							( this);
#endif
/*
	m_present_sync_mode = select_presentation_interval();

	o.hw_smap = support( D3DFMT_D24X8, D3DRTYPE_TEXTURE, D3DUSAGE_DEPTHSTENCIL);
	//	ASSERT( o.hw_smap);

	o.hw_smap_format = D3DFMT_D24X8;
	o.smap_size = 2048;

	D3DFORMAT null_rt_fmt = ( D3DFORMAT)MAKEFOURCC( 'N','U','L','L');
	o.null_rt_format = support( null_rt_fmt, D3DRTYPE_SURFACE, D3DUSAGE_RENDERTARGET) ? null_rt_fmt : D3DFMT_R5G6B5;

	//o.fp16_filter = support( D3DFMT_A16B16G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_FILTER);
	o.fp16_blend  = support( D3DFMT_A16B16G16R16F, D3DRTYPE_TEXTURE, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING);
	ASSERT( o.fp16_blend);

*/
}

void device::destroy()
{
	// moved to backend destructor
	//backend::ref().on_device_destroy();

	log_ref_count( "m_base_zb",m_base_zb);
	safe_release( m_base_zb);

	log_ref_count( "m_base_rt",m_base_rt);
	safe_release( m_base_rt);

	log_ref_count( "* DESTROY: Device", m_device);
	safe_release( m_device);

	free_vid_mode_list();

	destroy_d3d();
}

void device::update_window_props( HWND hw) const
{
	XRAY_UNREFERENCED_PARAMETER	( hw);

	/*
	BOOL	windowed				= TRUE;
	#ifndef _EDITOR
	if ( !g_dedicated_server)
	windowed			= !psDeviceFlags.is( rsFullscreen);
	#endif	

	u32		dwWindowStyle			= 0;
	// Set window properties depending on what mode were in.
	if ( windowed)		{
	if ( m_move_window) {
	if ( strstr( Core.Params,"-no_dialog_header"))
	SetWindowLong	( m_hWnd, GWL_STYLE, dwWindowStyle=( WS_BORDER|WS_VISIBLE));
	else
	SetWindowLong	( m_hWnd, GWL_STYLE, dwWindowStyle=( WS_BORDER|WS_DLGFRAME|WS_VISIBLE|WS_SYSMENU|WS_MINIMIZEBOX));
	// When moving from fullscreen to windowed mode, it is important to
	// adjust the window size after recreating the device rather than
	// beforehand to ensure that you get the window size you want.  For
	// example, when switching from 640x480 fullscreen to windowed with
	// a 1000x600 window on a 1024x768 desktop, it is impossible to set
	// the window size to 1000x600 until after the display mode has
	// changed to 1024x768, because windows cannot be larger than the
	// desktop.

	RECT			m_rcWindowBounds;
	BOOL			bCenter = FALSE;
	if ( strstr( Core.Params, "-center_screen"))	bCenter = TRUE;

	#ifndef _EDITOR
	if ( g_dedicated_server)
	bCenter		= TRUE;
	#endif

	if( bCenter){
	RECT				DesktopRect;

	GetClientRect		( GetDesktopWindow(), &DesktopRect);

	SetRect( 			&m_rcWindowBounds, 
	( DesktopRect.right-m_dev_pparams.BackBufferWidth)/2, 
	( DesktopRect.bottom-m_dev_pparams.BackBufferHeight)/2, 
	( DesktopRect.right+m_dev_pparams.BackBufferWidth)/2, 
	( DesktopRect.bottom+m_dev_pparams.BackBufferHeight)/2			);
	}else{
	SetRect( 			&m_rcWindowBounds,
	0, 
	0, 
	m_dev_pparams.BackBufferWidth, 
	m_dev_pparams.BackBufferHeight);
	};

	AdjustWindowRect		( 	&m_rcWindowBounds, dwWindowStyle, FALSE);

	SetWindowPos			( 	m_hWnd, 
	HWND_TOP,	
	m_rcWindowBounds.left, 
	m_rcWindowBounds.top,
	( m_rcWindowBounds.right - m_rcWindowBounds.left),
	( m_rcWindowBounds.bottom - m_rcWindowBounds.top),
	SWP_SHOWWINDOW|SWP_NOCOPYBITS|SWP_DRAWFRAME);
	}
	}
	else
	{
	SetWindowLong			( m_hWnd, GWL_STYLE, dwWindowStyle=( WS_POPUP|WS_VISIBLE));
	}

	#ifndef _EDITOR
	if ( !g_dedicated_server)
	{
	ShowCursor	( FALSE);
	SetForegroundWindow( m_hWnd);
	}
	#endif
	*/
}


// Not sure if we need this in DX10 ========
//
// u32 device::select_gpu() const
// {
// 	if ( m_caps.force_gpu_sw) return D3DCREATE_SOFTWARE_VERTEXPROCESSING;
// 
// 	D3DCAPS9	caps;
// 	m_d3d->GetDeviceCaps( m_dev_adapter, m_dev_type, &caps);
// 
// 	if( caps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT)
// 	{
// 		if ( m_caps.force_gpu_non_pure)	
// 			return D3DCREATE_HARDWARE_VERTEXPROCESSING;
// 		else 
// 		{
// 			if ( caps.DevCaps&D3DDEVCAPS_PUREDEVICE) 
// 				return D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE;
// 			else 
// 				return D3DCREATE_HARDWARE_VERTEXPROCESSING;
// 		}
// 	} else return D3DCREATE_SOFTWARE_VERTEXPROCESSING;
// }

DXGI_RATIONAL device::select_refresh( u32 width, u32 height, DXGI_FORMAT fmt) const
{
	XRAY_UNREFERENCED_PARAMETERS	( width, height, fmt);

	DXGI_RATIONAL res;
	res.Numerator	= 60;
	res.Denominator = 1;

	return res;
	//	TODO: OPTIONS
	/*
	if ( psDeviceFlags.is( rsRefresh60hz))	return D3DPRESENT_RATE_DEFAULT;
	else
	{
	u32 selected	= D3DPRESENT_RATE_DEFAULT;
	u32 count		= pD3D->GetAdapterModeCount( m_dev_adapter,fmt);
	for ( u32 I=0; I<count; I++)
	{
	D3DDISPLAYMODE	Mode;
	pD3D->EnumAdapterModes( m_dev_adapter,fmt,I,&Mode);
	if ( Mode.Width==dwWidth && Mode.Height==dwHeight)
	{
	if ( Mode.RefreshRate>selected) selected = Mode.RefreshRate;
	}
	}
	return selected;
	}
	*/
}

bool device::support( DXGI_FORMAT fmt, DWORD usage)
{
	//	TODO:DX10: implement stub for this code.
	//	ASSERT( !"Implement CHW::support");
	u32 res;
	m_device->CheckFormatSupport( fmt, &res);

	return res == ( res&usage);
}

void device::select_resolution( u32 & width, u32 & height, bool windowed, HWND window) const
{
	fill_vid_mode_list( this);

	if( windowed)
	{
		//	TODO: OPTIONS
		//width		= psCurrentVidMode[0];
		//height	= psCurrentVidMode[1];
		RECT rect;
		GetClientRect( window, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	else
	{
		width	= 1024;
		height	= 768;
		//ASSERT( !"device::select_resolution: Not implemented for fullscreen.");
		//	TODO: OPTIONS
		//string64					buff;
		//sprintf_s					( buff,sizeof( buff),"%dx%d",psCurrentVidMode[0],psCurrentVidMode[1]);

		//if( _ParseItem( buff,vid_mode_token)==u32( -1)) //not found
		//{ //select safe
		//	sprintf_s				( buff,sizeof( buff),"vid_mode %s",vid_mode_token[0].name);
		//	Console->Execute		( buff);
		//}

		//dwWidth						= psCurrentVidMode[0];
		//dwHeight					= psCurrentVidMode[1];
	}
}


void device::reset( /*bool windowed*/)
{
	m_chain_desc.Windowed	= m_windowed;

	R_CHK( m_swap_chain->SetFullscreenState( !m_windowed, NULL));

	DXGI_MODE_DESC	&desc = m_chain_desc.BufferDesc;

	select_resolution(desc.Width, desc.Height, m_windowed, m_hwnd);

	if( m_windowed)
	{
		desc.RefreshRate.Numerator = 60;
		desc.RefreshRate.Denominator = 1;
	}
	else
		desc.RefreshRate = select_refresh( desc.Width, desc.Height, desc.Format);

	R_CHK( m_swap_chain->ResizeTarget(&desc));


#ifdef DEBUG
	//	_RELEASE			(dwDebugSB);
#endif
	log_ref_count		("refCount:pBaseZB",m_base_zb);
	log_ref_count		("refCount:pBaseRT",m_base_rt);

	safe_release(m_base_zb);
	safe_release(m_base_rt);

	R_CHK(	m_swap_chain->ResizeBuffers(
			m_chain_desc.BufferCount,
			desc.Width,
			desc.Height,
			desc.Format,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	update_targets();

	update_window_props	( m_hwnd);

	m_present_sync_mode = select_presentation_interval();

	// --Porting to DX10_
	//setup_states();
}


bool device::begin_frame()
{
	if ( m_device_state == ds_need_reset)
		reset();

	//	TODO: SYSTEM
	//RCache.OnFrameBegin		();
	//if ( HW.m_caps.SceneMode)	overdrawBegin	();

	m_frame_started = true;

	return true;
}

void device::end_frame()
{
	if ( !m_frame_started) return;

	m_frame_started = false;

	m_swap_chain->Present( m_present_sync_mode, 0 );
	++m_frame;


#ifndef DEDICATED_SERVER

//	if ( dwPrecacheFrame)
//	{
//		::Sound->set_master_volume	( 0.f);
//		dwPrecacheFrame	--;
//		pApp->load_draw_internal	();
//		if ( 0==dwPrecacheFrame)
//		{
//			m_pRender->updateGamma();
//
//			if( precache_light) precache_light->set_active	( false);
//			if( precache_light) precache_light.destroy		();
//			::Sound->set_master_volume						( 1.f);
//			pApp->destroy_loading_shaders					();
//
//			m_pRender->ResourcesDestroyNecessaryTextures	();
//			Memory.mem_compact								();
//			Msg												( "* MEMORY USAGE: %d K",Memory.mem_usage()/1024);
//			Msg												( "* End of synchronization A[%d] R[%d]",b_is_Active, b_is_Ready);
//
//#ifdef FIND_CHUNK_BENCHMARK_ENABLE
//			g_find_chunk_counter.flush();
//#endif // FIND_CHUNK_BENCHMARK_ENABLE
//
//			CheckPrivilegySlowdown							();
//
//			if( g_pGamePersistent->GameType()==1)//haCk
//			{
//				WINDOWINFO	wi;
//				GetWindowInfo( m_hWnd,&wi);
//				if( wi.dwWindowStatus!=WS_ACTIVECAPTION)
//					Pause( TRUE,TRUE,TRUE,"application start");
//			}
//		}
//	}

	// end scene
	//	Present goes here, so call OA Frame end.
	//if ( g_SASH.IsBenchmarkRunning())
	//	g_SASH.DisplayFrame( Device.fTimeGlobal);


	//if ( HW.m_caps.SceneMode)	overdrawEnd();

	//RCache.OnFrameEnd	();
	//Memory.dbg_check		();

#endif

}

void device::setup_states() const
{
	// General Render States
	//mView.identity			();
	//mProject.identity		();
	//mFullTransform.identity	();
	//vCameraPosition.set		( 0,0,0);
	//vCameraDirection.set	( 0,0,1);
	//vCameraTop.set			( 0,1,0);
	//vCameraRight.set		( 1,0,0);

	device::ref().update_caps();

	// --Porting to DX10_
	// setup_states_platform();
}

void device::test_corrporate_level()
{
	m_device_state = ds_ok;
}

u32	device::select_presentation_interval	()
{
// Todo: consider render options to turn on vsync
// 	if ( psDeviceFlags.test( rsVSync)) 
// 	{
// 		// 		if ( caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE)
// 		// 			return D3DPRESENT_INTERVAL_IMMEDIATE;
// 		return D3DPRESENT_INTERVAL_ONE;
// 	}
	return D3DPRESENT_INTERVAL_DEFAULT;
}

// This may be removed for DX10 
device::device_state device::get_device_state() const
{
// 	HRESULT	_hr		= pdevice->TestCooperativeLevel();
// 	if (FAILED(_hr))
// 	{
// 		// If the device was lost, do not render until we get it back
// 		if		(D3DERR_DEVICELOST==_hr)
// 			return ds_lost;
// 
// 		// Check if the device is ready to be reset
// 		if		(D3DERR_DEVICENOTRESET==_hr)
// 			return ds_need_reset;
// 	}
// 
 	return ds_ok;
}

ID3DRenderTargetView * device::get_base_render_target()
{
	return m_base_rt;
}

ID3DDepthStencilView * device::get_depth_stencil_target()
{
	return m_base_zb;
}

u32	device::video_memory_size		()
{
	return m_adapter_desc.DedicatedVideoMemory;
}



} // namespace render 
} // namespace xray 
