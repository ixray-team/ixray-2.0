////////////////////////////////////////////////////////////////////////////
//	Created		: 16.12.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "terrain_object.h"
#include "terrain_quad.h"
#include <xray/macro_library_name.h>

#include <FreeImage/FreeImage.h>
#pragma comment(lib, XRAY_LIBRARY_NAME( FreeImage, lib ) )
#define RGB2GRAY(r,g,b) (((b)*117 + (g)*601 + (r)*306) >> 10)
static u32	color_argb	(u8 a, u8 r, u8 g, u8 b)		{	return (a<<24)|(r<<16)|(g<<8)|(b);}

namespace xray {
namespace editor {

void terrain_core::import(terrain_import_settings^ settings, terrain_node_key lt_key, terrain_node_key rb_key)
{
	unmanaged_string fn		(settings->m_source_filename);
	FREE_IMAGE_FORMAT	fif = FreeImage_GetFIFFromFilename(fn.c_str());
	FIBITMAP* fibitmap		= FreeImage_Load(fif, fn.c_str());
	u32 bitmap_width		= FreeImage_GetWidth(fibitmap);
	u32 bitmap_height		= FreeImage_GetHeight(fibitmap);
	RGBQUAD					rgb_quad;
	
	int node_sz				= 64;
	int area_w				= (rb_key.x - lt_key.x + 1) * node_sz;
	int area_h				= (lt_key.z - rb_key.z + 1) * node_sz;
	ASSERT					(area_w>0 && area_h>0);
	
	float k_w				= bitmap_width / (float)area_w;
	float k_h				= bitmap_height / (float)area_h;

	for(int ix = lt_key.x; ix<=rb_key.x; ++ix)
		for(int iz = lt_key.z; iz>=rb_key.z; --iz)
		{
			terrain_node_key	key(ix, iz);
			if(!m_nodes.ContainsKey(key))
			{
				if(settings->m_b_create_cell)
				{
					LOG_INFO("import: new node [%d:%d]", key.x, key.z);
					create_node(key, 64);
				}else
				{
					LOG_INFO("import: skip empty node [%d:%d]", key.x, key.z);
					continue;
				}
			}else
			{
				LOG_INFO("import: node exist [%d:%d]", key.x, key.z);
			}
			
			terrain_node^	terrain = m_nodes[key];
			int image_lt	= math::floor( (ix - lt_key.x)*node_sz*k_w );
			int image_rb	= math::floor( (lt_key.z - iz)*node_sz*k_h );
			
			vert_id_list^ id_list		= gcnew vert_id_list;
			for(int cx=0; cx<terrain->m_dimension+1; ++cx)
				for(int cz=0; cz<terrain->m_dimension+1; ++cz)
				{
					u32 idx				= terrain->vertex_id(cx, cz);
					id_list->Add		((u16)idx);
					terrain_vertex% v	= terrain->m_vertices[idx];

					int px 				= image_lt + math::floor( k_w * cx);
					int py 				= image_rb + math::floor( k_h * cz);

					FreeImage_GetPixelColor(fibitmap, px, bitmap_height-py-1, &rgb_quad); // v flip

					switch (settings->m_options)
					{
						case terrain_import_settings::options::rgba_diffuse:
						{
							v.vertex_color	= color_argb(rgb_quad.rgbReserved, rgb_quad.rgbRed, rgb_quad.rgbGreen, rgb_quad.rgbBlue);
						}break;

						case terrain_import_settings::options::rgb_diffuse:
						{
							v.vertex_color	= color_argb(255, rgb_quad.rgbRed, rgb_quad.rgbGreen, rgb_quad.rgbBlue);
						}break;

						case terrain_import_settings::options::rgb_diffuse_a_heightmap:
						{
							v.vertex_color	= color_argb(255, rgb_quad.rgbRed, rgb_quad.rgbGreen, rgb_quad.rgbBlue);
							int hmap		= rgb_quad.rgbReserved - 128;
							v.height		= hmap*settings->m_height_scale;
						}break;

						case terrain_import_settings::options::a_heightmap:
						{
							int hmap		= rgb_quad.rgbReserved - 128;
							v.height		= hmap*settings->m_height_scale;
						}break;
					};
					terrain->m_vertices[idx]= v;
				}
			// sync terrain
			sync_visual_vertices	(terrain, id_list);
		}
	FreeImage_Unload		(fibitmap);
}

} // namespace editor
} // namespace xray