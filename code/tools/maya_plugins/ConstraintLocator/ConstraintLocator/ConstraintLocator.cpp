///////////////////////////////////////////////////////////////////////////////
// Copyright © Bizarre Creations 2008
//
// File:        ConstraintLocator.cpp
//
// Created:     23/05/08
// Author:      David Al-Daini
///////////////////////////////////////////////////////////////////////////////

#include "ConstraintLocator.h"
#include <maya/MGlobal.h>
#include <maya/MPointArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MMatrix.h>
#include <maya/MFnTransform.h>
#include <maya/MTransformationMatrix.h>
#include "AddConstraintLocator.h"

const MTypeId ConstraintLocator::typeId( 0xBC0013 );
const MString ConstraintLocator::typeName("constraintLocator");

MObject ConstraintLocator::constraintType;
MObject ConstraintLocator::drawScale;
MObject ConstraintLocator::inWorldMatrix1;
MObject ConstraintLocator::inWorldMatrix2;
MObject ConstraintLocator::transformTranslate;
MObject ConstraintLocator::axis;

const double M_2PI = M_PI * 2.0;

/*
bool ConstraintLocator::getCirclePoints(MPointArray &pts)
{
	if (typeValue == kHinge)
	{
		
	}
}
*/

/*
bool ConstraintLocator::getCirclePoints( MPointArray &pts ) const 
{
	MStatus stat;
	MObject thisNode = thisMObject();
	MFnDagNode dagFn(thisNode);  

	MPlug xWidthPlug = dagFn.findPlug(xWidth, &stat);
	float xWidthValue;
	xWidthPlug.getValue(xWidthValue);

	MPlug zWidthPlug = dagFn.findPlug(zWidth, &stat);
	float zWidthValue;
	zWidthPlug.getValue(zWidthValue);

	MPlug typePlug = dagFn.findPlug(dispType, &stat);
	short typeValue;
	typePlug.getValue(typeValue);
	
	MPlug constraintTypePlug = dagFn.findPlug(constraintType, &stat);
	int constraintTypeValue;
	constraintTypePlug.getValue(constraintTypeValue);

	unsigned int nCirclePts;

	switch (constraintTypeValue) 
	{
		case kHinge:
			MGlobal::displayInfo("Hinge");
			break;
		case kNail:
			MGlobal::displayInfo("Nail");
			break;
		case kBallAndSocket:
			MGlobal::displayInfo("Ball and socket");
			break;
		case kSliding:
			MGlobal::displayInfo("Sliding");
			break;
	}

	switch(typeValue)
	{
		case 0:
			nCirclePts = 4;
			break;
		case 1:
			nCirclePts = 5;
			break;
		default:
			nCirclePts = 20;
			break;
	}

	pts.clear();
	pts.setSizeIncrement(nCirclePts);

	MPoint pt;
	pt.y = 0.0;

	const double angleIncr = M_2PI /(nCirclePts - 1);
	double angle = 0.0;
	unsigned int i=0;

	for( ; i<nCirclePts; i++, angle+=angleIncr)
	{
		pt.x = xWidthValue * cos(angle);
		pt.z = zWidthValue * sin(angle);
		pts.append(pt);
	}

	return true;
}
*/


void ConstraintLocator::getCubePoints(MPointArray &pts, MPoint cubeCentre, float length) const
{
	MPoint pt;

	float halfLength = length/2.0;

	pt.x = cubeCentre.x - halfLength;
	pt.y = cubeCentre.y + halfLength;
	pt.z = cubeCentre.z + halfLength;
	pts.append(pt);

	pt.x = cubeCentre.x + halfLength;
	pt.y = cubeCentre.y + halfLength;
	pt.z = cubeCentre.z + halfLength;
	pts.append(pt);

	pt.x = cubeCentre.x + halfLength;
	pt.y = cubeCentre.y - halfLength;
	pt.z = cubeCentre.z + halfLength;
	pts.append(pt);

	pt.x = cubeCentre.x - halfLength;
	pt.y = cubeCentre.y - halfLength;
	pt.z = cubeCentre.z + halfLength;
	pts.append(pt);

	pt.x = cubeCentre.x - halfLength;
	pt.y = cubeCentre.y + halfLength;
	pt.z = cubeCentre.z - halfLength;
	pts.append(pt);

	pt.x = cubeCentre.x + halfLength;
	pt.y = cubeCentre.y + halfLength;
	pt.z = cubeCentre.z - halfLength;
	pts.append(pt);

	pt.x = cubeCentre.x + halfLength;
	pt.y = cubeCentre.y - halfLength;
	pt.z = cubeCentre.z - halfLength;
	pts.append(pt);

	pt.x = cubeCentre.x - halfLength;
	pt.y = cubeCentre.y - halfLength;
	pt.z = cubeCentre.z - halfLength;
	pts.append(pt);
}



