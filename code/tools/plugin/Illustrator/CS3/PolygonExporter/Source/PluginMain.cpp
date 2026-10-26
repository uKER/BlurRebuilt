#pragma pack(push, 4)
#include "AIPlugIn.h"
#include "ADMBasic.h"
#pragma pack(pop)

#include "Defines.h"
#include "PluginHandler.h"

#include "Suites.h"
#include "PolygonExporter.h"
#include "MessageHandler.h"

// Tell Xcode to export the following symbols
#if defined(__GNUC__)
#pragma GCC visibility push(default)
#endif

// Plug-in entry point
extern "C" ASAPI ASErr PluginMain(char* caller, char* selector, void* message);

// Tell Xcode to return to default visibility for symbols
#if defined(__GNUC__)
#pragma GCC visibility pop
#endif

// general globals
Globals* gGlobals;
short gAcquireCount;

static AIErr StartupPlugin( SPInterfaceMessage *message );
static AIErr UnloadPlugin( SPAccessMessage *message );
static AIErr ReloadPlugin( SPAccessMessage *message );
static AIErr ShutdownPlugin( SPInterfaceMessage *message );
static AIErr AboutPlugin( SPInterfaceMessage *message );

//-------------------------------------------------------------------
// Function : PluginMain
// 
// Desc : Program Entry point.
//-------------------------------------------------------------------
extern "C" ASAPI ASErr PluginMain(char* caller, char* selector, void* message)
{
	AIErr error = kNoErr;
	SPBasicSuite* sSPBasic = ((SPMessageData*)message)->basic;

	// standard caller/selectors

	// if these do anything other than restore globals, you may need to move them
	// into the aquire/releaseSuites block
	if ( sSPBasic->IsEqual( caller, kSPAccessCaller ) ) 
	{
		if ( sSPBasic->IsEqual( selector, kSPAccessUnloadSelector ) )
			error = UnloadPlugin( (SPAccessMessage*) message );

		else if ( sSPBasic->IsEqual( selector, kSPAccessReloadSelector ) )
			error = ReloadPlugin( (SPAccessMessage*) message );
	} 

	else 
	{
		// we need suites for the following routines, so acquire all of them now
		error = acquireSuites( ((SPInterfaceMessage*)message)->d.basic );
		if ( error )	
		{
			releaseSuites( ((SPInterfaceMessage*)message)->d.basic );						
		}
		else
		{
			if ( sSPBasic->IsEqual( caller, kSPInterfaceCaller ) ) 
			{
				if ( sSPBasic->IsEqual( selector, kSPInterfaceStartupSelector ) )
				{
					error = StartupPlugin( (SPInterfaceMessage*) message );
				}
				else if ( sSPBasic->IsEqual( selector, kSPInterfaceAboutSelector ) )
				{
					error = AboutPlugin( (SPInterfaceMessage*) message );
				}
				else if ( sSPBasic->IsEqual( selector, kSPInterfaceShutdownSelector ) )
				{
					error = ShutdownPlugin( (SPInterfaceMessage*) message );
				}
			} 
			else  
				error = handlePluginCaller( caller, selector, message );
		}	
	}

	return error;
}


// Acquire all of the suites in the imports array
extern AIErr acquireSuites( SPBasicSuite *sBasic ) {

	AIErr error = kNoErr;
	long i;

	if ( !gAcquireCount ) {	// suites have never been acquired, so get them
		for ( i = 0; i < sizeof(imports)/sizeof(imports[0]); ++i ) {
			if ( imports[i].suite != NULL ) {
				error = sBasic->AcquireSuite( imports[i].name, imports[i].version, (const void**)imports[i].suite );
				if ( error && sADMBasic != NULL)
				{
					char message[256];
					sprintf_s(message, sizeof( message ), "Error: %d, suite: %s, version: %d!", error, imports[i].name, imports[i].version );
					sADMBasic->MessageAlert( message );
					goto error;
				}
			}
		}
	}


error:
	gAcquireCount += 1;
	return error;


}


extern AIErr releaseSuites( SPBasicSuite *sBasic ) {

	AIErr error = kNoErr;
	long i;

	if ( gAcquireCount == 1 ) { // only release the suites on the last release, otherwise they are still in use
		for ( i = 0; i < sizeof(imports)/sizeof(imports[0]); ++i ) {

			void **procs = imports[i].suite;

			if ( procs != NULL ) { // && *procs != NULL ) {
				error = sBasic->ReleaseSuite( imports[i].name, imports[i].version );
				if ( error )
					goto error;
				*procs = NULL;
			}

		}
	}


error:	
	gAcquireCount--;
	return error;

}


static AIErr StartupPlugin ( SPInterfaceMessage *message ) 
{
	AIErr error = kNoErr;

	// Better to use SPBasicSuite for memory allocation (as opposed to alternatives such as SPBlocksSuite).
	error = message->d.basic->AllocateBlock( sizeof(Globals), (void **) &gGlobals );
	if ( error )
		goto error;

	error = pluginStartup( message );

error:
	return error;

}


static AIErr UnloadPlugin( SPAccessMessage *message ) {

	if ( message->d.globals )
		unloadPluginSuiteFunctions( message );

	message->d.globals = gGlobals; 
	return kNoErr;					

}


static AIErr ReloadPlugin( SPAccessMessage *message ) {

	AIErr error = kNoErr;		

	if ( message->d.globals )
		error = reloadPluginSuiteFunctions( message );

	gGlobals = (Globals*) message->d.globals;
	gAcquireCount = 0;
	return error;

}


static AIErr ShutdownPlugin( SPInterfaceMessage *message ) {

	AIErr error = kNoErr;		

	pluginShutdown( message );

	message->d.basic->FreeBlock( gGlobals );
	gGlobals = NULL;
	message->d.globals = NULL;
	return error;

}


static AIErr AboutPlugin( SPInterfaceMessage *message ) {

//	goAbout( message );
	return kNoErr;

}