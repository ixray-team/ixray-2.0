////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_CLASSES_H_INCLUDED
#define XRAY_RESOURCES_CLASSES_H_INCLUDED

namespace xray {
namespace resources {

enum class_id
{ 
	unknown_data_class,
	raw_data_class,
	texture_wrapper_class,
	texture_class,
	texture_converter_class,
	texture_tga_to_argb_class,
	visual_class,
	solid_visual_class,
	collision_class,
	config_class,
	binary_config_class,
	config_ltx_class,
	config_lua_class,
	test_resource_class,
	ogg_sound_wrapper_class,
	ogg_sound_class,
	ogg_class,
	ogg_converter_class,
	ogg_options_converter_class,
	animation_data_class,
	skeleton_class,
	texture_options_class,
	terrain_cell_class,
	dialog_resources_class,
	sound_class,
	sound_stream_class,
	ogg_raw_file,
	dialog_class,
	game_dialog_class,
	game_project_class,
	game_cell_class,
	game_object_class,
	last_resource_class,
};

} // namespace resources
} // namespace xray

#endif // #ifndef XRAY_RESOURCES_CLASSES_H_INCLUDED