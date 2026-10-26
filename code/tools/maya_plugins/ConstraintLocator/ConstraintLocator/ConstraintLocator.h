///////////////////////////////////////////////////////////////////////////////
// Copyright © Bizarre Creations 2008
//
// File:        ConstraintLocator.cpp
//
// Created:     23/05/08
// Author:      David Al-Daini
///////////////////////////////////////////////////////////////////////////////


#ifndef ConstraintLocator_H
#define ConstraintLocator_H

#include <maya/MPxLocatorNode.h> 
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/M3dView.h>
#include <maya/MDistance.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MPxManipContainer.h>
#include <maya/MPointArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MArgList.h>
#include <math.h>

class ConstraintLocator : public MPxLocatorNode
{
public:
    virtual void draw(M3dView & view, const MDagPath & path, M3dView::DisplayStyle style, M3dView::DisplayStatus status);
    void drawHinge(MFnDagNode dagFn, M3dView &view, float Scale, M3dView::DisplayStatus status);
	void drawNail(MFnDagNode dagFn, M3dView &view, float Scale, M3dView::DisplayStatus status);
	void drawBallandSocket(MFnDagNode dagFn, M3dView &view, float Scale, M3dView::DisplayStatus status);
	void drawSliding(MFnDagNode dagFn, M3dView &view, float Scale, M3dView::DisplayStatus status);

    virtual bool isBounded() const;
    virtual MBoundingBox boundingBox() const; 

    static void *creator();
    static MStatus initialize();

    static const MTypeId typeId;			// id of the node
    static const MString typeName;			// name of the node

    // Attributes

	static MObject axis;
 	static MObject constraintType;
	static MObject drawScale;
	static MObject inWorldMatrix1;
	static MObject inWorldMatrix2;
	static MObject transformTranslate;

private:
    bool getPoints(MPointArray &pts) const;
	void getCubePoints(MPointArray &pts, MPoint cubeCentre, float length) const;
	MMatrix ConstraintLocator::getInverseMatrix(MStatus &stat);

	enum constraintTypeEnum 
	{
		kBallAndSocket,
		kHinge,
		kSliding,
		kNail
	};

	enum constraintAxisEnum
	{
		kXAxis,
		kYAxis,
		kZAxis
	};
};

MStatus getMeshFromTransform(MObject transform, MObject &mesh, MString type);

#endif