bool ConstraintLocator::getPoints(MPointArray &pts) const 
{
	MStatus stat;
	MObject thisNode = thisMObject();
	MFnDagNode dagFn(thisNode);  

	MPlug constraintTypePlug = dagFn.findPlug(constraintType, &stat);
	int constraintTypeValue;
	constraintTypePlug.getValue(constraintTypeValue);

	unsigned int nCirclePts;

	switch (constraintTypeValue) 
	{
		case kBallAndSocket:
			MGlobal::displayInfo("Ball and socket");
			break;
		case kHinge:
			MGlobal::displayInfo("Hinge");
			break;
		case kSliding:
			MGlobal::displayInfo("Sliding");
			break;
		case kNail:
			MGlobal::displayInfo("Nail");
			break;
	}

	nCirclePts = 2;

	pts.clear();
	pts.setSizeIncrement(nCirclePts);

	MPoint pt;
	pt.y = 0.0;
	pt.x = 0.0;
	pt.z = 0.0;

	pts.append(pt);

	pt.z += 2.0;

	pts.append(pt);

	return true;
}


void ConstraintLocator::drawHinge(MFnDagNode dagFn, M3dView &view, float Scale, M3dView::DisplayStatus status)
{
	MStatus stat;

	MMatrix InverseMatrix = getInverseMatrix(stat);

	// Get inMatrix1 

	MObject matrixObject1;
	MPlug inMatrix1Plug = dagFn.findPlug("inWorldMatrix1", &stat);
	inMatrix1Plug.getValue(matrixObject1);
	MFnMatrixData matrix1Fn(matrixObject1);
	MMatrix Object1Matrix = matrix1Fn.matrix();

	// Get inMatrix2 

	MObject matrixObject2;
	MPlug inMatrix2Plug = dagFn.findPlug("inWorldMatrix2", &stat);
	inMatrix2Plug.getValue(matrixObject2);
	MFnMatrixData matrix2Fn(matrixObject2);
	MMatrix Object2Matrix = matrix2Fn.matrix();
	MPoint inTranslatePoint;

	inTranslatePoint.x = Object1Matrix(3,0);
	inTranslatePoint.y = Object1Matrix(3,1);
	inTranslatePoint.z = Object1Matrix(3,2);

	inTranslatePoint *= InverseMatrix;		// These need to be multiplied through because we want them in terms of locator object space

	// Draw cube at constraint locator position - will be origin in object space

	MPointArray cubePts;

	MPoint centrePoint;
	
	centrePoint.x = 0;
	centrePoint.y = 0;
	centrePoint.z = 0;

	float cubeSize = 0.2 * Scale;
	getCubePoints(cubePts, centrePoint, cubeSize);

	// Draw line on either side of locator along axis specified
	
	// Get axis value from relevant plug

	MPlug axisPlug = dagFn.findPlug("axis", &stat);
	int axisValue;
	axisPlug.getValue(axisValue);
	MPoint linePoint1, linePoint2;
	float LineLength = 5.0 * Scale;

	switch (axisValue) 
	{
		case kXAxis:
			linePoint1.x = centrePoint.x - LineLength;
			linePoint2.x = centrePoint.x + LineLength;
			linePoint1.y = linePoint2.y = centrePoint.y;
			linePoint1.z = linePoint2.z = centrePoint.z;
			break;
		case kYAxis:
			linePoint1.y = centrePoint.y - LineLength;
			linePoint2.y = centrePoint.y + LineLength;
			linePoint1.x = linePoint2.x = centrePoint.x;
			linePoint1.z = linePoint2.z = centrePoint.z;
			break;
		case kZAxis:
			linePoint1.z = centrePoint.z - LineLength;
			linePoint2.z = centrePoint.z + LineLength;
			linePoint1.x = linePoint2.x = centrePoint.x;
			linePoint1.y = linePoint2.y = centrePoint.y;
			break;
	}

	view.beginGL();  
	glPushAttrib(GL_CURRENT_BIT);
	glBegin(GL_LINES);

	glVertex3f(linePoint1.x, linePoint1.y, linePoint1.z);
	glVertex3f(linePoint2.x, linePoint2.y, linePoint2.z);

	for (unsigned int i=0; i<cubePts.length(); i++)
	{
		if (i != 3 && i != 7)
		{
			glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
			glVertex3f(cubePts[i+1].x, cubePts[i+1].y, cubePts[i+1].z);
		}
		else 
		{
			glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
			glVertex3f(cubePts[i-3].x, cubePts[i-3].y, cubePts[i-3].z);
		}
	}

	for (unsigned int i=0; i<4; i++)
	{
		glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
		glVertex3f(cubePts[i+4].x, cubePts[i+4].y, cubePts[i+4].z);
	}

	// Draw a line from the locator to the object (or objects);

	if (centrePoint.x != inTranslatePoint.x || centrePoint.y != inTranslatePoint.y || centrePoint.z != inTranslatePoint.z)
	{
		glVertex3f(centrePoint.x, centrePoint.y, centrePoint.z);
		glVertex3f(inTranslatePoint.x, inTranslatePoint.y, inTranslatePoint.z);
	}

	// Get inTranslate 2 coordinates

	MPoint inTranslate2Point;

	inTranslate2Point.x = Object2Matrix(3,0);
	inTranslate2Point.y = Object2Matrix(3,1);
	inTranslate2Point.z = Object2Matrix(3,2);

	inTranslate2Point *= InverseMatrix;		// These need to be multiplied through because we want them in terms of locator object space

	if (centrePoint.x != inTranslate2Point.x || centrePoint.y != inTranslate2Point.y || centrePoint.z != inTranslate2Point.z)
	{
		glVertex3f(centrePoint.x, centrePoint.y, centrePoint.z);
		glVertex3f(inTranslate2Point.x, inTranslate2Point.y, inTranslate2Point.z);
	}

	glEnd();

	glPopAttrib();
	view.endGL();    
}


