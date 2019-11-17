#include "pch.h"

#include "anim_text_file_export.h"
#include "anim_export_util.h"
#include "maya_memory.h"

#include <xray/configs.h>
#include <xray/animation/world.h>
#include <xray/animation/api.h>
#include <xray/animation/i_skeleton_animation_data.h>
#include <xray/animation/i_bone_names.h>

#include <xray/maya_animation/discrete_data.h>
#include <xray/maya_animation/world.h>
#include <xray/maya_animation/api.h>
#include <xray/maya_animation/raw_bi_spline.h>
#include <xray/maya_animation/anim_track.h>
#include <xray/maya_animation/data_header.h>
#include "maya_engine.h"

namespace xray {
namespace core {
namespace configs {

xray::configs::lua_config* create_lua_config	( pcstr file_name, memory::reader& reader );

} // namespace configs
} // namespace core
} // namespace xray

using	namespace xray::animation;
using	namespace xray::maya_animation;

using	xray::maya_animation::discrete_data;
using	xray::maya::maya_engine;
using	xray::maya::g_maya_engine;

int assembleAnimCurve		(EtReadKey* firstKey, int numKeys, bool isWeighted, EtCurve* src_and_dest);

MStatus 	export_animated_plugs	( anim_track& track, const MPlugArray &animatedPlugs );
MStatus 	export_animated_plugs	( MObject joint, FILE* f );
MStatus 	export_animated_plugs	( MSelectionList &all_joints , FILE* f );
MStatus 	export_frames			( MSelectionList &all_joints , FILE* f );
MStatus		export_bi_splines		( MSelectionList &all_joints, xray::configs::lua_config_value & ,xray::configs::lua_config_value &  );

anim_exporter::anim_exporter()
{}

anim_exporter::~anim_exporter()
{}

MStatus anim_exporter::writer ( const MFileObject &file, const MString& options, FileAccessMode mode )
{
	XRAY_UNREFERENCED_PARAMETER( mode );
	XRAY_UNREFERENCED_PARAMETER( options );
/*
	MSelectionList					lst_;
	MGlobal::getActiveSelectionList (lst_);

	MPlugArray						xf_animatedPlugs;
	MObject							node;
	if(lst_.length()!=1)
	{
		MGlobal::displayError("Select one object to export animation pls");
		return MStatus::kFailure;
	}
	lst_.getDependNode				(0, node);
	MAnimUtil::findAnimatedPlugs	(node, xf_animatedPlugs);
*/
	MStringArray					all_joints_str;
	MGlobal::executeCommand			("skinCluster -q -inf", all_joints_str);
	MSelectionList all_joints;
	int length						= all_joints_str.length();
	
	MString str = "found ";	str += length;	str += " animation joints";
	MGlobal::displayInfo(str);
	
	for(int i=0; i<length; ++i)
		all_joints.add( all_joints_str[i] );

	//FILE*		f;
	//int res		= fopen_s(&f, file.fullName().asChar(), "wb");
	//

	//if(res==0)
	//{

	//	//CHK_STAT( export_animated_plugs( all_joints, f ) );
	//	//CHK_STAT( export_frames( all_joints, f ) );
	//	
	//	fclose								(f);
	//}

	xray::configs::lua_config_ptr cfg_splines		= xray::configs::create_lua_config();
	xray::configs::lua_config_value  v_cfg_splines  = *cfg_splines;

	xray::configs::lua_config_ptr	cfg_check_data		= xray::configs::create_lua_config();
	xray::configs::lua_config_value  v_cfg_check_data  = *cfg_check_data;

	export_bi_splines( all_joints, v_cfg_splines, v_cfg_check_data );
	cfg_splines->save_as( file.fullName().asChar() );
	cfg_check_data->save_as( ( file.fullName() + "_check_data" ).asChar() );

	return	(MS::kSuccess);
}

bool anim_exporter::haveWriteMethod () const
{	return (true);}

MString anim_exporter::defaultExtension () const
{	return (MString("lua"));}

