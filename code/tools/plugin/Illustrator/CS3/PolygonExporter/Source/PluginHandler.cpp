#include "Defines.h"

#include "PluginHandler.h"
#include "NotifierHandler.h"

#pragma pack(push, 4)
#include "AIFileFormat.h"
#pragma pack(pop)

#include "PolygonExporter.h"

// For menu items.
#pragma pack(push, 4)
#include "SDKDef.h"
#include "SDKAboutPluginsHelper.h"
#pragma pack(pop)

#include "ui/Illustrator/Ipf/IpfWriter.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

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

	if ( sSPBasic->IsEqual( caller, kSelectorAIGoFileFormat ) )
	{
		int test = 0;
	}

	if ( sSPBasic->IsEqual( caller, kCallerAIMenu ) ) 
	{
		if ( sSPBasic->IsEqual( selector, kSelectorAIGoMenuItem ) )
			error = goMenuItem( ( AIMenuMessage* )message );

		else 
		if ( sSPBasic->IsEqual( selector, kSelectorAIUpdateMenuItem ) )
			error = updateMenuItem( ( AIMenuMessage* )message );
	} 


	if ( sSPBasic->IsEqual( caller, kCallerAINotify ) ) 
	{
		if ( sSPBasic->IsEqual( selector, kSelectorAINotify ) )
			error = goNotifier( ( AINotifierMessage* )message );
	}
	return error;
}


// extend startup here
AIErr pluginStartup( SPInterfaceMessage *message ) 
{
	// store a pointer to basic (needed for use in callbacks)
	// we will need this later during the ADM initialization and destroy procs
	gGlobals->pBasic = message->d.basic;

	AIErr error = kNoErr;

	error = addFileFormat( message );

	if ( !error ) 
		error = addMenu( message );

	// Adds all notifiers used in this plug-in
	addNotifiers( message );

	return error;
}


void pluginShutdown( SPInterfaceMessage *message ) 
{
	if ( gGlobals->nonModalDialog) 
	{									
		sADMDialog->Destroy( gGlobals->nonModalDialog );
	}
}


AIErr reloadPluginSuiteFunctions( SPAccessMessage *message ) 
{
	return kNoErr;
}


void unloadPluginSuiteFunctions( SPAccessMessage *message ) 
{
}


//-------------------------------------------------------------------
// Menu item functions.
//-------------------------------------------------------------------
extern AIErr goMenuItem( AIMenuMessage *message ) 
{
	AIErr error = kNoErr;
	if ( message->menuItem == gGlobals->aboutPluginMenu ) 
	{
		Util::C_String version = "A plug-in for converting Adobe scenes into vector graphic files (.ipf) \\n\\nVersion ";
					   version += Ipf::C_IpfWriter::GetFileVersionString();
					   version += "\\n\\nCopyright 2008 Bizarre Creations \\n \\n";

		// Pop this plug-in's about box.
		SDKAboutPluginsHelper aboutPluginsHelper;
		aboutPluginsHelper.PopAboutBox(message, "About Polygon Exporter", version );
	}
	else
	if ( message->menuItem == gGlobals->showExportOptions )
	{
		sADMDialog->Show( gGlobals->nonModalDialog, !( sADMDialog->IsVisible( gGlobals->nonModalDialog ) ) );
	}
	return error;
}


AIErr updateMenuItem( AIMenuMessage *message ) 
{
	AIErr error = kNoErr;

	if ( sADMDialog->IsVisible( gGlobals->nonModalDialog ) )
	{
		sAIMenu->SetItemText( message->menuItem, ai::UnicodeString("Hide Export Options") );
	}
	else
	{
		sAIMenu->SetItemText( message->menuItem, ai::UnicodeString("Show Export Options") );
	}
	return error;
}

	

extern AIErr addMenu( SPInterfaceMessage *message ) 
{
	AIErr error = kNoErr;

	// Add a menu item to the About SDK Plug-ins menu group.
	SDKAboutPluginsHelper aboutPluginsHelper;
	error = aboutPluginsHelper.AddAboutPluginsMenuItem( message, 
														"About plug-ins", 
														ai::UnicodeString("About plug-ins"), 
														"Polygon Exporter...", 
														&gGlobals->aboutPluginMenu);

	// Add a new menu to the windows panel.
	// Add other menus.
	AIPlatformAddMenuItemDataUS menuData;
	AIPlatformAddMenuItemDataUS pluginMenuData;

	const char* pMenuGroupStr	= "BizarreGroup";
	const char* pMenuStr		= "Bizarre Tools";
	const char *pPluginStr		= "Polygon Exporter";

	AIMenuGroup	menuGroup;
	AIMenuGroup	pluginMenuGroup;
	AIMenuGroup dummyGroup;
	AIMenuItemHandle dummyItem;

	menuData.groupName = kOtherPalettesMenuGroup;
	menuData.itemText = ai::UnicodeString(pMenuStr);

	pluginMenuData.groupName = pMenuGroupStr;
	pluginMenuData.itemText = ai::UnicodeString(pPluginStr);
	
	// search through the menus and 
	// see if another SDK plug in has already set up the group
	long count;
	sAIMenu->CountMenuGroups( &count );

	bool bCreateMenuGroup = true;
	bool bCreateSubMenuGroup = true;

	for ( long i = 0; i < count; i++ )
	{
		sAIMenu->GetNthMenuGroup( i, &dummyGroup );

		char* pName = NULL;
		error = sAIMenu->GetMenuGroupName( dummyGroup, (const char**)&pName );

		if ( strcmp( pName, pMenuGroupStr ) == 0 )
		{
			bCreateMenuGroup = false;
		}
		if(std::strcmp( pName, pPluginStr ) == 0)
		{
			bCreateSubMenuGroup = false;
		}
	}

	if ( bCreateMenuGroup )
	{
		sAIMenu->AddMenuItem( message->d.self, pMenuGroupStr, &menuData, 0, &dummyItem );
		sAIMenu->AddMenuGroupAsSubMenu( pMenuGroupStr, kMenuGroupSortedAlphabeticallyOption, dummyItem, &menuGroup );
	}
	if ( bCreateSubMenuGroup )
	{
		sAIMenu->AddMenuItem( message->d.self, pPluginStr, &pluginMenuData, 0, &dummyItem );
		sAIMenu->AddMenuGroupAsSubMenu( pPluginStr, kMenuGroupSortedAlphabeticallyOption, dummyItem, &pluginMenuGroup );
	}

	// Add option to turn window on/off
	pluginMenuData.groupName = pPluginStr;
	sAIMenu->AddMenuItem( message->d.self, pPluginStr, &pluginMenuData, kMenuItemWantsUpdateOption, &gGlobals->showExportOptions );

	return error;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", on ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif