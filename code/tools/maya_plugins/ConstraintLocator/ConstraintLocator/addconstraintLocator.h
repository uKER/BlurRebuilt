// Copyright © Bizarre Creations 2008
//
// File:        AddConstraintLocator.h
//
// Created:     22/05/08
// Author:      Michael Bibby
///////////////////////////////////////////////////////////////////////////////

#ifndef _AddConstraintLocator
#define _AddConstraintLocator

#include "ConstraintLocator.h"

#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MDagModifier.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMatrix.h>
#include <maya/MFnDagNode.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPlug.h>

//=======================================
// AddConstraintLocator class
//=======================================

class AddConstraintLocator : MPxCommand {

public:

	 AddConstraintLocator();
	~AddConstraintLocator();

	virtual bool		isUndoable() const { return true; }
	virtual MStatus		doIt ( const MArgList& );
	virtual MStatus		redoIt ();
	virtual MStatus		undoIt ();

	static void*		creator() { return new AddConstraintLocator; }
	static MSyntax		newSyntax();

	static MString		typeName;
	static MString		axis;
private:

	MObject object1Mesh;
	MObject object2Mesh;

	MDagModifier dagMod;
	MDGModifier  dgMod;
};


//=============================================
//  Define the Error checking Macros
//=============================================

inline MString errorFormat( const MString &msg, 
						    const MString &sourceFile,
							const int &sourceLine ) {

	MString txt( msg );
			txt += ", File: ";
			txt += sourceFile;
			txt += " at Line: ";
			txt += sourceLine;
			return txt;

}

#define reportError(msg) {									\
	MString __txt = errorFormat(msg, __FILE__, __LINE__);	\
	MGlobal::displayError(__txt);							\
}

#define CHECK_STATUS(stat, msg)								\
	if(!stat) {												\
		reportError(msg);									\
		return MS::kFailure;								\
	}

#define CHECKOBJECT(obj, msg)								\
	if( obj.isNull() ) {									\
		reportError(msg);									\
		return MS::kFailure;								\
	}

#endif