MPxFileTranslator::MFileKind anim_exporter::identifyFile (	const MFileObject &file,
																const char * buffer,
																short size) const
{
	XRAY_UNREFERENCED_PARAMETER( size );
	XRAY_UNREFERENCED_PARAMETER( buffer );
	const char *name		= file.name().asChar();
	int   nameLength		= (int)strlen(name);

	if ((nameLength > 5) && !strcasecmp(name+nameLength-6, ".clip")) {
		return (kIsMyFileType);
	}

	return (kNotMyFileType);
}

void * anim_exporter::creator ()
{	
	return CRT_NEW(anim_exporter); 
}
	//\li matrix = [S] * [RO] * [R] * [JO] * [IS] * [T]


	//(where '*' denotes matrix multiplication).

	//These matrices are defined as follows:


	//\li <b>[S]</b> : scale
	//\li <b>[RO]</b> : rotateOrient (attribute name is rotateAxis)
	//\li <b>[R]</b> : rotate
	//\li <b>[JO]</b> : jointOrient
	//\li <b>[IS]</b> : parentScaleInverse
	//\li <b>[T]</b> : translate

//The methods to get the value of these matrices are:
//	\li <b>[S]</b> : getScale
//	\li <b>[RO]</b> : getScaleOrientation
//	\li <b>[R]</b> : getRotation
//	\li <b>[JO]</b> : getOrientation
//	\li <b>[IS]</b> : (the inverse of the getScale on the parent transformation matrix)
//	\li <b>[T]</b> : translation

static const int max_frame = 500;
//MStatus export_transform(lua_config_value table, MFnTransform& node_tr )

void get_joint_params(const MFnIkJoint &jnt, MEulerRotation &rot, MVector &translation, double scale[3] )
{
	MStatus status;

	MEulerRotation R;
	MQuaternion RO;
	MEulerRotation JO;
	CHK_STAT( jnt.getScaleOrientation( RO ) );
	CHK_STAT( jnt.getOrientation( JO ) );
	CHK_STAT( jnt.getRotation( R ) );
	rot = RO.asEulerRotation()  * R * JO;
	MEulerRotation::RotationOrder ro = MEulerRotation::kXYZ ;

	ASSERT_U( rot.order == ro );
	translation = jnt.translation( MSpace::kTransform, &status ); CHK_STAT( status );

	
	jnt.getScale( scale );
}

void get_root_joint_params( MFnIkJoint &jnt, MEulerRotation &rot, MVector &translation, double scale[3] )
{
	MStatus status;
	MDagPath	my_path;
	CHK_STAT( jnt.getPath( my_path ) );
	MMatrix  m = my_path.inclusiveMatrix(&status); CHK_STAT( status );
	MTransformationMatrix transformation( m );
	ASSERT( transformation.rotationOrder( &status ) == MTransformationMatrix::kXYZ );

	//transformation.reorderRotation( MTransformationMatrix::kXYZ ) 
	rot = transformation.eulerRotation();

	ASSERT( rot.order == MEulerRotation::kXYZ );
	//rot.reorderIt( MEulerRotation::kXYZ );

	translation = transformation.getTranslation( MSpace::kTransform, &status );
	CHK_STAT( transformation.getScale( scale, MSpace::kTransform ) );

}

void joint_params_to_frame( const MEulerRotation &rot, const MVector &translation_, const double scale[3], frame &frm )
{
	MDistance temp; temp.setUnit( MDistance::internalUnit() );
	MVector translation = translation_;
	temp.setValue( translation.x ); translation.x = temp.asMeters();
	temp.setValue( translation.y ); translation.y = temp.asMeters();
	temp.setValue( translation.z ); translation.z = -temp.asMeters();

	frm.rotation = xray::float3( float( rot.x ),float( rot.y ),float( -rot.z ));

	frm.translation = xray::float3(float( translation.x ),float( translation.y ),float( translation.z ));
	frm.scale = xray::float3(float( scale[0] ),float( scale[1] ),float( scale[2] ));


	ASSERT( xray::math::valid( frm.translation.x ) );
	ASSERT( xray::math::valid( frm.translation.y ) );
	ASSERT( xray::math::valid( frm.translation.z ) );
	ASSERT( xray::math::valid( frm.rotation.x ) );
	ASSERT( xray::math::valid( frm.rotation.y ) );
	ASSERT( xray::math::valid( frm.rotation.z ) );
	ASSERT( xray::math::valid( frm.scale.x ) );
	ASSERT( xray::math::valid( frm.scale.y ) );
	ASSERT( xray::math::valid( frm.scale.z ) );
}

