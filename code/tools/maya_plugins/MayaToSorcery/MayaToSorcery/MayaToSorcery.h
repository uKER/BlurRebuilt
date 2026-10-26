///////////////////////////////////////////////////////////////////////////////
// Copyright © Bizarre Creations 2006
//
// File:        MayaToSorcery.h
//
// Created:     07/11/06
// Author:      Michael Bibby
///////////////////////////////////////////////////////////////////////////////

#ifndef _MayaToSorcery
#define _MayaToSorcery

#pragma warning(disable:4355)

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_

#include <winsock2.h>
#include <windows.h>

#include <assert.h>

#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MDGModifier.h>
#include <maya/MTime.h>
#include <maya/MFnDagNode.h>
#include <maya/MArgDatabase.h>
#include <maya/MStringArray.h>
#include <maya/MDagPath.h>
#include <maya/MItDag.h>
#include <maya/MDagPathArray.h>
#include <maya/MCommandResult.h>
#include <maya/MPlug.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnTransform.h>
#include <maya/MMatrix.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <xt/XtTool.h>
#include <svn_wrapper.h>
#include <System/Artery/Artery.h>
#include <xbdm.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>

const float	WIDTH_INCREMENT  = 200;
const float	HEIGHT_INCREMENT = 60;

//=======================================
// SvnXtStorage class
//=======================================
class NullStorage : public Xt::Storage
{
public:
	virtual bool Start() { return true; }
	virtual bool New(const char* filename) { return true; }
	virtual bool Dirty(const char* filename) { return true; }
	virtual bool Rename(const char* oldFilename, const char* newFilename) { return true; }
	virtual bool Remove(const char* filename) { return true; }
	virtual bool IsModified(const char* filename) { return true; }
	virtual bool Stop() { return true; }
};

//=======================================
// MtsStatus class
//=======================================
class MtsStatus : public Xt::StatusCallback
{
	Xt::StatusMessage::Severity lastSev;

public:
	MtsStatus() : lastSev(Xt::StatusMessage::kInfo) { }

	virtual void Status(const Xt::StatusMessage& msg)
	{
		switch(msg.severity)
		{
			case Xt::StatusMessage::kInfo: MGlobal::displayInfo(MString("XtStatus: " + msg.message)); break;
			case Xt::StatusMessage::kWarning: MGlobal::displayWarning(MString("XtStatus: " + msg.message)); break;
			case Xt::StatusMessage::kError: MGlobal::displayError(MString("XtStatus: " + msg.message)); break;

			case Xt::StatusMessage::kTrace:
				switch(lastSev)
				{
					case Xt::StatusMessage::kInfo: MGlobal::displayInfo(MString("XtStatus: " + msg.message)); break;
					case Xt::StatusMessage::kWarning: MGlobal::displayWarning(MString("XtStatus: " + msg.message)); break;
					case Xt::StatusMessage::kError: MGlobal::displayError(MString("XtStatus: " + msg.message)); break;
				}
			break;
		}

		if (msg.severity != Xt::StatusMessage::kTrace)
			lastSev = msg.severity;
	}
};

//=======================================
// MayaToSorcery class
//=======================================
class MayaToSorcery : MPxCommand {

public:

	MayaToSorcery();
	~MayaToSorcery();

	virtual MStatus		doIt ( const MArgList& );

	static void*		creator() { return new MayaToSorcery; }
	static MSyntax		newSyntax();

	static MString		typeName;

private:

	// Holder for global database
	Xt::DomDb			m_database;
	
	// Holder for storage required by Xt
	NullStorage			m_storage;
	
	// Holder for the pack name
	XtString			m_pack;

	// Holder for the location of the sorcery database
	MString				m_databasePath;

	// Holder for the record ID if it is required
	XtId				m_id;

	// Holder for ID to be sent to game
	XtId				m_networkSendId;

	// Holder for the current user project
	MString				m_currentProject;

	// Holders for the type names
	XtString			m_recordTypeName;
	XtString			m_damageLevelName;
	XtString			m_elementName;
	XtString			m_objectName;

	// Holders for the base arrays
	Xt::DomArray*		m_damageLevelsArray;
	Xt::DomArray*		m_elementsArray;
	Xt::DomArray*		m_objectsArray;

