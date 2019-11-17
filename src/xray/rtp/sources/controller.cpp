////////////////////////////////////////////////////////////////////////////
//	Created		: 03.11.2009
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "controller.h"

#include "test_grasping_controller.h"
#include "animation_grasping_controller.h"
#include "navigation_controller.h"

#include <xray/animation/world.h>
//#include "test_grasping_action.h"

namespace xray {
namespace rtp {

void learn_step_controllers(  vector< base_controller* > &controllers )
{
		vector< base_controller*>::iterator b = controllers.begin(), e = controllers.end();
		vector< base_controller*>::iterator  i = b;	

		for( ; i != e  ; ++i )
		{
			//float max_residual = (*i)->learn_step();
			(*i)->step_logic(true);
			//LOGI_INFO("", "%f", max_residual );
		}
}

void	learn_init_controllers(  vector< base_controller* > &controllers )
{
		vector< base_controller*>::iterator b = controllers.begin(), e = controllers.end();
		vector< base_controller*>::iterator  i = b;	

		for( ; i != e  ; ++i )
		{

			(*i)->learn_init();
		
		}
}


void	learn_stop_controllers(  vector< base_controller* > &controllers )
{
		vector< base_controller*>::iterator b = controllers.begin(), e = controllers.end();
		vector< base_controller*>::iterator  i = b;	

		for( ; i != e  ; ++i )
		{

			(*i)->learn_stop();
		
		}
}

base_controller* create_test_grasping_controller(  )
{
	//g_allocator->user_thread_id	(threading::current_thread_id()); 
	return NEW( test_grasping_controller );
}


base_controller* create_animation_grasping_controller( animation::world &w )
{
	//g_allocator->user_thread_id	(threading::current_thread_id());
	animation_grasping_controller *ctrl = NEW( animation_grasping_controller )( w.dbg_tmp_controller_set ( ) );
	ctrl->debug_test_add_actions( );
	return ctrl;
}

base_controller* create_navigation_controller( animation::world &w )
{

	navigation_controller *ctrl = NEW( navigation_controller )( w.dbg_tmp_controller_set ( ) );
	ctrl->debug_test_add_actions( );
	return ctrl;
}


base_controller* create_controller( pcstr type, animation::world &w )
{
	
	//g_allocator->user_thread_id	(threading::current_thread_id());

	if( strings::compare( type, "test_grasping" ) == 0 )
		return  NEW( test_grasping_controller )(  );
	else if( strings::compare( type, "animation_grasping" ) == 0 )
		return NEW( animation_grasping_controller )( w.dbg_tmp_controller_set ( ) );
	else if( strings::compare( type, "navigation" ) == 0 )
		return NEW( navigation_controller )( w.dbg_tmp_controller_set ( ) );

		//return create_animation_grasping_controller( w );

	NODEFAULT(return 0);
}

} // namespace rtp
} // namespace xray