void ConstraintLocator::drawNail(MFnDagNode dagFn, M3dView &view, float Scale, M3dView::DisplayStatus status)
{
	MStatus stat;

	MMatrix InverseMatrix = getInverseMatrix(stat);

	// Get inMatrix1 

	MObject matrixObject1;
	MPlug inMatrix1Plug = dagFn.findPlug("inWorldMatrix1", &stat);
	inMatrix1Plug.getValue(matrixObject1);
	MFnMatrixData matrix1Fn(matrixObject1);
	MMatrix Object1Matrix = matrix1Fn.matrix();

	MPoint inTranslatePoint;

	inTranslatePoint.x = Object1Matrix(3,0);
	inTranslatePoint.y = Object1Matrix(3,1);
	inTranslatePoint.z = Object1Matrix(3,2);

	inTranslatePoint *= InverseMatrix;

	// Draw cube at constraint locator position - will be origin in object space

	MPointArray cubePts;

	MPoint centrePoint;
	
	centrePoint.x = 0;
	centrePoint.y = 0;
	centrePoint.z = 0;

	float cubeSize = 0.2 * Scale;
	getCubePoints(cubePts, centrePoint, cubeSize);

	view.beginGL();  
	glPushAttrib(GL_CURRENT_BIT);
	glBegin(GL_LINES);

	for (unsigned int i=0; i<cubePts.length(); i++)
	{
		if (i != 3 && i != 7)
		{
			glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
			glVertex3f(cubePts[i+1].x, cubePts[i+1].y, cubePts[i+1].z);
		}
		else 
		{
			glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
			glVertex3f(cubePts[i-3].x, cubePts[i-3].y, cubePts[i-3].z);
		}
	}

	for (unsigned int i=0; i<4; i++)
	{
		glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
		glVertex3f(cubePts[i+4].x, cubePts[i+4].y, cubePts[i+4].z);
	}

	// Draw a line from the locator to the object

	if (centrePoint.x != inTranslatePoint.x || centrePoint.y != inTranslatePoint.y || centrePoint.z != inTranslatePoint.z)
	{
		glVertex3f(centrePoint.x, centrePoint.y, centrePoint.z);
		glVertex3f(inTranslatePoint.x, inTranslatePoint.y, inTranslatePoint.z);
	}

	glEnd();

	glPopAttrib();
	view.endGL();    
}