void get_root_frame( MFnIkJoint &jnt, frame &frm )
{
		MStatus status;
		MEulerRotation rot;
		MVector translation;
		double scale[3];
		get_root_joint_params( jnt, rot, translation, scale );
		joint_params_to_frame( rot, translation, scale, frm );
}



void get_frame(const MFnIkJoint &jnt, frame &frm )
{


	MEulerRotation rot;
	MVector translation;
	double scale[3];

	get_joint_params( jnt, rot, translation, scale );
	joint_params_to_frame( rot, translation, scale, frm );


}

bool has_parent_joint( const MFnIkJoint &jnt )
{
	MStatus	status;
	const u32 pcnt = jnt.parentCount( &status  ); CHK_STAT( status );
	for( u32 i = 0; i < pcnt; ++i )
	{
		MObject p_obj = jnt.parent( i, &status ); CHK_STAT( status );

		if( p_obj.hasFn( MFn::kJoint ) )
			return true;
	}
	return false;
}

bool is_object_mover( const MFnIkJoint &jnt )
{
	
	return ! has_parent_joint( jnt );

	//MStatus	status;
	
	
	//bool ret = jnt.name() == MString( "Hips" );

	//return ret;
}

u32 object_mover_id( const MSelectionList &all_joints )
{
	MStatus	status;
	
	u32 ret = u32(-1);

	int length = all_joints.length		();
	for( int i=0; i<length; ++i )
	{
		MObject		anim_joint;
		status = all_joints.getDependNode( i, anim_joint ); CHK_STAT( status );
		MFnIkJoint jnt( anim_joint, &status ); CHK_STAT( status );
		if( is_object_mover( jnt ) )
		{
			R_ASSERT_U( ret == u32(-1), "object_mover ambiguous" );
			ret = i;
		}
	}

	R_ASSERT_U( ret != u32(-1), "can not define object_mover" );
	return ret;
}

MStatus write_discrete_data( discrete_data &data, const MSelectionList &all_joints )
{
	MStatus	status;	
	
	int length = all_joints.length		( &status ); CHK_STAT( status );

	if( length == 0 )
		return MStatus::kFailure; 
	

	// Remember the frame the scene was at so we can restore it later.
	MTime currentFrame	= MAnimControl::currentTime();
	MTime startFrame	= MAnimControl::minTime();
	MTime endFrame		= MAnimControl::maxTime();

	int frameFirst			= (int) startFrame.as( MTime::uiUnit() );
	int frameLast			= (int) endFrame.as( MTime::uiUnit() );
	MTime	tmNew; tmNew.setUnit		(MTime::uiUnit());

	if (currentFrame.unit()!=MTime::kNTSCFrame){
		MGlobal::displayError("!Can't export animation with FPS!=30.f") ;
		return MStatus::kFailure;
	}

	data.set_bone_count( length + 1 );
	xray::math::clamp( frameLast, 0, max_frame );
	
	const u32 id_object_mover = object_mover_id( all_joints );
	ASSERT( id_object_mover < u32( length + 1 ) );
	for ( int j=frameFirst; j<=frameLast; j++ )//frameLast
	{
		tmNew.setValue ( j );
		MGlobal::viewFrame( tmNew );
		for( int i=0; i<length; ++i )
		{
			MObject							anim_joint;
			all_joints.getDependNode		( i, anim_joint );
			
			MFnIkJoint jnt( anim_joint, &status ); CHK_STAT( status );
			frame frm;
			if( id_object_mover == u32( i ) )
			{
				get_root_frame( jnt, frm );
				data.add_frame( 0, frm );

				data.add_frame( i + 1, zero );
				continue;
			}

			get_frame( jnt, frm );
			data.add_frame( i + 1, frm );
			
		}

	}
	data.calculate();

	return MStatus::kSuccess;
}

