////////////////////////////////////////////////////////////////////////////
//	Created		: 04.06.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "texture_storage.h"
#include <xray/render/core/sources/texture_pool.h>
#include <xray/render/core/device.h>

namespace xray {
namespace render_dx10 {

u32 get_hash( u32 width, u32 height, DXGI_FORMAT format)
{
	return (utils::log_2( width) | (utils::log_2( height)<<5) | ( (u32)format<<10));
}

texture_storage::texture_storage():
m_initialized	(false)
{
	
}

texture_storage::~texture_storage() 
{
	pools::iterator			it	= m_pools.begin();
	pools::const_iterator	end	= m_pools.end();

	for( ; it != end; ++it)
		DELETE( it->second);
}


void texture_storage::initialize( xray::configs::lua_config_value  const & initial_params, u32 memory_amount, D3D_USAGE usage)
{
	ASSERT( !m_initialized);

	if( m_initialized)
		return;

	m_initialized = true;

	xray::configs::lua_config_value::const_iterator it = initial_params.begin();
	xray::configs::lua_config_value::const_iterator end = initial_params.end();

	float base_memory_usage = 0;
	for( ; it != end; ++it)
	{
		u32 width	= (*it)["width"];
		u32 height	= (*it)["height"];
		u32 format	= (*it)["format"];
		u32 count	= (*it)["count"];

		u32 mips = utils::calc_mipmap_count( width, height);

		base_memory_usage += count*utils::calc_texture_size( width, height, (DXGI_FORMAT)format, mips);
	}

	float memory_mag_factor = memory_amount/base_memory_usage;

	it = initial_params.begin();

	for( ; it != end; ++it)
	{
		u32 width	= (*it)["width"];
		u32 height	= (*it)["height"];
		u32 format	= (*it)["format"];
		u32 count	= (*it)["count"];

		u32 mips = utils::calc_mipmap_count( width, height);

		m_pools.insert( utils::mk_pair( get_hash( width, height, (DXGI_FORMAT)format), 
										NEW(texture_pool)( width, height, (DXGI_FORMAT)format, mips, u32(count*memory_mag_factor + 0.999f) /*ceil*/, usage )));
	}
}


res_texture*	texture_storage::get		( u32 width, u32 height, DXGI_FORMAT format)
{
	pools::iterator it = m_pools.find( get_hash( width, height, format));

	if( it == m_pools.end())
		return NULL;

	return it->second->get( );
}

void	texture_storage::release	( res_texture* texture)
{
	pools::iterator it = m_pools.find( get_hash( texture->width(), texture->height(), texture->format()));

	if( it == m_pools.end())
	{
		ASSERT( false, "There is no initialized pool with specified parameters.");
		return;
	}
	
	it->second->release( texture);
}


} // namespace render_dx10
} // namespace xray
