////////////////////////////////////////////////////////////////////////////
//	Created		: 07.05.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "collision_node.h"
#include "create_collision_cmd.h"
#include "build_collision_cmd.h"
#include "body_node.h"
#include "body_node_manip.h"
#include "joint_node.h"
#include "anim_export_util.h"
#include "skeleton_export_util.h"
#include "create_body_cmd.h"
#include "create_joint_cmd.h"
#include "physics_exporter.h"
#include "solid_visual_exporter.h"
#include "fake_drawing_locator.h"
#include "xray_shader_node.h"
#include <xray/core/core.h>
#include <xray/command_line_extensions.h>
#include <xray/core/simple_engine.h>
#include "maya_engine.h"
#include <xray/maya_animation/world.h>
#include <xray/maya_animation/api.h>
#include <xray/os_include.h>
#include <xray/animation/api.h>
#include <xray/resources_fs.h>
#include <xray/core/sources/fs_file_system.h>

MCallbackIdArray callbackIds;

xray::maya::allocator_type			xray::maya::g_allocator;
xray::maya::crt_allocator_type		xray::maya::g_crt_allocator;
xray::maya::maya_engine				core_engine;
xray::maya::maya_engine&			xray::maya::g_maya_engine = core_engine;

void mount_func( bool success)
{
	R_ASSERT(success);
}


XRAY_DLL_EXPORT MStatus initializePlugin( MObject obj )
{
	xray::core::preinitialize	(
		& core_engine,
		"-fpe_disabled -log_to_stdout",
		xray::command_line::contains_application_false,
		"maya",
		__DATE__
	);

	xray::memory::g_crt_allocator	= &xray::maya::g_crt_allocator;
	
	LOG_INFO( "initialize xray maya plugin!" );

	xray::maya::g_allocator.do_register	( 50*1024*1024, "maya" );
	xray::memory::allocate_region		( );
	xray::core::initialize				( "maya plugin", xray::core::no_log, xray::core::perform_debug_initialization );

	MStatus				stat;
	
	xray::animation::memory_allocator( xray::maya::g_allocator );

	core_engine.animation_world = xray::animation::create_world( core_engine );
	ASSERT( core_engine.animation_world );
	
	xray::maya_animation::memory_allocator( xray::maya::g_allocator );
	core_engine.maya_animation_world = xray::maya_animation::create_world( core_engine );

	ASSERT( core_engine.maya_animation_world );

	MFnPlugin plugin	( obj, "GSC Game World", "1.0", "Any");

	build_sphere		(stat);
	if(!stat)			return stat;

	build_cylinder		(stat);
	if(!stat)			return stat;

	stat = plugin.registerTransform( collision_node::typeName,
										collision_node::typeId,
										&collision_node::creator, 
										&collision_node::initialize, 
										MPxTransformationMatrix::creator, 
										MPxTransformationMatrix::baseTransformationMatrixId);

	if (!stat)
		stat.perror("registerNode failed");


	stat = plugin.registerTransform(	body_node::typeName, 
										body_node::typeId, 
										&body_node::creator, 
										&body_node::initialize, 
										MPxTransformationMatrix::creator, 
										MPxTransformationMatrix::baseTransformationMatrixId);
	if (!stat)
		stat.perror("registerNode failed");
	stat = plugin.registerTransform( joint_node::typeName,
										joint_node::typeId,
										&joint_node::creator, 
										&joint_node::initialize, 
										MPxTransformationMatrix::creator, 
										MPxTransformationMatrix::baseTransformationMatrixId);

	if (!stat)
		stat.perror("registerNode failed");

/*
	stat = plugin.registerNode( body_node::typeName,
										body_node::typeId,
										&body_node::creator, 
										&body_node::initialize, 
										MPxNode::kLocatorNode );*/

	stat = plugin.registerNode(			fake_drawing_locator::typeName,
										fake_drawing_locator::typeId,
										&fake_drawing_locator::creator, 
										&fake_drawing_locator::initialize, 
										MPxNode::kLocatorNode );

	if (!stat)
		stat.perror("register body_node::typeName failed");

	stat = plugin.registerNode( body_node_manip::typeName,
										body_node_manip::typeId, 
										&body_node_manip::creator,
										&body_node_manip::initialize, 
										MPxNode::kManipContainer );

	if (!stat)
		stat.perror("registerNode failed");



	stat = plugin.registerFileTranslator("xray_anim_exporter", NULL, &anim_exporter::creator);
	if (!stat)
		stat.perror("registerFileTranslator failed");

	stat = plugin.registerCommand( "xray_body", createBodyCmd::creator, createBodyCmd::newSyntax );
	if (!stat)
		stat.perror("registerCommand failed");

	stat = plugin.registerCommand( createCollisionCmd::Name, createCollisionCmd::creator, createCollisionCmd::newSyntax );
	if (!stat)
		stat.perror("registerCommand failed");

	stat = plugin.registerCommand( createJointCmd::Name, createJointCmd::creator, createJointCmd::newSyntax );
	if (!stat)
		stat.perror("registerCommand failed");

	stat = plugin.registerCommand( build_collision_cmd::Name, build_collision_cmd::creator, build_collision_cmd::newSyntax );
	if (!stat)
		stat.perror("registerCommand failed");

	stat = plugin.registerFileTranslator("xray_ph_exporter", NULL, &physics_exporter::creator);
	if (!stat)
		stat.perror("registerFileTranslator failed");

	stat = plugin.registerFileTranslator("xray_skeleton_exporter", NULL, &skeleton_exporter::creator);
	if (!stat)
		stat.perror("registerFileTranslator failed");

	stat = plugin.registerCommand(solid_visual_exporter::Name, solid_visual_exporter::creator, solid_visual_exporter::newSyntax );
	if (!stat)
		stat.perror("registerCommand failed");

	const MString	UserClassify( "shader/surface" );

	MString command( "if( `window -exists createRenderNodeWindow` ) {refreshCreateRenderNodeWindow(\"" );

	stat = plugin.registerNode( xray_shader_node::typeName, 
								xray_shader_node::typeId, 
								xray_shader_node::creator, 
								xray_shader_node::initialize, 
								MPxNode::kDependNode,
								&UserClassify );

	if (!stat)
		stat.perror("registerNode failed");

   command += UserClassify;
   
   command += "\");}\n";

   MGlobal::executeCommand( command );


   xray::resources::query_mount_disk	("andy", 
								"e:/stk2/resources/library/static", 
								NULL,
								&xray::maya::g_allocator);

   xray::fs::set_allocator_thread_id	(xray::threading::current_thread_id());

	return stat;
}