#include <io.h>

using xray::animation::enum_channel_id;
MStatus		export_bi_splines( MSelectionList &all_joints, xray::configs::lua_config_value &cfg_splines, xray::configs::lua_config_value &cfg_check_data  )
{

	MStatus	status;
	ASSERT( g_maya_engine.maya_animation_world );
	ASSERT( g_maya_engine.animation_world );
	discrete_data *data = g_maya_engine.maya_animation_world->create_discrete_data();

	write_discrete_data( *data, all_joints );


	xray::animation::i_skeleton_animation_data *skel = g_maya_engine.animation_world->create_skeleton_animation_data( 0 );

	
	{
		xray::configs::lua_config_value tmp = cfg_check_data["discrete_data"] ;
		data->save( tmp );
	}
//	{
//		using xray::configs::lua_config_ptr;
//		lua_config_ptr temp_config	= xray::configs::create_lua_config();
//		*temp_config				= v;
//
//		pcstr const file_name		= "d:/temp.lua";
//		temp_config->save_as		( file_name );
//
//		FILE* f						= fopen(file_name, "rb" );
//		u32 const file_size			= _filelength(_fileno(f));
//		pstr buffer					= (pstr)MALLOC( file_size + 1, "test temp animation file" );
//		fread						( buffer, file_size, 1, f );
//		fclose						( f );
//
//		buffer[file_size]			= 0;
//		lua_config_ptr const temp	= xray::configs::create_lua_config_from_string( buffer );
//		FREE						( buffer );
//
//		data->check					( (*temp)["discrete_data"] );
////		data->load					( (*temp)["discrete_data"] );
//	}

	g_maya_engine.maya_animation_world->build_animation_data( *data, *skel );
	

	
	int length = all_joints.length( );

	skel->get_bone_names()->set_bones_number( length + 1 );
	skel->get_bone_names()->set_name( 0, object_mover_bone_name );
	for( int i = 0; i< length; ++ i)
	{
			MObject							anim_joint;
			all_joints.getDependNode		( i  , anim_joint );
			MFnIkJoint jnt( anim_joint, &status ); CHK_STAT( status );
			skel->get_bone_names()->set_name( i + 1 , jnt.name().asChar() );
	}
	
	{
		xray::configs::lua_config_value tmp = cfg_splines["splines"];
		skel->save( tmp );
	}

	g_maya_engine.maya_animation_world->destroy( data );
	g_maya_engine.animation_world->destroy( skel );




	return status;
}

MStatus 	export_animated_plugs( MSelectionList &all_joints , FILE* f )
{

		int length = all_joints.length		();
		animation_curve_data_header			header;
		header.type = maya_spline;
		fwrite								(&header, sizeof(header), 1, f);
		fwrite								(&length, sizeof(length), 1, f);

		for(int i=0; i<length; ++i)
		{
			MObject							anim_joint;
			all_joints.getDependNode		(i, anim_joint);
			MDagPath pth;
			CHK_STAT_R( all_joints.getDagPath			(i, pth) );

			MGlobal::displayInfo(pth.fullPathName());
			CHK_STAT_R( export_animated_plugs( anim_joint, f ) ) ;

		}
	return MStatus::kSuccess;
}

MStatus 	export_animated_plugs(MObject anim_joint, FILE*	f )
{
	MPlugArray						xf_animatedPlugs;
	MAnimUtil::findAnimatedPlugs	(anim_joint, xf_animatedPlugs);

	MFnDependencyNode				dep_node(anim_joint);
	MString str = "joint ";	str += dep_node.name();	str += " has "; str += xf_animatedPlugs.length(); str += " animated plugs";
	MGlobal::displayInfo			(str);

	anim_track						track;

	CHK_STAT_R( export_animated_plugs			(track, xf_animatedPlugs) );
	track.save						(f);
	xf_animatedPlugs.clear			();
	return MStatus::kSuccess;
}

#define kDegRad 0.0174532925199432958f

