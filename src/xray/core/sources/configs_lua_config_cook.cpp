////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "configs_lua_config_cook.h"

namespace xray {
namespace core {
namespace configs {

extern memory::doug_lea_allocator_type	g_lua_allocator;

mutable_buffer   lua_config_cook::allocate_resource	(resources::query_result_for_cook &	in_query, 
													 const_buffer						raw_file_data, 
													 bool								file_exist)
{
	XRAY_UNREFERENCED_PARAMETERS			(& in_query, & raw_file_data, & file_exist);
	return									mutable_buffer (lua_allocate( sizeof(xray::configs::lua_config) ), 
															sizeof(xray::configs::lua_config));
}

void   lua_config_cook::destroy_resource	(resources::unmanaged_resource * resource)
{
	destroy_lua_config						(resource);
}

void   lua_config_cook::deallocate_resource	(pvoid buffer)
{
	lua_deallocate							(buffer);
}

void   lua_config_cook::create_resource (resources::query_result_for_cook &	in_out_query, 
											 						  const_buffer							raw_file_data,
											 						  mutable_buffer						in_out_unmanaged_resource_buffer)
{
	xray::memory::reader reader		((u8 const*)raw_file_data.c_ptr(), raw_file_data.size());

	pstr path						= 0;	
	STR_DUPLICATEA					( path, in_out_query.get_requested_path() );	
	pstr const found				= strrchr( path, '.');	
	if ( found )		
		*found						= 0;	
	char const resources_string[]	= "resources/";	
	if ( strings::starts_with(path, resources_string) )		
		path						+= sizeof(resources_string) - 1;

	xray::configs::lua_config *	out_config	=	create_lua_config_inplace( in_out_unmanaged_resource_buffer, path, reader);

	in_out_query.set_unmanaged_resource	(out_config, 
										 resources::memory_type_non_cacheable_resource, 
										 in_out_unmanaged_resource_buffer.size());

	in_out_query.finish_query		(result_success);
}

} // namespace configs
} // namespace core
} // namespace xray