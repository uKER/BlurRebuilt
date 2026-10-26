///////////////////////////////////////////////////////////////////////////////
// Copyright © Bizarre Creations 2006
//
// File:        pluginMain.h
//
// Created:     08/11/06
// Author:      Michael Bibby
///////////////////////////////////////////////////////////////////////////////

#include "MayaToSorcery.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj ) {

	MStatus   status;
	MFnPlugin plugin( obj, "Michael Bibby", "", "Any");

	status = plugin.registerCommand( MayaToSorcery::typeName, 
									 MayaToSorcery::creator, 
									 MayaToSorcery::newSyntax);
	if (!status) {
		status.perror("Registering command: mayaToSorcery");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj) {

	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterCommand( MayaToSorcery::typeName );
	if (!status) {
		status.perror("De-registering command: mayaToSorcery");
		return status;
	}

	return status;
}
