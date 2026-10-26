///////////////////////////////////////////////////////////////////////////////
// Copyright © Bizarre Creations 2008
//
// File:        AddConstraintLocator.cpp
//
// Created:     29/05/08
// Author:      David Al-Daini
///////////////////////////////////////////////////////////////////////////////

#include "addConstraintLocator.h"

MString AddConstraintLocator::typeName = "addConstraintLocator";
MString AddConstraintLocator::axis = "z";

AddConstraintLocator::AddConstraintLocator() {};
AddConstraintLocator::~AddConstraintLocator() {};

//************************************************************
// Argument syntax
//
//
//************************************************************
//const char *getDefListFlag    = "-gdl", *getDefListFlagLong    = "-getDefinitionList";

MSyntax AddConstraintLocator::newSyntax() 
{

	MSyntax syntax;

	//syntax.addFlag(getDefListFlag,		getDefListFlagLong,		MSyntax::kString);

	return syntax;
}



MStatus AddConstraintLocator::doIt( const MArgList& args ) 
{
	MStatus stat = MS::kSuccess;
	
	// Grab the current selection

	MSelectionList selection;
	MGlobal::getActiveSelectionList(selection);

	// Parse argument

	if (args.length()== 2)
	{
		typeName = args.asString(0);
		axis = args.asString(1);
	}
	else if (args.length()==1)
	{
		typeName = args.asString(0);
		axis = "z";
	}
	else		// Assume hinge
	{
		typeName = "Hinge";
		axis = "z";
	}

	if (typeName == "Hinge")
	{
		if (selection.length() != 2)
		{
			MGlobal::displayError("For a hinge 2 objects should be selected");
			return MS::kFailure;
		}
	}
	else if (typeName == "Nail")
	{
		if (selection.length() > 1)
		{
			MGlobal::displayError("For a nail only 1 object should be selected");
			return MS::kFailure;
		}
	}
	else if (typeName == "BallAndSocket")
	{
		if (selection.length() != 2)
		{
			MGlobal::displayError("For a ball and socket constraint 2 objects need to be selected");
			return MS::kFailure;
		}
	}
	else if (typeName == "Sliding")
	{
		if (selection.length() != 2)
		{
			MGlobal::displayError("2 objects must be selected for a sliding constraint");
			return MS::kFailure;
		}
	}

	MItSelectionList iter(selection);

	int Count = 0;

	for (; !iter.isDone(); iter.next() ) 
	{
		MObject selectedObject, selectedShape;
		
		stat = selection.getDependNode(Count, selectedObject);
		//CHECK_STATUS(stat, "getting base mesh from selection");

		stat = getMeshFromTransform(selectedObject, selectedShape, "mesh");		// Curiously, selected shape doesn't seem to be used after this

		if (!stat) 
		{
			MGlobal::displayError(selectedObject.apiTypeStr());
			return MS::kFailure;
		}

		if (!Count)
		{
			object1Mesh = selectedObject;
		}
		else if (Count == 1)
		{
			object2Mesh = selectedObject;
		}

		Count++;
	} 
	
	// Call the command's redo function to do the actual work

	redoIt();

	return stat;
}



