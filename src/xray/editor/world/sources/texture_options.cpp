////////////////////////////////////////////////////////////////////////////
//	Created		: 11.02.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "texture_options.h"
#include <xray/editor/base/managed_delegate.h>
#include "resource_editor.h"

using namespace System;

namespace xray {
namespace editor {

	static void set_default_parameters(texture_options* );

		texture_options::texture_options			(fs::path_string absolute_resource_path, xray::fs::path_string file_path):
	resource_options(absolute_resource_path, file_path),
	type(type_2d),
	width(0),
	height(0),
	alpha_blend_mode(none),
	pack_mode(packed),
	has_mip_levels(true),
	normal_name(""),
	color_name(""),
	detail_scale(0),
	bump_mode(paralax),
	bump_name(""),
	tile(128),
	m_is_default_values(true)
{
	this->type = get_type_from_name(this->m_resource_name.c_str());
	set_default_parameters(this);
}

		texture_options::texture_options			(fs::path_string absolute_resource_path, configs::lua_config_ptr config):
	resource_options(absolute_resource_path), m_is_default_values(false)
{
	this->m_resource_name.append(config->get_file_name());
	this->type = get_type_from_name(this->m_resource_name.c_str());
	load(config);
}

texture_options::ETType	texture_options::get_type_from_name(pcstr name)
{
	fs::path_string resource_path = name;
	fs::path_string resource_name;

	resource_name.assign(resource_path.begin()+resource_path.rfind('/')+1, resource_path.end());

	if(resource_name.find("terrain") == 0)
	{
		if( resource_name.find( "bump", resource_name.rfind('_') ) != -1 )			return texture_options::terrain_bump;
		if( resource_name.find( "color", resource_name.rfind('_') ) != -1 )			return texture_options::terrain_color;
		return texture_options::terrain;
	}
	else
	{
		if( resource_name.find( "sky", resource_name.rfind('_') ) != -1 ) 			return texture_options::sky;
		if( resource_name.find( "bump", resource_name.rfind('_') ) != -1 )			return texture_options::bump;
		if( resource_name.find( "nmap", resource_name.rfind('_') ) != -1 )			return texture_options::normal_map;
		return texture_options::type_2d;
	}
}

static void set_default_parameters(texture_options* options)
{
	switch(options->type)
	{
	case texture_options::sky:
		options->has_mip_levels = false;
		break;
	case texture_options::bump:
		options->alpha_blend_mode = texture_options::blend;
		break;
	//case normal_map:
	//case terrain:
	//case terrain_bump:
	//case terrain_color:
	//case type_2d:
	}
}

void	texture_options::load							(configs::lua_config_ptr config)
{
	//get config root
	configs::lua_config_value const& config_root = config->get_root();

	configs::lua_config_value table = config_root["options"]; 

	//Getting properties
	width					=		table["width"];
	height					=		table["height"];

	alpha_blend_mode		=		static_cast<ETAlphaBlendMode>((u32)table["alpha_blend_mode"]); 
	pack_mode				=		static_cast<ETPackMode>((u32)table["pack_mode"]); 
	has_mip_levels			=		((u32)table["has_mip_levels"]) != 0;

	// detail ext
	normal_name				=		(pcstr)table["normal_name"];
	color_name				=		(pcstr)table["color_name"];
	detail_scale			=		table["detail_scale"];
	// bump	
	bump_mode				=		static_cast<ETBumpMode>((u32)table["bump_mode"]);
	bump_name				=		(pcstr)table["bump_name"];

	tile					=		table["tile"];
}

void	texture_options::save							()
{
	//create correct path
	
	String^ file_path = Path::GetFullPath(gcnew String(m_absolute_resource_path.c_str())+"/textures_new/sources/"+(gcnew String(m_resource_name.c_str()))+".options");
	
	//open config file and get root of it
	configs::lua_config_ptr	const&		config	= configs::create_lua_config(unmanaged_string(file_path).c_str());
	configs::lua_config_value config_options	= config->get_root()["options"];

	//store data to config file
	
	config_options["width"]					= width;
	config_options["height"]				= height;

	config_options["alpha_blend_mode"]		= safe_cast<Int32>(alpha_blend_mode);
	config_options["pack_mode"]				= safe_cast<Int32>(pack_mode);
	config_options["has_mip_levels"]		= safe_cast<Int32>(has_mip_levels);

	config_options["normal_name"]			= normal_name.c_str();
	config_options["color_name"]			= color_name.c_str();
	config_options["detail_scale"]		    = detail_scale;

	config_options["bump_mode"]				= safe_cast<Int32>(bump_mode);
	config_options["bump_name"]				= bump_name.c_str();

	config_options["tile"]					= tile;
	//save config file to disk
	config->save();
}

}//namespace editor
}//namespace xray