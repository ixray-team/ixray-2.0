////////////////////////////////////////////////////////////////////////////
//	Created		: 07.06.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "object_cooker.h"
#include "object_solid_visual.h"
#include "object_light.h"

namespace stalker2 {

object_cooker::object_cooker( game& game)
:super	( resources::game_object_class, reuse_true, use_current_thread_id, use_current_thread_id ),
m_game	( game )
{}

mutable_buffer object_cooker::allocate_resource( resources::query_result_for_cook&	in_query, 
												 const_buffer						raw_file_data, 
												 bool								file_exist )
{
	XRAY_UNREFERENCED_PARAMETERS	( file_exist, &in_query );
	configs::lua_config_value* t_object = (configs::lua_config_value*)(raw_file_data.c_ptr());
	pcstr type					= (*t_object)["tool_name"];

	u32 object_size = 0;
	if( 0==strings::compare("solid_visual", type))
	{
		object_size = sizeof(object_solid_visual);
	}else 
	if( 0==strings::compare("light", type))
	{
		object_size = sizeof(object_light);
	}else
	{
		NOT_IMPLEMENTED				( );
	}

	pvoid data				= MALLOC( object_size, type );
	mutable_buffer	result	( data, object_size );
	return result;

}

void object_cooker::deallocate_resource( pvoid buffer )
{
	FREE		( buffer );
}

void   object_cooker::create_resource (resources::query_result_for_cook &	in_out_query, 
								 	   const_buffer						raw_file_data, 
									   mutable_buffer						in_out_unmanaged_resource_buffer)
{
	configs::lua_config_value* t_object = (configs::lua_config_value*)(raw_file_data.c_ptr());
	pcstr type							= (*t_object)["tool_name"];

	game_object*	resource = NULL;
	if( 0==strings::compare("solid_visual", type))
	{
		resource = new (in_out_unmanaged_resource_buffer.c_ptr()) object_solid_visual( m_game );
	}else 
	if( 0==strings::compare("light", type))
	{
		resource = new (in_out_unmanaged_resource_buffer.c_ptr()) object_light( m_game );
	}else
	{
		NOT_IMPLEMENTED				( );
	}

	resource->load(	*t_object );

	in_out_query.set_unmanaged_resource( resource, resources::memory_type_non_cacheable_resource, in_out_unmanaged_resource_buffer.size() );
	
	in_out_query.finish_query		( result_success );
}

void object_cooker::destroy_resource( resources::unmanaged_resource* resource )
{
	resource->~unmanaged_resource();
}



} // namespace stalker2
