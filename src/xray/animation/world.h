////////////////////////////////////////////////////////////////////////////
//	Created 	: 17.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_ANIMATION_WORLD_H_INCLUDED
#define XRAY_ANIMATION_WORLD_H_INCLUDED

namespace xray {

namespace render {
namespace debug {

	struct renderer;

} // namespace render
} // namespace xray

namespace animation {

struct handler;
struct i_skeleton_animation_data;
class  i_animation_controller_set;

struct XRAY_NOVTABLE world {

	virtual	void						tick							( ) = 0;
	virtual	void						render							( xray::render::debug::renderer& renderer ) const	=0;
	virtual i_skeleton_animation_data	*create_skeleton_animation_data	( u32 num_bones ) = 0;
	virtual	void						destroy							( i_skeleton_animation_data	* &p ) =0;

	virtual	i_animation_controller_set	*create_controller_set			( configs::lua_config_value	const& cfg ) = 0;
	virtual	void						destroy_controller_set			( i_animation_controller_set* &set ) = 0;
/////////////////////////////////////
	virtual i_animation_controller_set	*dbg_tmp_controller_set			( ) =0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( world )
}; // class world

} // namespace animation
} // namespace xray

#endif // #ifndef XRAY_ANIMATION_WORLD_H_INCLUDED