	// Holders for the position of the nodes in node graph
	float				m_xPos;
	float				m_yPos;

private:

	// Open the database
	MStatus				openDatabase();
	
	// Close the database if its open
	void				closeDatabase();
	
	// Find the given pack file
	Xt::DomPack*		findPack(const char* name, MStatus *stat = NULL);
	
	// Get the current schema
	LarvaSchema*		getSchema();

	// See if the given record exists in the database
	bool				recordExists(const char* name);

	// Return the given record if it exists
	Xt::DomRecord*		getRecord(const char* name, const char* type);

	// Return the given record from the given ID if it exists
	Xt::DomRecord*		getRecord(XtId id);

	// Create a new record and return a pointer to it
	Xt::DomRecord*		createNewRecord(const char* name);

	// Add a field to the given block
	Xt::DomField*		addField(Xt::DomBlock* block, const char* key, const char* value);

	// Add a component to the given array
	Xt::DomComponent*	addComponent(Xt::DomArray* domArray);

	// Add a ref to the given array
	Xt::DomRef*			addRef(Xt::DomArray* domArray, XtId id);

	// Add a ref to the given block with the given name
	Xt::DomRef*			addRef(Xt::DomBlock* domBlock, const char* key, XtId id);

	// Add a component to the given array with the given type
	Xt::DomComponent*	addComponent(Xt::DomArray* domArray, XtString type);

	// Get a list of xtDefinitions for the the given type
	MStatus				getDefinitionList(MStringArray *list, MString xtField, bool includeId);
	
	// Create a dynamic object from the current scene hierarchy and import it into sorcery
	MStatus				addPhysicalObject(MString recordName);

	// Recursively create a phsyicalObject type in the Xt database
	MStatus				recursePhysicalObject(MDagPath object, MString objectName, Xt::DomArray* parentArray);

	// Recursively create a damage level
	MStatus				recurseDamageLevel(MDagPath object, Xt::DomArray* currentArray);

	// Recursively create a a child
	MStatus				recurseChildren(MDagPath object, Xt::DomArray* currentArray);

	// Return a list of child objects
	MStatus				getObjects(MDagPath object, MDagPathArray* objects);

	// Return a list of replacement objects
	MStatus				getReplacements(MDagPath object, MDagPathArray* replacements);

	// Return a list of effect objects
	MStatus				getEffects(MDagPath object, MDagPathArray* effects);

	// Check if the object is a game object
	bool				isObject(MDagPath object);

	// Check if the object is a damage level
	bool				isDamageLevel(MDagPath object);

	// Check if the object is a replacement element
	bool				isReplacement(MDagPath object);

	// Check if the object is an effect
	bool				isEffect(MDagPath object);

	// Check if the object is a debris element
	bool				isDebris(MDagPath object);

	// Check if the given string shoul dbe treated as an ID
	bool				treatAsHexID(MString value, XtId &id);

	// Set the node graph position parameters of the given node
	void				setNodeGraphPosition(Xt::DomComponent* component, bool incrementX = false, bool incrementY = false);

	// Save the given data on the given object with the given name
	MStatus				saveDataOnObject(XtId id, MString name, MString shortName, MDagPath object);

	// Remove all parts of the name before the final item name
	MStatus				stripObjectPath(MString* objectName);

	// Find the component in the array matching the given id
	Xt::DomComponent*	findComponentMatchingID(XtId id, Xt::DomArray* theArray);

	// Update the give record on the devkit via an artery connection
	MStatus				sendRecordToGame(XtId recordId);

	// Find all attrbutes on the given node which match the xt_ convension
	MStatus				setNodeXtAttributes(MDagPath node, Xt::DomComponent* component);

	// Send the given records to the game
	void				SendNetworkSub(ArteryConnection con, Xt::DomDb* db, Xt::DomRecord** records, int count);

friend void ConProc(ArteryConnection con, void* context);
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

#define CHECKSTATUS(stat, msg)								\
	if(!stat) {												\
		reportError(msg);									\
		return MS::kFailure;								\
	}

#define CHECK_STATUS_RETURN(stat, ptr, obj)					\
	if (!stat) {											\
		*ptr = stat;										\
		return obj;											\
	}

#endif