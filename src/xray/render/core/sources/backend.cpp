////////////////////////////////////////////////////////////////////////////
//	Created		: 16.02.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/core/backend.h>


namespace xray {
namespace render_dx10 {

backend::backend( ID3DDevice* device):
m_device					(NULL),
m_constant_update_counter	(1),
m_rasterizer_state			(NULL),
m_depth_stencils_state		(NULL),
m_effect_state				(NULL),
m_stencil_ref				(0x00),
m_sample_mask				(0xFFFFFFFF),
m_vb						(NULL),
m_ib						(NULL),
m_vs						(NULL),
m_ps						(NULL),
m_gs						(NULL),
m_decl						(NULL),
m_input_layout				(NULL)
{
	for( int i = 0; i < enum_shader_types_count; ++i)
	{
		m_constant_update_markers[i] = 1;
	}

	on_device_create( device);

	reset();
}

backend::~backend()
{
	for( constant_hosts::iterator it = m_constant_hosts.begin(); it< m_constant_hosts.end(); ++it)
		DELETE( *it);

	on_device_destroy();
}

void backend::on_device_create(ID3DDevice* device)
{
	m_device = device;
	m_device->AddRef();

	vertex.create( (512+1024)*1024);
	index.create( 512*1024);

	m_base_rt = device::ref().get_base_render_target();
	m_base_zb = device::ref().get_depth_stencil_target();

#pragma warning(push)
#pragma warning(disable:4244)

	D3D_VIEWPORT VP = { 0, 0, device::ref().get_width(), device::ref().get_height(), 0.f, 1.f };//	{0,0,T->get_width(),T->get_height(),0,1.f };

#pragma warning(pop)

	set_viewport( VP);
}

void backend::on_device_destroy()
{
	safe_release(m_device);
	m_device = 0;

// 	vertex.destroy();
// 	index.destroy();
}

void backend::register_sampler		( char const * name, res_sampler_state * sampler)
{
	ASSERT( find_registered_sampler(name) == NULL, "A sampler with the specified name allready was registered!");
	ASSERT( sampler);
	m_samplers_registry.push_back( samplers_registry_record( name_string(name), sampler) );
}

res_sampler_state * backend::find_registered_sampler	( char const* name)
{
	samplers_registry::const_iterator it	= m_samplers_registry.begin();
	samplers_registry::const_iterator end	= m_samplers_registry.end();
	
	for( ; it != end; ++it)
	{
		if( it->first == name)
			return it->second;
	}

	return NULL;
}

// static inline bool sorted_vector_predicate( constant_host const * first, constant_host const * second)
// {
// 	return first->name < second->name;
// }

static inline bool sorted_vector_predicate( constant_host const * first, shared_string const & second)
{
	return first->name < second;
}

constant_host * backend::register_constant_host	( constant const& constant)
{
	return register_constant_host( constant.name);
}

constant_host * backend::register_constant_host	( shared_string name)
{
	constant_hosts::iterator it = std::lower_bound( m_constant_hosts.begin(), m_constant_hosts.end(), name, &sorted_vector_predicate);

	if ( it == m_constant_hosts.end() || !((*it)->name == name))
		return *m_constant_hosts.insert( it, NEW (constant_host)(name));
	else
		return *it;
}

constant_host const * backend::find_constant_host		( shared_string name, bool create_if_not)
{
	constant_hosts::iterator it = std::lower_bound( m_constant_hosts.begin(), m_constant_hosts.end(), name, sorted_vector_predicate);

	if ( it == m_constant_hosts.end() || !(**it == name) )
	{
		if( !create_if_not)
			return  NULL;
		else
			return register_constant_host( name);
	}
	else
		return *it;
}

void backend::reset_constant_update_markers	()
{
	LOG_INFO("reset_constant_update_markers called !!!");
	constant_hosts::iterator		it =	m_constant_hosts.begin();
	constant_hosts::const_iterator	end =	m_constant_hosts.end();

	for( ; it!= end; ++it)
		for( int i = 0; i < sizeof((*it)->update_markers)/sizeof(u32); ++it)
			(*it)->update_markers[i] = 0;

	m_constant_update_counter = 1;
}


} // namespace render 
} // namespace xray 