MStatus AddConstraintLocator::redoIt() 
{
	MStatus stat = MS::kSuccess;

	//-----------------------------------------
	// Create the constraint locator node
	//-----------------------------------------

	MObject ConstraintLocatorObject = dagMod.createNode("constraintLocator",  MObject::kNullObj, &stat);		// This will be the transform node
	
	CHECK_STATUS(stat, stat.errorString());

	stat = dagMod.doIt();			// Adds node created above to dependency graph

	CHECK_STATUS(stat, "adding node to dependency graph");

	// Set the constraint locator's name

	MFnDagNode LocatorFn(ConstraintLocatorObject);
	MString Name = LocatorFn.setName("ConstraintLocator#", &stat);

	CHECK_STATUS(stat, "setting node name");

	//-----------------------------------------
	// Apply function sets
	//-----------------------------------------

	MObject selectedShape;
	//dgMod.doIt();	
	stat = getMeshFromTransform(ConstraintLocatorObject, selectedShape, "constraintLocator");		// Gets the shape node
	CHECK_STATUS(stat, "getting shape from locator node");

	MFnDependencyNode ConstraintLocatorFn(selectedShape);
	MFnDependencyNode Object1MeshFn(object1Mesh);

	this->setResult(ConstraintLocatorFn.name());

	// Set the translate value of the locator transform to the translate of the object (from world matrix)

	MFnDependencyNode ConstraintTransformFn(ConstraintLocatorObject);
	MPlug ConstraintTransformPlug = ConstraintTransformFn.findPlug("translate", &stat);
	CHECK_STATUS(stat, stat.errorString());

	MPlug ConstraintTranslateXPlug = ConstraintTransformFn.findPlug("translateX", &stat);
	MPlug ConstraintTranslateYPlug = ConstraintTransformFn.findPlug("translateY", &stat);
	MPlug ConstraintTranslateZPlug = ConstraintTransformFn.findPlug("translateZ", &stat);

	MPlug Object1WorldMatrixPlug = Object1MeshFn.findPlug("worldMatrix", &stat);

	// Set the type - hardcoded (oh well....)

	MPlug ConstraintTypePlug = ConstraintLocatorFn.findPlug("constraintType", &stat);
	CHECK_STATUS(stat, "finding locator transform constraint type plug");
	
	int intValue;
	int nSelectedObjects;

	if (typeName == "BallAndSocket")
	{
		intValue = 0;
		nSelectedObjects = 2;
	}
	else if (typeName == "Hinge")
	{
		intValue = 1;
		nSelectedObjects = 2;
	}
	else if (typeName == "Sliding")
	{
		intValue = 2;
		nSelectedObjects = 2;
	}
	else if (typeName == "Nail")
	{
		intValue = 3;
		nSelectedObjects = 1;
	}
	
	

	stat = ConstraintTypePlug.setValue(intValue);
	CHECK_STATUS(stat, "setting constraint type value");

	// Set the axis

	MPlug ConstraintAxisPlug = ConstraintLocatorFn.findPlug("axis", &stat);
	CHECK_STATUS(stat, "finding locator axis plug");
	
	int axisValue;
	
	if (axis == "x")
	{
		axisValue = 0;
	}
	else if (axis == "y")
	{
		axisValue = 1;
	}
	else if (axis == "z")
	{
		axisValue = 2;
	}
	
	stat = ConstraintAxisPlug.setValue(axisValue);
	CHECK_STATUS(stat, "setting constraint axis value");

	MPlug ConstraintLocatorTransformTranslatePlug = ConstraintLocatorFn.findPlug("transformTranslate", &stat);
	CHECK_STATUS(stat, "finding locator transform translate plug");

	// connect the translate attribute of the locator transform to the "transformTranslate" attribute of the locator shape - for future access

	stat = dgMod.connect(ConstraintTransformPlug, ConstraintLocatorTransformTranslatePlug);
	CHECK_STATUS(stat, "connecting translate of locator transform to locator shape");

	// Set the translate value according to whether there are 1 or 2 selected objects - if 2 place in the midpoint, otherwise at the centre of the object

	if (nSelectedObjects == 1)
	{
		float TranslateX, TranslateY, TranslateZ;

		// Need to get the x,y and z translate values from the world matrix of the objects now
		
		MObject matrixObject;

		Object1WorldMatrixPlug.getValue(matrixObject);

		MFnMatrixData matrixFn(matrixObject);
		MMatrix Object1Matrix;

		Object1Matrix = matrixFn.matrix();

		ConstraintTranslateXPlug.setValue(Object1Matrix(3,0));
		ConstraintTranslateYPlug.setValue(Object1Matrix(3,1));
		ConstraintTranslateZPlug.setValue(Object1Matrix(3,2));

		MPlug ConstraintLocatorInMatrix1Plug = ConstraintLocatorFn.findPlug("inWorldMatrix1", &stat);
		CHECK_STATUS(stat, stat.errorString());

		MPlug Object1MatrixPlug = Object1MeshFn.findPlug("worldMatrix", &stat);
		CHECK_STATUS(stat, "finding object's world matrix plug");

		MPlug Object1MatrixElemPlug = Object1MatrixPlug.elementByLogicalIndex(0, &stat);
		CHECK_STATUS(stat, "finding object1's matrix element");

		stat = dgMod.connect(Object1MatrixElemPlug, ConstraintLocatorInMatrix1Plug);
		CHECK_STATUS(stat, "connecting matrix of object1 to locator");
	}
	else
	{
		float Translate1X, Translate1Y, Translate1Z, Translate2X, Translate2Y, Translate2Z;
		float MidPointX, MidPointY, MidPointZ;

		MObject matrixObject;

		Object1WorldMatrixPlug.getValue(matrixObject);

		MFnMatrixData matrixFn(matrixObject);
		MMatrix Object1Matrix;

		Object1Matrix = matrixFn.matrix();

		Translate1X = Object1Matrix(3,0);
		Translate1Y = Object1Matrix(3,1);
		Translate1Z = Object1Matrix(3,2);

		MFnDependencyNode Object2MeshFn(object2Mesh);
		MPlug Object2WorldMatrixPlug = Object2MeshFn.findPlug("worldMatrix", &stat);
		CHECK_STATUS(stat, "finding world matrix plug of second selected object");

		MObject matrixObject2;

		Object2WorldMatrixPlug.getValue(matrixObject2);

		MFnMatrixData matrixFn2(matrixObject2);
		MMatrix Object2Matrix;

		Object2Matrix = matrixFn2.matrix();

		Translate2X = Object2Matrix(3,0);
		Translate2Y = Object2Matrix(3,1);
		Translate2Z = Object2Matrix(3,2);

		MidPointX = (Translate1X + Translate2X)/2.0;
		MidPointY = (Translate1Y + Translate2Y)/2.0;
		MidPointZ = (Translate1Z + Translate2Z)/2.0;

		ConstraintTranslateXPlug.setValue(MidPointX);
		ConstraintTranslateYPlug.setValue(MidPointY);
		ConstraintTranslateZPlug.setValue(MidPointZ);

		// Object1

		MPlug ConstraintLocatorInMatrix1Plug = ConstraintLocatorFn.findPlug("inWorldMatrix1", &stat);
		CHECK_STATUS(stat, "finding locator's inWorldMatrix1 plug");

		MPlug Object1MatrixPlug = Object1MeshFn.findPlug("worldMatrix", &stat);
		CHECK_STATUS(stat, "finding object's world matrix plug");

		MPlug Object1MatrixElemPlug = Object1MatrixPlug.elementByLogicalIndex(0, &stat);
		CHECK_STATUS(stat, "finding object1's matrix element");

		stat = dgMod.connect(Object1MatrixElemPlug, ConstraintLocatorInMatrix1Plug);
		CHECK_STATUS(stat, "connecting matrix of object1 to locator");

		// Now object2

		MPlug ConstraintLocatorInMatrix2Plug = ConstraintLocatorFn.findPlug("inWorldMatrix2", &stat);
		CHECK_STATUS(stat, "finding locator's inWorldMatrix2 plug");

		MPlug Object2MatrixPlug = Object2MeshFn.findPlug("worldMatrix", &stat);
		CHECK_STATUS(stat, "finding object's world matrix plug");

		MPlug Object2MatrixElemPlug = Object2MatrixPlug.elementByLogicalIndex(0, &stat);
		CHECK_STATUS(stat, "finding object2's matrix element");

		stat = dgMod.connect(Object2MatrixElemPlug, ConstraintLocatorInMatrix2Plug);
		CHECK_STATUS(stat, "connecting matrix of object2 to locator");
	}

	dgMod.doIt();
	
	return MS::kSuccess;
}


MStatus AddConstraintLocator::undoIt() 
{
	dagMod.undoIt();
	dgMod.undoIt();
	return MS::kSuccess;
}


