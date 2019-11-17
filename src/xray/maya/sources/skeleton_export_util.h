////////////////////////////////////////////////////////////////////////////
//	Created		: 05.02.2010
//	Author		: Konstantin Slipchenko
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef SKELETON_EXPORT_UTIL_H_INCLUDED
#define SKELETON_EXPORT_UTIL_H_INCLUDED

class skeleton_exporter : public MPxFileTranslator 
{
public:
						skeleton_exporter		();
	virtual				~skeleton_exporter		(); 

	virtual MStatus 	writer				(const MFileObject &file, const MString &optionsString, FileAccessMode mode);

public:
	virtual bool		haveWriteMethod		() const;
	virtual MString 	defaultExtension	() const;
	virtual MFileKind	identifyFile		(const MFileObject &, const char *buffer, short size) const;

	static void *		creator				();
};

#endif // #ifndef SKELETON_EXPORT_UTIL_H_INCLUDED