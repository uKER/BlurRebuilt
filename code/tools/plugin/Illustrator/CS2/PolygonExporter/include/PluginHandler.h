
#ifndef PLUGINHANDLER_H
#define PLUGINHANDLER_H


#include "AIPlugIn.h"
#include "ADMBasic.h"
#include "SPInterf.h"


AIErr handlePluginCaller( char* caller, char* selector, void *message );

AIErr pluginStartup( SPInterfaceMessage *message );
void pluginShutdown( SPInterfaceMessage *message );
AIErr reloadPluginSuiteFunctions( SPAccessMessage *message );
void unloadPluginSuiteFunctions( SPAccessMessage *message );

#endif