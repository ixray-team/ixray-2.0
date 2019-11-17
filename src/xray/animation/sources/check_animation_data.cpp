////////////////////////////////////////////////////////////////////////////
//	Created		: 08.02.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "check_animation_data.h"
#include "skeleton_animation_data.h"
#include "skeleton_animation.h"
#include <xray/animation/world.h>

#if XRAY_USE_MAYA_ANIMATION
#	include <xray/maya_animation/world.h>
#	include <xray/maya_animation/api.h>
#	include <xray/maya_animation/discrete_data.h>
#endif // #if XRAY_USE_MAYA_ANIMATION

namespace xray {

namespace maya_animation
{
	class discrete_data;
	struct world;
} // namespace maya_animation

namespace animation {

bool	compare(	const xray::maya_animation::discrete_data &disc_data, 
					const skeleton_animation &anim )
{

	XRAY_UNREFERENCED_PARAMETER( anim );
	XRAY_UNREFERENCED_PARAMETER( disc_data );

#if XRAY_USE_MAYA_ANIMATION
	const u32 num_bones = disc_data.bone_count();
	if( anim.bone_count() != num_bones ) 
		return false;
	
	const u32 frame_count = disc_data.frame_count();

	for( u32 bone = 0; bone < num_bones; ++bone )
	{

			for(u32 ch = channel_translation_x; ch  < channel_max; ++ch )
			{
				for(u32 frame_number = 0; frame_number < frame_count; ++frame_number )	
				{
					xray::animation::frame frame; float time = xray::math::QNaN;
					disc_data.get_frame( bone, frame_number, time, frame ) ;

					float v = xray::math::QNaN;
					anim.bone( bone ).channel( enum_channel_id(ch) ).evaluate( time, v );
				
					if(! xray::math::similar(v,frame.channels[ch],disc_data.precision( bone, enum_channel_id(ch) ) ) )
						return false;
				}
			}
	}
#endif // #if XRAY_USE_MAYA_ANIMATION

	return true;
}

void	test_data(	xray::configs::lua_config_value	const &animation_data, 
					xray::configs::lua_config_value	const &check_data,
					animation::world &world
								)
{

	XRAY_UNREFERENCED_PARAMETER( animation_data );
	XRAY_UNREFERENCED_PARAMETER( check_data );
	XRAY_UNREFERENCED_PARAMETER( world );

#if XRAY_USE_MAYA_ANIMATION
	xray::maya_animation::engine* meng(0);
	xray::maya_animation::memory_allocator( *xray::animation::g_allocator );
	xray::maya_animation::world *maw = xray::maya_animation::create_world( *meng );
	ASSERT( maw );

	xray::maya_animation::discrete_data *disc_data = maw->create_discrete_data();

	disc_data->load( check_data["discrete_data"] );

	xray::animation::i_skeleton_animation_data *ianim_data_builded	= world.create_skeleton_animation_data( 0 );
	xray::animation::i_skeleton_animation_data *ianim_data_loaded	= world.create_skeleton_animation_data( 0 );
	
	xray::animation::skeleton_animation_data *anim_data_builded =  static_cast_checked<xray::animation::skeleton_animation_data*> ( ianim_data_builded  );
	xray::animation::skeleton_animation_data *anim_data_loaded =  static_cast_checked<xray::animation::skeleton_animation_data*> ( ianim_data_loaded );

	maw->build_animation_data( *disc_data, *anim_data_builded );

	anim_data_loaded->load( animation_data["splines"] );

	vector< u32 > identity_index;
	const u32 bone_count = anim_data_loaded->num_bones();
	identity_index.resize( bone_count );
	for( u32 bone = 0 ; bone < bone_count ; ++bone )
		identity_index[bone] = bone;


	skeleton_animation *anim_builded = NEW(skeleton_animation)( *anim_data_builded, identity_index );
	ASSERT( compare( *disc_data, *anim_builded ) );
	skeleton_animation *anim_loaded = NEW(skeleton_animation)( *anim_data_loaded, identity_index );
	ASSERT( compare( *disc_data, *anim_loaded ) );

	ASSERT( anim_data_builded->compare( *anim_data_loaded ) );
	
	maw->destroy( disc_data );
	world.destroy( ianim_data_builded );
	world.destroy( ianim_data_loaded );
	DELETE( anim_builded );
	DELETE( anim_loaded );
	xray::maya_animation::destroy_world( maw );
#endif // #if XRAY_USE_MAYA_ANIMATION
}

} // namespace animation
} // namespace xray