void ConstraintLocator::drawBallandSocket(MFnDagNode dagFn, M3dView &view, float Scale, M3dView::DisplayStatus status)
{
	MStatus stat;

	MMatrix InverseMatrix = getInverseMatrix(stat);

	// Need to find the midpoint of the 2 selected objects

	// Get inMatrix1 

	MObject matrixObject1;
	MPlug inMatrix1Plug = dagFn.findPlug("inWorldMatrix1", &stat);
	inMatrix1Plug.getValue(matrixObject1);
	MFnMatrixData matrix1Fn(matrixObject1);
	MMatrix Object1Matrix = matrix1Fn.matrix();

	MPoint inTranslatePoint1;

	inTranslatePoint1.x = Object1Matrix(3,0);
	inTranslatePoint1.y = Object1Matrix(3,1);
	inTranslatePoint1.z = Object1Matrix(3,2);

	inTranslatePoint1 *= InverseMatrix;

	// Get inTranslate2 coordinates

	// Get inMatrix2 

	MObject matrixObject2;
	MPlug inMatrix2Plug = dagFn.findPlug("inWorldMatrix2", &stat);
	inMatrix2Plug.getValue(matrixObject2);
	MFnMatrixData matrix2Fn(matrixObject2);
	MMatrix Object2Matrix = matrix2Fn.matrix();

	MPoint inTranslatePoint2;

	inTranslatePoint2.x = Object2Matrix(3,0);
	inTranslatePoint2.y = Object2Matrix(3,1);
	inTranslatePoint2.z = Object2Matrix(3,2);

	inTranslatePoint2 *= InverseMatrix;

	// Now get midpoint

	MPoint midPoint;

	midPoint.x = (inTranslatePoint1.x + inTranslatePoint2.x)/2.0;
	midPoint.y = (inTranslatePoint1.y + inTranslatePoint2.y)/2.0;
	midPoint.z = (inTranslatePoint1.z + inTranslatePoint2.z)/2.0;

	// Draw cube at constraint locator position - will be origin in object space

	MPointArray cubePts;

	MPoint centrePoint;
	
	centrePoint.x = 0;
	centrePoint.y = 0;
	centrePoint.z = 0;
	
	float cubeSize = 0.2 * Scale;
	getCubePoints(cubePts, centrePoint, cubeSize);

	view.beginGL();  
	glPushAttrib(GL_CURRENT_BIT);
	glBegin(GL_LINES);

	for (unsigned int i=0; i<cubePts.length(); i++)
	{
		if (i != 3 && i != 7)
		{
			glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
			glVertex3f(cubePts[i+1].x, cubePts[i+1].y, cubePts[i+1].z);
		}
		else 
		{
			glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
			glVertex3f(cubePts[i-3].x, cubePts[i-3].y, cubePts[i-3].z);
		}
	}

	for (unsigned int i=0; i<4; i++)
	{
		glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
		glVertex3f(cubePts[i+4].x, cubePts[i+4].y, cubePts[i+4].z);
	}

	// Draw a line from the locator to both the objects
	
	if (centrePoint.x != inTranslatePoint1.x || centrePoint.y != inTranslatePoint1.y || centrePoint.z != inTranslatePoint1.z)
	{
		glVertex3f(centrePoint.x, centrePoint.y, centrePoint.z);
		glVertex3f(inTranslatePoint1.x, inTranslatePoint1.y, inTranslatePoint1.z);
	}
	
	if (centrePoint.x != inTranslatePoint2.x || centrePoint.y != inTranslatePoint2.y || centrePoint.z != inTranslatePoint2.z)
	{
		glVertex3f(centrePoint.x, centrePoint.y, centrePoint.z);
		glVertex3f(inTranslatePoint2.x, inTranslatePoint2.y, inTranslatePoint2.z);
	}
	
	glEnd();

	glPopAttrib();
	view.endGL();    
}