MStatus 	export_animated_plugs(anim_track& track, const MPlugArray &animatedPlugs)
{
	unsigned int numPlugs		= animatedPlugs.length();
	for (unsigned int i = 0; i < numPlugs; i++) 
	{

		MPlug plug				= animatedPlugs[i];
		MObjectArray			animation;

		if (!MAnimUtil::findAnimation (plug, animation))	
		{
			MString str = "  plug ";	str += plug.name();	str += " has no animation";
			MGlobal::displayInfo			(str);
			
			continue;
		}

		MObject attrObj			= plug.attribute();
		MFnAttribute fnAttr		(attrObj);

		MObject animCurveNode = animation[0];
		if (!animCurveNode.hasFn (MFn::kAnimCurve))
		{
			MGlobal::displayError("error: !animCurveNode.hasFn");
			return MStatus::kFailure;
		}
		MFnAnimCurve manim		(animCurveNode);

		const char* att_name	= fnAttr.name().asChar();
		enum_channel_id ch_id	= track.get_channel_id(att_name);
		if(ch_id == channel_unknown)
		{
			MString str =					"Ignoring animated attribute [";	
			str								+= att_name;
			str								+= "]";
			MGlobal::displayInfo			(str);
			continue;
		}
		MString str =					"Exporting animated attribute [";	
		str								+= att_name;
		str								+= "]";
		MGlobal::displayInfo			(str);

		EtCurve*	curve		= track.get_channel(ch_id, true);

//--------
		float angularConversion		= 1.0f;
		float frameRate				= 1.0;
		{
			//calcs angular conversion
			MAngle::Unit ut = MAngle::uiUnit();
			switch(ut)
			{
			case MAngle::kRadians:
				angularConversion		= 1.0f;
				break;
			case MAngle::kDegrees:
				angularConversion		= kDegRad;
				break;
			default:
				MGlobal::displayError("unknown angle units");
			}

		 //calc frame rate
			MTime						t;
			t.setValue					(1.0f);
			frameRate					= 1.0f / (float)t.as(MTime::kSeconds);
		}
		EtReadKey*	firstKey			= 0;
		EtReadKey*	lastKey				= 0;
		EtReadKey*	readKey				= 0;

//		bool endOfCurve					= false;
		bool isWeighted					= manim.isWeighted();
		int numKeys						= manim.numKeys();
		float unitConversion			= 1.0;

		if(manim.animCurveType()==MFnAnimCurve::kAnimCurveTA || manim.animCurveType()==MFnAnimCurve::kAnimCurveUA)
			unitConversion	= angularConversion;

		for(int kidx=0; kidx<numKeys; ++kidx)
		{
			
			readKey					= ALLOC(EtReadKey,1);
			readKey->time			= (float)manim.time(kidx).value() / frameRate;
			readKey->value			= (float)manim.value(kidx) * unitConversion;
			readKey->inTangentType	= manim.inTangentType(kidx);
			readKey->outTangentType = manim.outTangentType(kidx);

			if (readKey->inTangentType == MFnAnimCurve::kTangentFixed) 
			{
				MAngle angle;
				double weight;
				manim.getTangent	(kidx, angle, weight, true);

				readKey->inAngle	= (float)angle.value() * angularConversion;
				readKey->inWeightX	= (float)weight / frameRate; 
				readKey->inWeightY	= (float)weight; 
			}

			if (readKey->outTangentType == MFnAnimCurve::kTangentFixed) 
			{
				MAngle angle;
				double weight;

				manim.getTangent	(kidx, angle, weight, false);
				readKey->outAngle	= (float)angle.value() * angularConversion;
				readKey->outWeightX = (float)weight / frameRate; 
				readKey->outWeightY = (float)weight; 
			}
			
			readKey->next			= 0;

			if (firstKey == 0) 
			{
				firstKey			= readKey;
				lastKey				= firstKey;
			}else 
			{
				lastKey->next		= readKey;
				lastKey				= readKey;
			}
		} // collect keys
//		int res					= 
			assembleAnimCurve(firstKey, numKeys, isWeighted, curve);

		curve->preInfinity		= (EtInfinityType)manim.preInfinityType();
		curve->postInfinity		= (EtInfinityType)manim.postInfinityType();
	}
	return MStatus::kSuccess;
}