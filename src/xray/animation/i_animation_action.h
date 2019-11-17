////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef I_ANIMATION_ACTION_H_INCLUDED
#define I_ANIMATION_ACTION_H_INCLUDED

namespace xray {

namespace render {
namespace debug {
	struct renderer;
} // namespace debug
} // namespace render

namespace animation {

class i_animation_controller_set;

class i_animation_action {

public:
	virtual void	get_displacement_transform	( float4x4 &m,  const buffer_vector< float > &weights )const = 0;
	virtual	u32		animations_number			( )const = 0;
	virtual float	duration					( )const = 0;
	virtual	float	run							( i_animation_controller_set *set, const buffer_vector< float > &from_weights, const buffer_vector< float > &weights, u32 step, float time )const =0;
	virtual	u32		get_id						( )const = 0;
	virtual	u32		gait						( )const = 0;
	virtual	u32		bone_index					( pcstr bone_name )const =0;
	virtual void	local_bone_matrix			( float4x4 &m,  u32 bone_idx,  const buffer_vector< float > &weights )const =0;
	virtual pcstr	name						( )const = 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( i_animation_action )

}; // class i_animation_action

} // namespace animation
} // namespace xray

#endif // #ifndef I_ANIMATION_ACTION_H_INCLUDED