void ConstraintLocator::drawSliding(MFnDagNode dagFn, M3dView &view, float Scale, M3dView::DisplayStatus status)
{
	MStatus stat;

	MMatrix InverseMatrix = getInverseMatrix(stat);

	// Need to find the midpoint of the 2 selected objects

	// Get inMatrix1 

	MObject matrixObject1;
	MPlug inMatrix1Plug = dagFn.findPlug("inWorldMatrix1", &stat);
	inMatrix1Plug.getValue(matrixObject1);
	MFnMatrixData matrix1Fn(matrixObject1);
	MMatrix Object1Matrix = matrix1Fn.matrix();

	MPoint inTranslatePoint1;

	inTranslatePoint1.x = Object1Matrix(3,0);
	inTranslatePoint1.y = Object1Matrix(3,1);
	inTranslatePoint1.z = Object1Matrix(3,2);

	inTranslatePoint1 *= InverseMatrix;

	// Get inTranslate2 coordinates

	// Get inMatrix2 

	MObject matrixObject2;
	MPlug inMatrix2Plug = dagFn.findPlug("inWorldMatrix2", &stat);
	inMatrix2Plug.getValue(matrixObject2);
	MFnMatrixData matrix2Fn(matrixObject2);
	MMatrix Object2Matrix = matrix2Fn.matrix();

	MPoint inTranslatePoint2;

	inTranslatePoint2.x = Object2Matrix(3,0);
	inTranslatePoint2.y = Object2Matrix(3,1);
	inTranslatePoint2.z = Object2Matrix(3,2);

	inTranslatePoint2 *= InverseMatrix;

	// Now get midpoint

	MPoint midPoint;

	midPoint.x = (inTranslatePoint1.x + inTranslatePoint2.x)/2.0;
	midPoint.y = (inTranslatePoint1.y + inTranslatePoint2.y)/2.0;
	midPoint.z = (inTranslatePoint1.z + inTranslatePoint2.z)/2.0;

	// Draw cube at constraint locator position - will be origin in object space

	MPointArray cubePts;

	MPoint centrePoint;
	
	centrePoint.x = 0;
	centrePoint.y = 0;
	centrePoint.z = 0;
	
	float cubeSize = 0.2 * Scale;
	getCubePoints(cubePts, centrePoint, cubeSize);

	view.beginGL();  
	glPushAttrib(GL_CURRENT_BIT);
	glBegin(GL_LINES);

	for (unsigned int i=0; i<cubePts.length(); i++)
	{
		if (i != 3 && i != 7)
		{
			glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
			glVertex3f(cubePts[i+1].x, cubePts[i+1].y, cubePts[i+1].z);
		}
		else 
		{
			glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
			glVertex3f(cubePts[i-3].x, cubePts[i-3].y, cubePts[i-3].z);
		}
	}

	for (unsigned int i=0; i<4; i++)
	{
		glVertex3f(cubePts[i].x, cubePts[i].y, cubePts[i].z);
		glVertex3f(cubePts[i+4].x, cubePts[i+4].y, cubePts[i+4].z);
	}

	// Draw a line from the locator to both the objects and draw a cube at each end (to distinguish from ball and socket)
	
	if (centrePoint.x != inTranslatePoint1.x || centrePoint.y != inTranslatePoint1.y || centrePoint.z != inTranslatePoint1.z)
	{
		glVertex3f(centrePoint.x, centrePoint.y, centrePoint.z);
		glVertex3f(inTranslatePoint1.x, inTranslatePoint1.y, inTranslatePoint1.z);
	}
	
	if (centrePoint.x != inTranslatePoint2.x || centrePoint.y != inTranslatePoint2.y || centrePoint.z != inTranslatePoint2.z)
	{
		glVertex3f(centrePoint.x, centrePoint.y, centrePoint.z);
		glVertex3f(inTranslatePoint2.x, inTranslatePoint2.y, inTranslatePoint2.z);
	}

	MPointArray cubePts1;
	getCubePoints(cubePts1, inTranslatePoint1, 0.2);

	for (unsigned int i=0; i<cubePts1.length(); i++)
	{
		if (i != 3 && i != 7)
		{
			glVertex3f(cubePts1[i].x, cubePts1[i].y, cubePts1[i].z);
			glVertex3f(cubePts1[i+1].x, cubePts1[i+1].y, cubePts1[i+1].z);
		}
		else 
		{
			glVertex3f(cubePts1[i].x, cubePts1[i].y, cubePts1[i].z);
			glVertex3f(cubePts1[i-3].x, cubePts1[i-3].y, cubePts1[i-3].z);
		}
	}

	for (unsigned int i=0; i<4; i++)
	{
		glVertex3f(cubePts1[i].x, cubePts1[i].y, cubePts1[i].z);
		glVertex3f(cubePts1[i+4].x, cubePts1[i+4].y, cubePts1[i+4].z);
	}

	MPointArray cubePts2;
	getCubePoints(cubePts2, inTranslatePoint2, 0.2);

	for (unsigned int i=0; i<cubePts2.length(); i++)
	{
		if (i != 3 && i != 7)
		{
			glVertex3f(cubePts2[i].x, cubePts2[i].y, cubePts2[i].z);
			glVertex3f(cubePts2[i+1].x, cubePts2[i+1].y, cubePts2[i+1].z);
		}
		else 
		{
			glVertex3f(cubePts2[i].x, cubePts2[i].y, cubePts2[i].z);
			glVertex3f(cubePts2[i-3].x, cubePts2[i-3].y, cubePts2[i-3].z);
		}
	}

	for (unsigned int i=0; i<4; i++)
	{
		glVertex3f(cubePts2[i].x, cubePts2[i].y, cubePts2[i].z);
		glVertex3f(cubePts2[i+4].x, cubePts2[i+4].y, cubePts2[i+4].z);
	}

	// Draw line through centre of second object along axis specified
	
	// Get axis value from relevant plug

	MPlug axisPlug = dagFn.findPlug("axis", &stat);
	int axisValue;
	axisPlug.getValue(axisValue);
	MPoint linePoint1, linePoint2;
	float LineLength = 5.0 * Scale;

	switch (axisValue) 
	{
		case kXAxis:
			linePoint1.x = inTranslatePoint2.x - LineLength;
			linePoint2.x = inTranslatePoint2.x + LineLength;
			linePoint1.y = linePoint2.y = inTranslatePoint2.y;
			linePoint1.z = linePoint2.z = inTranslatePoint2.z;
			break;
		case kYAxis:
			linePoint1.y = inTranslatePoint2.y - LineLength;
			linePoint2.y = inTranslatePoint2.y + LineLength;
			linePoint1.x = linePoint2.x = inTranslatePoint2.x;
			linePoint1.z = linePoint2.z = inTranslatePoint2.z;
			break;
		case kZAxis:
			linePoint1.z = inTranslatePoint2.z - LineLength;
			linePoint2.z = inTranslatePoint2.z + LineLength;
			linePoint1.x = linePoint2.x = inTranslatePoint2.x;
			linePoint1.y = linePoint2.y = inTranslatePoint2.y;
			break;
	}

	glVertex3f(linePoint1.x, linePoint1.y, linePoint1.z);
	glVertex3f(linePoint2.x, linePoint2.y, linePoint2.z);

	glEnd();

	glPopAttrib();
	view.endGL();    
}


