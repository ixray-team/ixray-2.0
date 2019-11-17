////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "solid_visual_exporter.h"
#include "solid_visual_writer.h"
#include <xray/core/sources/fs_file_system.h>
#include <xray/resources_fs.h>

using namespace xray;

const MString solid_visual_exporter::Name	( "xray_solid_visual_export" );

#define file_name_flag			"-o"
#define file_name_flag_l		"-outfilename"

#define mesh_name_flag			"-im"
#define mesh_name_flag_l		"-inputmesh"

#define collision_name_flag		"-c"
#define collision_name_flag_l	"-collisionmesh"

#define tootle_opt_flag			"-t"
#define tootle_opt_flag_l		"-tootle"

#define tootle_opt_hq_flag		"-hq"
#define tootle_opt_hq_flag_l	"-hq_l"

MSyntax solid_visual_exporter::newSyntax()
{
    MSyntax			syntax;
	syntax.addFlag	( file_name_flag, file_name_flag_l, MSyntax::kString );
	syntax.addFlag	( mesh_name_flag, mesh_name_flag_l, MSyntax::kString );
	syntax.addFlag	( collision_name_flag, collision_name_flag_l, MSyntax::kString );
	syntax.addFlag	( tootle_opt_flag, tootle_opt_flag_l, MSyntax::kNoArg );
	syntax.addFlag	( tootle_opt_hq_flag, tootle_opt_hq_flag_l, MSyntax::kNoArg );


	syntax.enableEdit( false );

	return syntax;
}

void* solid_visual_exporter::creator( )
{
	return CRT_NEW(solid_visual_exporter)();
}

MStatus solid_visual_exporter::doIt( const MArgList& arglist)
{
	MArgDatabase argData( syntax(), arglist );

	MStatus						result;
	MString						fileName;

	if( argData.isFlagSet( file_name_flag ) )
	{
		result		= argData.getFlagArgument( file_name_flag, 0, fileName );
		CHK_STAT_R	(result);
	}else
	{
		MGlobal::displayError	("solid_visual_exporter: output file name flag (-outfilename) not specified");
		return					MStatus::kFailure;
	}

	solid_visual_writer writer	( argData, result );
	CHK_STAT_R					( result );

	MDagPath					dag_path;

	if( argData.isFlagSet( mesh_name_flag ) )
	{
		MString					mesh_name;
		result					= argData.getFlagArgument( mesh_name_flag, 0, mesh_name );
		CHK_STAT_R				(result);
		dag_path				= get_path_by_name( mesh_name, result );
		CHK_STAT_R				(result);
	}else
	{
		MGlobal::displayError	("solid_visual_exporter: input mesh name flag (-inputmesh) not specified");
		return					MStatus::kFailure;
	}

	result						= writer.extract_render_geometry( dag_path );

	if( argData.isFlagSet( collision_name_flag ) )
	{
		MString					collision_name;
		result					= argData.getFlagArgument( collision_name_flag, 0, collision_name );
		CHK_STAT_R				(result);
		dag_path				= get_path_by_name( collision_name, result );
		CHK_STAT_R				(result);
		result					= writer.extract_collision_geometry( dag_path );
	}else
	{
		MGlobal::displayWarning( "building collision geometry by render surfaces" );
		result					= writer.build_collision_geometry_by_render( );
	}


	xray::memory::writer w_render( &xray::maya::g_allocator );

	if( result )
		result					= writer.write_render_geometry( w_render );

	xray::memory::writer w_collision( &xray::maya::g_allocator );
	if( result )
		result					= writer.write_collision_geometry( w_collision );
	
	xray::fs::make_dir_r		( fileName.asChar(), true );
	MString render_file_name	= fileName;
	render_file_name			+= "/geometry.ogf";
	w_render.save_to			( render_file_name.asChar() );

	MString collision_file_name	= fileName;
	collision_file_name			+= "/collision.ogf";
	w_collision.save_to			( collision_file_name.asChar() );

	fs::file_system		fat;

	fs::path_string				directory;
	directory					= fs::file_name_from_path( fileName.asChar() );
	fat.mount_disk				(directory.c_str(), render_file_name.asChar(), fs::file_system::watch_directory_false);
	fat.mount_disk				(directory.c_str(), collision_file_name.asChar(), fs::file_system::watch_directory_false);

	fixed_vector<fs::fat_inline_data::item, 1>	items;
	fs::fat_inline_data			inline_data(& items);
	fat.save_db					("e:/andy.db", "e:/andy.db", false, 512, &xray::maya::g_allocator, NULL, 0, xray::fs::file_system::db_target_platform_pc, inline_data, NULL);

	if( !result )
		MGlobal::displayError	("solid_visual_exporter: Export failed");
	else
		MGlobal::displayInfo	("solid_visual_exporter: Export to " + fileName + " successful!");

	return						result;
}
