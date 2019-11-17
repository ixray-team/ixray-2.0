////////////////////////////////////////////////////////////////////////////
//	Created		: 05.02.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "skeleton_export_util.h"
#include <xray/configs.h>

skeleton_exporter::skeleton_exporter()
{

}

skeleton_exporter::~skeleton_exporter()
{

}

void * skeleton_exporter::creator ()
{	
	return CRT_NEW(skeleton_exporter); 
}

bool skeleton_exporter::haveWriteMethod () const
{	return (true);}

MString skeleton_exporter::defaultExtension () const
{	return (MString("lua"));}


MPxFileTranslator::MFileKind skeleton_exporter::identifyFile (	const MFileObject &file,
																const char * buffer,
																short size) const
{
	XRAY_UNREFERENCED_PARAMETER( size );
	XRAY_UNREFERENCED_PARAMETER( buffer );

	const char *name		= file.name().asChar();
	int   nameLength		= (int)strlen(name);

	if ((nameLength > 5) && !strcasecmp(name+nameLength-6, ".lua")) {
		return (kIsMyFileType);
	}

	return (kNotMyFileType);
}

MStatus		export_skeleton_node( MFnIkJoint  &ik_anim_joint, xray::configs::lua_config_value &cfg, const  MSelectionList &all_claster_joints )
{
	MStatus stat;
	pcstr name = ik_anim_joint.name(&stat).asChar();CHK_STAT_R( stat )

	
	u32 ch_cnt = ik_anim_joint.childCount( &stat );CHK_STAT( stat );
	//if( ch_cnt==0 )
	xray::configs::lua_config_value my_cfg = cfg; 

	MObject		my_obj 	= ik_anim_joint.object( &stat );
	MDagPath	my_path;
	CHK_STAT_R( ik_anim_joint.getPath( my_path ) );

	if( all_claster_joints.hasItem( my_path, my_obj,  &stat ) )
	{
		CHK_STAT( stat );
		my_cfg = cfg[ name ];
		my_cfg.create_table();
	} else
		return stat;

	
	for(u32 i = 0 ; i < ch_cnt ; ++i )
	{
		MObject	 obj = ik_anim_joint.child( i, &stat );CHK_STAT( stat );
		if( !obj.hasFn( MFn::kJoint ) )
			continue;
		MFnIkJoint ik_anim_joint_child( obj, &stat );CHK_STAT_R( stat );
		
		xray::configs::lua_config_value temp_cfg = my_cfg;
		CHK_STAT_R( export_skeleton_node( ik_anim_joint_child, temp_cfg, all_claster_joints ) );
	}
	return stat;

}

MStatus		export_skeleton( MSelectionList &all_claster_joints, xray::configs::lua_config_value &cfg  )
{
	MStatus stat;
	
	MObject			anim_joint;
	CHK_STAT_R( all_claster_joints.getDependNode		(0, anim_joint) );// get root

	ASSERT( anim_joint.hasFn( MFn::kJoint ) );
	MFnIkJoint ik_anim_joint_root( anim_joint, &stat );
	CHK_STAT_R( stat );
	u32 pcnt = ik_anim_joint_root.parentCount( &stat );CHK_STAT( stat );

	R_ASSERT_U( pcnt==0 ,"not a root");

	xray::configs::lua_config_value object_movenent_cfg = cfg; 
	object_movenent_cfg = cfg[ object_mover_bone_name ];
	object_movenent_cfg.create_table();

	export_skeleton_node( ik_anim_joint_root, object_movenent_cfg, all_claster_joints );
	return stat;

}


MStatus skeleton_exporter::writer (const MFileObject &file, const MString& options, FileAccessMode mode)
{
	XRAY_UNREFERENCED_PARAMETER( mode );
	XRAY_UNREFERENCED_PARAMETER( options );


	MStringArray					all_joints_str;
	MGlobal::executeCommand			("skinCluster -q -inf", all_joints_str);
	

	MSelectionList all_joints;
	int length						= all_joints_str.length();
	
	MString str = "found ";	str += length;	str += " animation joints";
	MGlobal::displayInfo(str);
	
	for(int i=0; i<length; ++i)
		all_joints.add( all_joints_str[i] );

	//if( length )
	//	all_joints.add( all_joints_str[0] );
	//else
	//	return MStatus::kFailure;

	xray::configs::lua_config_ptr		cfg		= xray::configs::create_lua_config();
	xray::configs::lua_config_value		v_cfg  = *cfg;

	if( export_skeleton( all_joints, v_cfg ) )
		cfg->save_as( file.fullName().asChar() );
	return MStatus::kSuccess;
}