void ConstraintLocator::draw(M3dView & view, const MDagPath & path, M3dView::DisplayStyle style, M3dView::DisplayStatus status)
{ 
	MStatus stat;
	MObject thisNode = this->thisMObject();
	MFnDagNode dagFn(thisNode);  

	MPlug constraintTypePlug = dagFn.findPlug(constraintType, &stat);
	int typeValue;
	constraintTypePlug.getValue(typeValue);

	MPlug drawScalePlug = dagFn.findPlug(drawScale, &stat);
	float Scale;
	drawScalePlug.getValue(Scale);

	if (typeValue == kHinge)
	{
		drawHinge(dagFn, view, Scale, status);
	}
	else if (typeValue == kNail)
	{
		drawNail(dagFn, view, Scale, status);
	}
	else if (typeValue == kSliding)
	{
		drawSliding(dagFn, view, Scale, status);
	}
	else if (typeValue == kBallAndSocket)
	{
		drawBallandSocket(dagFn, view, Scale, status);
	}
}

bool ConstraintLocator::isBounded() const
{ 
	return false;
}

MBoundingBox ConstraintLocator::boundingBox() const
//
// N.B. It is important to have this bounding box function otherwise zoom selected and 
// zoom all won't work correctly.
//
{   
	//MPointArray pts;
	//getPoints(pts);

	MBoundingBox bbox;
	/*for( unsigned int i=0; i < pts.length(); i++ )
		bbox.expand( pts[i] );*/
	return bbox;
}


