
#ifndef PLUGINHANDLER_H
#define PLUGINHANDLER_H

#pragma pack(push, 4)
#include "AIPlugIn.h"
#include "ADMBasic.h"
#include "SPInterf.h"
#include "AIMenu.h"
#pragma pack(pop)

AIErr handlePluginCaller( char* caller, char* selector, void *message );

// Main plugin functionality..
AIErr pluginStartup( SPInterfaceMessage *message );
void pluginShutdown( SPInterfaceMessage *message );
AIErr reloadPluginSuiteFunctions( SPAccessMessage *message );
void unloadPluginSuiteFunctions( SPAccessMessage *message );


// About menu items.
AIErr addMenu( SPInterfaceMessage *message );
AIErr goMenuItem( AIMenuMessage *message );
AIErr updateMenuItem( AIMenuMessage *message );

#endif