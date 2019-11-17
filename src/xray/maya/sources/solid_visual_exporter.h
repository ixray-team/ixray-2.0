////////////////////////////////////////////////////////////////////////////
//	Created		: 06.05.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef SOLID_VISUAL_EXPORTER_H_INCLUDED
#define SOLID_VISUAL_EXPORTER_H_INCLUDED

class solid_visual_exporter : public MPxCommand
{
public:
	static const MString			Name;

public:
	virtual MStatus			doIt		( const MArgList& );
	virtual bool			isUndoable	( ) const		{ return false; } 

	static void*			creator		( );
	static MSyntax			newSyntax	( );

private:
	MStatus				export_dag_path		( MString const& filename, MDagPath& dagpath, MArgDatabase const& options );
}; // class solid_visual_exporter

#endif // #ifndef SOLID_VISUAL_EXPORTER_H_INCLUDED