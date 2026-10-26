#include <string>

#include "PluginHandler.h"
#include "AIFileFormat.h"
#include "PolygonExporter.h"


AIErr handlePluginCaller( char* caller, char* selector, void *message ) 
{
	AIErr error = kNoErr;

	SPBasicSuite *sSPBasic = ( ( SPMessageData* )message)->basic;

	if ( sSPBasic->IsEqual( caller, kCallerAIFileFormat ) ) 
	{
		// Build the shape list and write them out to a file.
		if ( sSPBasic->IsEqual( selector, kSelectorAIGoFileFormat ) )
			error = goFileFormat( ( AIFileFormatMessage* )message );
	}
	return error;
}


// extend startup here
AIErr pluginStartup( SPInterfaceMessage *message ) 
{
	return ( addFileFormat( message ) );
}


/*	There are no suites acquired at this point, as you should mostly be doing
system stuff here. */
void pluginShutdown( SPInterfaceMessage *message ) 
{
}


AIErr reloadPluginSuiteFunctions( SPAccessMessage *message ) 
{
	return kNoErr;
}


void unloadPluginSuiteFunctions( SPAccessMessage *message ) 
{
}