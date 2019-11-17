////////////////////////////////////////////////////////////////////////////
//	Created 	: 17.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef ANIMTION_WORLD_H_INCLUDED
#define ANIMTION_WORLD_H_INCLUDED

#include <xray/animation/world.h>

#include "poly_curve.h"
#include "bi_spline_data.h"
#include "skeleton_animation_data.h"
#include "resource_cookers.h"

namespace xray {

namespace configs{
	class lua_config_value;
} // namespace configs


namespace animation {

struct	engine;
struct  i_bone_animation_data;
struct	i_skeleton_animation_data;
class	skeleton;
class	skeleton_animation;
class	animation_mix;
class	i_bone_names;
class	animation_group;
class	animation_controller_set;
class	test_animation_player;

class animation_world:
	public animation::world,
	private boost::noncopyable
{
public:
					animation_world		( animation::engine& engine );
	virtual			~animation_world	( );

public:
	virtual	void	tick				( );

public:
	virtual skeleton_animation_data		*create_skeleton_animation_data			( u32 num_bones );
	virtual	void						destroy									( i_skeleton_animation_data* &p );
	virtual	void						render									( xray::render::debug::renderer& renderer ) const	;
	virtual i_animation_controller_set	*dbg_tmp_controller_set					( ) ;

	virtual	i_animation_controller_set	*create_controller_set					( configs::lua_config_value	const& cfg ) ;
	virtual	void						destroy_controller_set					(i_animation_controller_set* &set );

private:
	void								load_data								();
	void								load_test_animations					();

	void								on_anim_data_loaded						( xray::resources::queries_result& data );
	
	
	void								test_update_animation_controller_set	();

private:
	void								test_data								( configs::lua_config_value	const& animation_data, configs::lua_config_value	const& check_data  );

private:
	engine&										m_engine;

	bool										b_load;
	bool										b_data_loaded;

	skeleton									*skeleton_0;


	resources::unmanaged_resource_ptr			ptr_skeleton_0;
	skeleton_animation							*skeleton_animation_1;
	animation_controller_set					*m_animation_controller_set;

	vector< skeleton_animation* >				m_test_animations;		

	

	animation_mix								*m_animation_mix;
	
	//vector<u32>								m_index;
	xray::timing::timer							m_timer;
	

	resource_cookers							*m_resources;

	test_animation_player*						m_test_animation_player;
}; // class world

} // namespace animation
} // namespace xray

#endif // #ifndef ANIMTION_WORLD_H_INCLUDED