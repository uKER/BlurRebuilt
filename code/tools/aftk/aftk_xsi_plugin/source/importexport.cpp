#include "XSIA3DExportPCH.h"
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#include <xsi_decl.h>

using namespace XSI; 

//*****************************************************************************
/*!	Register the commands, menus, and properties that implement this plug-in.
	\param in_reg The PluginRegistrar created by XSI for this plug-in.
 */
//*****************************************************************************

XSIPLUGINCALLBACK CStatus XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"Bizarre Creations");
	in_reg.PutName(L"A3D Translator Plug-in");
	in_reg.PutVersion(1,0);
	
	// Register commands for importing and exporting a polygon mesh
	in_reg.RegisterCommand(L"A3DExport");
	in_reg.RegisterCommand(L"A3DImport");
	
	// Install a top-level menu for the import/export tool
	in_reg.RegisterMenu(siMenuMainFileImportID, L"A3DImporter", false, false);
	in_reg.RegisterMenu(siMenuMainFileExportID, L"A3DExporter", false, false);

	// Register a custom property to use as the export UI	
	in_reg.RegisterProperty(L"A3DExportProp");

	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	return CStatus::OK;
}
