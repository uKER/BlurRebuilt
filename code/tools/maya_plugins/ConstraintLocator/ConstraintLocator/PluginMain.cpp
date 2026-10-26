///////////////////////////////////////////////////////////////////////////////
// Copyright © Bizarre Creations 2008
//
// File:        PluginMain.cpp
//
// Created:     23/05/08
// Author:      David Al-Daini
///////////////////////////////////////////////////////////////////////////////

#include "ConstraintLocator.h"
#include "AddConstraintLocator.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{ 
	MStatus stat;
	MString errStr;
	MFnPlugin plugin( obj, "David Al-Daini", "", "Any");

	stat = plugin.registerCommand("addConstraintLocator", AddConstraintLocator::creator, AddConstraintLocator::newSyntax);

	stat = plugin.registerNode(ConstraintLocator::typeName, ConstraintLocator::typeId, &ConstraintLocator::creator, &ConstraintLocator::initialize, MPxNode::kLocatorNode);

	if (!stat)
	{
		errStr = "registerNode failed";
		goto error;
	}

	return stat;

	error:

	stat.perror(errStr);

	return stat;
}


MStatus uninitializePlugin( MObject obj)
{
	MStatus stat;
	MString errStr;
	MFnPlugin plugin(obj);

	stat = plugin.deregisterNode(ConstraintLocator::typeId);

	if (!stat) 
	{
		errStr = "deregisterNode failed";
		goto error;
	}

	return stat;

	error:

	stat.perror(errStr);

	return stat;
}