XRAY_DLL_EXPORT MStatus uninitializePlugin( MObject obj)
{
	MStatus			stat;
	MFnPlugin		plugin( obj );

	stat = plugin.deregisterNode( collision_node::typeId );
	if(!stat) 
		stat.perror("deregisterNode failed");

	stat = plugin.deregisterNode( body_node::typeId );
	if(!stat) 
		stat.perror("deregisterNode failed");

	stat = plugin.deregisterNode( body_node_manip::typeId );
	if(!stat) 
		stat.perror("deregisterNode failed");

	stat = plugin.deregisterNode( fake_drawing_locator::typeId );
	if(!stat) 
		stat.perror("deregisterNode failed");

	stat = plugin.deregisterNode( joint_node::typeId );
	if(!stat) 
		stat.perror("deregisterNode failed");

	stat = plugin.deregisterFileTranslator("xray_anim_exporter");
	if (!stat)
		stat.perror("registerFileTranslator failed");

	stat = plugin.deregisterCommand( createBodyCmd::Name );
	if (!stat)
		stat.perror("deregisterCommand failed");

	stat = plugin.deregisterCommand( createCollisionCmd::Name );
	if (!stat)
		stat.perror("deregisterCommand failed");

	stat = plugin.deregisterCommand( createJointCmd::Name );
	if (!stat)
		stat.perror("deregisterCommand failed");
	
	stat = plugin.deregisterCommand( build_collision_cmd::Name );
	if (!stat)
		stat.perror("deregisterCommand failed");

	stat = plugin.deregisterFileTranslator("xray_ph_exporter");
	if (!stat)
		stat.perror("registerFileTranslator failed");


	stat = plugin.deregisterFileTranslator("xray_skeleton_exporter");
	if (!stat)
		stat.perror("registerFileTranslator failed");
	
	stat = plugin.deregisterCommand( solid_visual_exporter::Name );
	if (!stat)
		stat.perror("deregisterCommand failed");
	
	for (unsigned int i=0; i<callbackIds.length(); i++ ) {
		MMessage::removeCallback( (MCallbackId)callbackIds[i] );
	}
	
	const MString UserClassify( "shader/surface" );

	MString command( "if( `window -exists createRenderNodeWindow` ) {refreshCreateRenderNodeWindow(\"" );

	stat = plugin.deregisterNode( xray_shader_node::typeId );

	command += UserClassify;
	command += "\");}\n";

	MGlobal::executeCommand( command );

	xray::maya_animation::destroy_world( core_engine.maya_animation_world );
	xray::animation::destroy_world( core_engine.animation_world );

	return stat;
}