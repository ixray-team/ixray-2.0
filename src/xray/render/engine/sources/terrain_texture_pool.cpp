////////////////////////////////////////////////////////////////////////////
//	Created		: 05.03.2010
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <xray/render/engine/terrain_texture_pool.h>
#include <xray/render/core/resource_manager.h>

namespace xray {
namespace render_dx10 {

terrain_texture_pool::terrain_texture_pool	( u32 const size, u32 const textures_count):
m_tiles				( (u32)math::ceil(math::sqrt( (float)textures_count)), (u32)math::ceil(math::sqrt( (float)textures_count))),
m_tile_size			( size),
m_textures_count	( m_tiles.x*m_tiles.y)
{
	ASSERT( m_tiles.x*size <= 4096 && m_tiles.y*size <= 4096);

// --Porting to DX10_
//	if( device::ref().support(D3DFMT_DXT1,D3DRTYPE_TEXTURE,D3DUSAGE_AUTOGENMIPMAP) )
	m_pool_texture	= resource_manager::ref().create_texture2d( get_pool_texture_name(), size, size, NULL, DXGI_FORMAT_BC1_UNORM /*D3DFMT_DXT1 not sure*/, D3D_USAGE_DEFAULT, 9, 16);
		
	ASSERT( m_pool_texture);
}

terrain_texture_pool::~terrain_texture_pool	()
{

}

int	terrain_texture_pool::add_texture		( render::texture_string const & name, bool deferred_load)
{
	int		ind		= -1;
	for( u32 i = 0; i < m_textures_count; ++i)
	{
		if( m_texture_items[i].name.length() == 0 )
		{
			if( ind == -1)
				ind = i;
		}
		else if( m_texture_items[i].name == name)
		{
			if( !deferred_load && !m_texture_items[i].loaded)
			{
				load_texture(i);
				m_texture_items[i].loaded = true;
			}

			return i;
		}
	}

	if( ind >= 0)
	{
		m_texture_items[ind].name		= name;
		if( !deferred_load)
		{
			load_texture(ind);
			m_texture_items[ind].loaded		= true;
		}
		else
			m_texture_items[ind].loaded		= false;
	}

	return ind;
}

void terrain_texture_pool::remove_texture	( u32 id)
{
	ASSERT( id < render::terrain_texture_max_count);
	ASSERT( m_texture_items[id].name != "" );

	m_texture_items[id].name = "";
	m_texture_items[id].loaded	= false;
}

void terrain_texture_pool::load_textures	()
{
	for( u32 i = 0; i < m_textures_count; ++i)
	{
		if( m_texture_items[i].name.length() > 0 && !m_texture_items[i].loaded )
		{
			load_texture(i);
			m_texture_items[i].loaded = true;
		}
	}
}

math::rectangle<int2>	terrain_texture_pool::get_tile_rect	(u32 ind)
{
	ASSERT(ind < m_tiles.x*m_tiles.y);
	int top		= ind/m_tiles.x;
	int left	= ind-top*m_tiles.x;

	return math::rectangle<int2>( int2(left*m_tile_size, top*m_tile_size), int2((left+1)*m_tile_size, (top+1)*m_tile_size));
}

bool	terrain_texture_pool::exchange_texture( render::texture_string const & old_texture, render::texture_string const &  new_texture, bool deffered_load)
{
	int	ind = get_texture_id( old_texture);
	ASSERT( ind >= 0);

	m_texture_items[ind].name		= new_texture;

	if( !deffered_load)
	{
		load_texture(ind);
		m_texture_items[ind].loaded		= true;	
	}
	else
		m_texture_items[ind].loaded		= false;
	
	return false;
}

int		terrain_texture_pool::get_texture_id	( render::texture_string const & name)
{
	for( u32 i = 0; i < m_textures_count; ++i)
	{
		if( m_texture_items[i].name ==  name)
			return i;
	}
	return -1;
}

void	terrain_texture_pool::load_texture	( u32 ind)
{
	ASSERT( m_texture_items[ind].name.length() > 0 && !m_texture_items[ind].loaded);
	resource_manager::ref().copy_texture_from_file( m_pool_texture, get_tile_rect(ind), ind, m_texture_items[ind].name.c_str() );	
}

} // namespace render
} // namespace xray
