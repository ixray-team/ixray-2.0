////////////////////////////////////////////////////////////////////////////
//	Created		: 02.03.2010
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef IMAGES16X16_H_INCLUDED
#define IMAGES16X16_H_INCLUDED

namespace xray {
namespace editor_base {

enum images16x16 {
	// lights
	ambient_light,
	area_light,
	directional_light,
	point_light,
	spot_light,
	volume_light,

	// tree nodes
	folder_closed,
	folder_open,
	folder_closed_modified,
	folder_open_modofied,
	node_resource,
	node_resource_modified,
	node_curve,
	node_group,
	node_joint,
	node_mesh,
	node_camera,
	node_sound_file,

	// tools
	tool_lasso,
	tool_manipulator,
	tool_paint_selection,
	tool_translate,
	tool_rotate,
	tool_scale,
	tool_select,

	// terrain modifiers
	terrain_bump,
	terrain_errosion,
	terrain_flatten,
	terrain_noise,
	terrain_raise_lower,
	terrain_ramp,
	terrain_smooth,
	terrain_painter,
	terrain_diffuse_painter,

	// modes
	mode_editor,
	mode_pause,

	// miscellanious
	default_object_set,

	// add new items here!

	images16x16_count,
}; // enum images16x16

} // namespace editor_base
} // namespace xray

#endif // #ifndef IMAGES16X16_H_INCLUDED