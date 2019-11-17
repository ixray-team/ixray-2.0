////////////////////////////////////////////////////////////////////////////
//	Created		: 05.05.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef ANIM_EXPORT_UTIL_H_INCLUDED
#define ANIM_EXPORT_UTIL_H_INCLUDED
class MDagPath;
class MObject;
namespace xray {
namespace maya_animation {


struct EtCurve;
}}

struct EtReadKey 
{
	float			time;
	float			value;
	MFnAnimCurve::TangentType	inTangentType;
	MFnAnimCurve::TangentType	outTangentType;
	float			inAngle;
	float			inWeightX;
	float			inWeightY;
	float			outAngle;
	float			outWeightX;
	float			outWeightY;
	struct EtReadKey *	next;
}; // struct EtReadKey 

class anim_exporter : public MPxFileTranslator 
{
public:
						anim_exporter	();
	virtual				~anim_exporter	();

public:
	virtual MStatus 	writer				(const MFileObject &file, const MString &optionsString, FileAccessMode mode);

public:
	virtual bool		haveWriteMethod		() const;
	virtual MString 	defaultExtension	() const;
	virtual MFileKind	identifyFile		(const MFileObject &, const char *buffer, short size) const;

	static void *		creator				();
}; // class anim_exporter

#endif // #ifndef ANIM_EXPORT_UTIL_H_INCLUDED