void *ConstraintLocator::creator()
{
	return new ConstraintLocator();
}


MStatus ConstraintLocator::initialize()
{ 
	MFnUnitAttribute unitFn;    
	MFnNumericAttribute numFn;
	MFnEnumAttribute enumFn;
	MFnMatrixAttribute matrixFn;
	MStatus stat;

	//=====================================
	// Create the enum attributes
	//=====================================

	constraintType = enumFn.create( "constraintType", "t", 0, &stat );
	enumFn.addField("BallAndSocket",	kBallAndSocket);
	enumFn.addField("Hinge",			kHinge);
	enumFn.addField("Sliding",			kSliding);
	enumFn.addField("Nail",				kNail);
		
	enumFn.setKeyable(true);
	stat = addAttribute(constraintType);

	axis = enumFn.create("axis", "a", 2, &stat);
	enumFn.addField("x", kXAxis);
	enumFn.addField("y", kYAxis);
	enumFn.addField("z", kZAxis);
	stat = addAttribute(axis);

	inWorldMatrix1 = matrixFn.create("inWorldMatrix1", "inWorldMat1");
	matrixFn.setKeyable(true);
	stat = addAttribute(inWorldMatrix1);

	if (!stat)
	{
		stat.perror( "Unable to add \"inWorldMatrix1\" attribute");
		return stat;
	}

	inWorldMatrix2 = matrixFn.create("inWorldMatrix2", "inWorldMat2");
	matrixFn.setKeyable(true);
	stat = addAttribute(inWorldMatrix2);

	if (!stat)
	{
		stat.perror( "Unable to add \"inWorldMatrix2\" attribute");
		return stat;
	}

	transformTranslate = numFn.createPoint("transformTranslate", "tTrans");
	numFn.setDefault(0.0, 0.0, 0.0);
	numFn.setKeyable(true);
	stat = addAttribute(transformTranslate);

	if (!stat)
	{
		stat.perror( "Unable to add \"transformTranslate\" attribute");
		return stat;
	}

	drawScale = numFn.create("drawScale", "dScale", MFnNumericData::kFloat);
	numFn.setDefault(1.0);
	numFn.setKeyable(true);
	stat = addAttribute(drawScale);

	return MS::kSuccess;
}


MMatrix ConstraintLocator::getInverseMatrix(MStatus &stat) 
{
	// Get the transform of this object

    MObject thisNode = this->thisMObject();

    MFnDagNode thisNodeFn(thisNode);

    MObject transform = thisNodeFn.parent(0, &stat);

    if (!stat) 
	{
		return MMatrix();
	}

    // Attach a function set

    MFnTransform transformFn(transform);

    // Now grab the transformation matrix object

    MTransformationMatrix transformationMatrix = transformFn.transformation(&stat);

    if (!stat) 
	{			
		return MMatrix();
	}
 
    // Convert to MMatrix

	return transformationMatrix.asMatrixInverse();
}



MStatus getMeshFromTransform(MObject transform, MObject &mesh, MString type) 
{
	MStatus stat = MS::kSuccess;

	// Ensure this is a dag node

	MFnDagNode transformFn(transform, &stat);

	if (!stat)
	{
		return stat;
	}

	// Ensure this transform has children

	if (!transformFn.childCount())
	{
		return MS::kFailure;
	}

	// Grab the first child

	MObject tempObj = transformFn.child(0, &stat);

	if (!stat)
	{
		return stat;
	}

	// See if this object is a mesh

	MFnDependencyNode meshFn(tempObj);
	MGlobal::displayInfo(meshFn.typeName());

	if (meshFn.typeName() != type)
	{
		return MS::kFailure;
	}

	// Now set the object and return the status code

	mesh = tempObj;

	return stat;
}

