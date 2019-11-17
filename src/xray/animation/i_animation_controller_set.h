////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef I_ANIMATION_CONTROLLER_SET_H_INCLUDED
#define I_ANIMATION_CONTROLLER_SET_H_INCLUDED

namespace xray {
namespace animation {

class i_animation_action;

class i_animation_controller_set {

public:
	virtual i_animation_action const	*taget_action			()const = 0;
	virtual	void						get_taget_transform		( float4x4 &m )const = 0;
	virtual i_animation_action const	*action					( u32 id )const = 0;
	virtual u32							num_actions				( )const = 0;
	virtual void						set_callback_on_loaded	( const boost::function< void ( ) >	 &cb  ) = 0;
	virtual	void						render					(  render::debug::renderer& renderer, float time_global ) const = 0;
	virtual	float4x4			const	&position				()const = 0;
	virtual float						crrent_action_start_time()const = 0;
	virtual	bool						loaded					()const = 0;
	virtual	void						reset					( )= 0;
	virtual	void						init					( u32 id, const buffer_vector< float > &weights, float time_global ) = 0;
	virtual	void						get_bone_world_matrix	( float4x4 &m, u32 bone_id ) const = 0;

	//virtual	void						start_transition		( const animation_group_action& next_action, float4x4 &root_displacement, const buffer_vector< float > &weights, float time_global ) = 0;
	//virtual	void						update					( float4x4 &root_displacement, float time_global ) = 0;

	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( i_animation_controller_set )
}; // class i_animation_controller_set

} // namespace animation
} // namespace xray

#endif // #ifndef I_ANIMATION_CONTROLLER_SET_H_INCLUDED