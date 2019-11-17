////////////////////////////////////////////////////////////////////////////
//	Created		: 11.06.2010
//	Author		: Nikolay Partas
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "gpu_timer.h"
#include "benchmark.h"

namespace xray {
namespace graphics_benchmark {

gpu_timer::gpu_timer(/*benchmark& bmrk*/)
{
	ID3DDevice* d3d_device = xray::render_dx10::device::ref().d3d_device();//bmrk.d3d_device();
	
	D3D11_QUERY_DESC desc;
	desc.MiscFlags = 0;

	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	
	if ( FAILED (d3d_device->CreateQuery( &desc, &m_freq_info)))
	{
		printf("gpu_timer: initialization failed!");
		return;
	}

	desc.Query = D3D11_QUERY_TIMESTAMP;

	if ( FAILED (d3d_device->CreateQuery( &desc, &m_start_time)) 
	||	 FAILED (d3d_device->CreateQuery( &desc, &m_stop_time))
	)
	{
		printf("gpu_timer: initialization failed!");
		return;
	}
}

gpu_timer::~gpu_timer()
{
	if (m_start_time) m_start_time->Release();
	if (m_stop_time) m_stop_time->Release();
	if (m_freq_info) m_freq_info->Release();
}

void gpu_timer::start()
{
	xray::render_dx10::device::ref().d3d_context()->Begin(m_freq_info);
	xray::render_dx10::device::ref().d3d_context()->End(m_start_time);
	//m_freq_info->Begin();
	//m_start_time->End();
}

void gpu_timer::stop()
{
	//m_stop_time->End();
	//m_freq_info->End();
	xray::render_dx10::device::ref().d3d_context()->End(m_stop_time);
	xray::render_dx10::device::ref().d3d_context()->End(m_freq_info);
}

void gpu_timer::get_elapsed_info(double& out_sec, u64& out_ticks) const
{
	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timer_info;

	u64 start_time	= 0,
		stop_time	= 0;
	
	// waiting for GPU
	while ( xray::render_dx10::device::ref().d3d_context()->GetData(m_freq_info, &timer_info, sizeof( timer_info ), 0) == S_FALSE);
	while ( xray::render_dx10::device::ref().d3d_context()->GetData(m_start_time, &start_time, sizeof( start_time ), 0) == S_FALSE);
	while ( xray::render_dx10::device::ref().d3d_context()->GetData(m_stop_time, &stop_time, sizeof( stop_time ), 0) == S_FALSE);


	out_ticks = stop_time - start_time;
	out_sec =  (double(out_ticks)) / double(timer_info.Frequency);
}

u64 gpu_timer::get_elapsed_ticks() const
{
	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timer_info;

	u64 start_time	= 0,
		stop_time	= 0;

	// waiting for GPU
	while ( xray::render_dx10::device::ref().d3d_context()->GetData(m_freq_info, &timer_info, sizeof( timer_info ), 0) == S_FALSE);
	while ( xray::render_dx10::device::ref().d3d_context()->GetData(m_start_time, &start_time, sizeof( start_time ), 0) == S_FALSE);
	while ( xray::render_dx10::device::ref().d3d_context()->GetData(m_stop_time, &stop_time, sizeof( stop_time ), 0) == S_FALSE);


	if ( timer_info.Disjoint)
		return 0;

	return stop_time - start_time;
}

double gpu_timer::get_elapsed_time() const
{
	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timer_info;

	u64 start_time	= 0,
		stop_time	= 0;

	// waiting for GPU
	while ( xray::render_dx10::device::ref().d3d_context()->GetData(m_freq_info, &timer_info, sizeof( timer_info ), 0) == S_FALSE);
	while ( xray::render_dx10::device::ref().d3d_context()->GetData(m_start_time, &start_time, sizeof( start_time ), 0) == S_FALSE);
	while ( xray::render_dx10::device::ref().d3d_context()->GetData(m_stop_time, &stop_time, sizeof( stop_time ), 0) == S_FALSE);


	if ( timer_info.Disjoint)
		return 0.0f;

	//printf("\ntimer_info.Frequency:%u",timer_info.Frequency);
	return (double(stop_time - start_time)) / double(timer_info.Frequency);
}


} // namespace graphics_benchmark
} // namespace xray
