////////////////////////////////////////////////////////////////////////////
//	Created		: 09.02.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "maya_engine.h"
#include <xray/fs_path.h>
#include <xray/fs_utils.h>
namespace xray {
namespace maya{


maya_engine::maya_engine( ): 
animation_world( 0 ), 
maya_animation_world( 0 )
{
	assign_resource_path( );
}

render::world&	maya_engine::get_renderer_world( )	
{
	UNREACHABLE_CODE(return *((render::world*)(0)));
}

void module_path_dir( string_path dir_path )
{
	pcstr module_name = "xray.maya.mll";
	HMODULE handle = GetModuleHandle( module_name  );
	ASSERT( handle );
	
	GetModuleFileName( handle, dir_path, sizeof( string_path ) );

	fs::path_string path_string = xray::fs::convert_to_portable( dir_path );

	fs::path_string dir_path_string;
	xray::fs::directory_part_from_path( &dir_path_string, path_string.c_str() );

	xray::strings::copy( dir_path, sizeof( string_path ), dir_path_string.c_str() );
}

void	maya_engine::assign_resource_path( )
{
	module_path_dir( m_resource_path );
	xray::strings::join( m_resource_path, sizeof( m_resource_path ), m_resource_path, "/../../resources/" );
	module_path_dir( m_underscore_G_path );
	xray::strings::join( m_underscore_G_path, sizeof( m_underscore_G_path ), m_underscore_G_path, "/../../resources/scripts/" );
}

pcstr	maya_engine::get_resource_path( )const			
{ 
	return m_resource_path; 
}

pcstr	maya_engine::get_underscore_G_path( )const
{ 

	return m_underscore_G_path; 
}

} // namespace maya
} // namespace xray