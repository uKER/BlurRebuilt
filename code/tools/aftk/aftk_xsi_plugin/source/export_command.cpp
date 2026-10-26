#include "XSIA3DExportPCH.h"

#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#include <xsi_string.h>
#include <xsi_argument.h>
#include <xsi_command.h>
#include <xsi_menu.h>
#include <xsi_model.h>
#include <xsi_parameter.h>
#include <xsi_x3dobject.h>
#include <xsi_selection.h>
#include <xsi_primitive.h>
#include <xsi_polygonmesh.h>
#include <xsi_geometryaccessor.h>
#include <xsi_longarray.h>
#include <xsi_floatarray.h>
#include <xsi_doublearray.h>
#include <xsi_bitarray.h>
#include <xsi_envelopeweight.h>
#include <xsi_customproperty.h>
#include <xsi_griddata.h>
#include <xsi_clusterproperty.h>
#include <xsi_material.h>
#include <xsi_ppglayout.h>
#include <xsi_userdatamap.h>
#include <xsi_imageclip2.h>
#include <xsi_source.h>
#include <xsi_texture.h>
#include <xsi_math.h>
#include <xsi_progressbar.h>
#include <xsi_uitoolkit.h>

#include "helper.h"

#include "Export/ExportOptions.h"
#include "Export/SceneExport.h"

extern XSI::CustomProperty GetA3DExportProp();
extern XSI::Parameter GetImportExportOption( const XSI::CString& in_strName );

// helper functions for exporting mesh components


XSIPLUGINCALLBACK XSI::CStatus A3DExport_Init( XSI::CRef& in_ctxt )
{
	XSI::Context ctxt( in_ctxt );
	XSI::Command oCmd;
	oCmd = ctxt.GetSource();
	
	// Specify that the command returns a value
	oCmd.EnableReturnValue(true);

	// Add arguments to the command
	XSI::ArgumentArray oArgs;
	oArgs = oCmd.GetArguments();

	// Subdivision level
	oArgs.Add(L"ExportFile", XSI::siControlFilePath);

	return XSI::CStatus::OK;
}

XSIPLUGINCALLBACK XSI::CStatus A3DExport_Execute( XSI::CRef& in_ctxt )
{
	// Unpack the command argument values
	XSI::Context ctxt( in_ctxt );
	XSI::CValueArray args = ctxt.GetAttribute(L"Arguments");

	XSI::Application app;	

	XSITrans::C_ExportOptions options;

	XSI::CString filePath = args[0];

	//Are we performing a selection export?
	XSI::X3DObject xobj(app.GetSelection()[0]);
	if (xobj.IsValid()) 
		options.exportSelection = true;

	if (!Aftk::C_Global::Create())
		goto fail;

	{
		//ensure Aftk::C_Scene goes out of scope before Aftk::C_Global::Destroy();
		XSITrans::C_SceneExport exporter;
		if (!exporter.Export(options))
			goto fail;

		if (!exporter.GetScene().Write(filePath.GetAsciiString()))
			goto fail;
	}

	Aftk::C_Global::Destroy();
	return XSI::CStatus::OK;

fail:
	Aftk::C_Global::Destroy();
	return XSI::CStatus::Fail;
}