#include "XSIA3DExportPCH.h"

//*****************************************************************************
/*!	\file importexport_prop.cpp
 	\brief Defines the callbacks that define the layout and behavior of the 
 	Import Export Demo property page.

	(c) Copyright 1996 - 2005 Softimage/Avid Corporation
 
 */
//*****************************************************************************

#include <xsi_application.h>
#include <xsi_model.h>
#include <xsi_customproperty.h>
#include <xsi_parameter.h>
#include <xsi_string.h>
#include <xsi_griddata.h>
#include <xsi_value.h>
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>

#define LABEL_MIN 140
#define LABEL_RATIO 40


XSI::CustomProperty GetA3DExportProp()
{	
	XSI::Application app;
	XSI::Model root = app.GetActiveSceneRoot();
	XSI::CustomProperty prop = root.GetProperties().GetItem(L"A3DExportProp");
	if (!prop.IsValid())
	{
		prop = root.AddProperty( L"A3DExportProp" );
	}
	return prop;
}

XSI::Parameter GetImportExportOption( const XSI::CString& in_strName )
{
	XSI::CustomProperty pset = GetA3DExportProp();
	if (!pset.IsValid()) return XSI::CRef();
	
	XSI::Parameter param = pset.GetParameters().GetItem(in_strName);
	
	if (!param.IsValid())
	{	
		pset.AddParameter(
			in_strName,
			XSI::CValue::siInt4,
			XSI::siReadOnly,
			L"",
			L"",
			(LONG)0,
			(LONG)0,
			(LONG)100,
			(LONG)0,
			(LONG)100,
			param);	
	}
	return param;
}

//*****************************************************************************
/*!	Callback that defines the parameters (options) of the custom property.
	\param in_ctxt The context that encapsulates the custom property to initialize.
 */
//*****************************************************************************

XSIPLUGINCALLBACK
XSI::CStatus A3DExportProp_Define( XSI::CRef& in_ctxt )
{
	XSI::Context ctxt( in_ctxt );
	XSI::CustomProperty pset( ctxt.GetSource() );
	
	XSI::Parameter param;
	pset.AddParameter(L"ExportFile",XSI::CValue::siString, XSI::siPersistable, L"", L"", L"", param );

	XSI::Application app;
	XSI::CString strDefaultFile( app.GetInstallationPath( XSI::siUserPath ) );

	strDefaultFile += L"";

	param.PutValue( strDefaultFile );
	
	pset.AddParameter(L"ImportFile",XSI::CValue::siString, XSI::siPersistable, L"", L"", L"", param );
	param.PutValue( strDefaultFile );	

	return XSI::CStatus::OK;
}

//*****************************************************************************
/*!	Callback that defines the layout of controls on the property page.
	\param in_ctxt The context that encapsulates the custom property layout to 
	initialize.
 */
//*****************************************************************************
XSIPLUGINCALLBACK
XSI::CStatus A3DExportProp_DefineLayout( XSI::CRef& in_ctxt )
{	
	XSI::Context ctxt(in_ctxt);
	XSI::PPGLayout ppg = ctxt.GetSource() ;

	ppg.Clear();

	// define the export section
	ppg.AddGroup(L"Export");
	
	LONG labelMinPixels = LABEL_MIN;
	LONG labelPercentage = LABEL_RATIO;

	XSI::PPGItem item = ppg.AddButton( L"ExportMeshButton", L"Export" ) ;
	item.PutAttribute( XSI::siUICX, (LONG)100 );

	item = ppg.AddItem( L"ExportFile", L"Export Filename", XSI::siControlFilePath);
	item.PutAttribute( L"initialdir", L"project");
	item.PutAttribute( XSI::siUIFileFilter, L"Abstract3D (*.a3d)|*.a3d|All Files (*.*)|*.*||" ) ;
//	item.PutAttribute( XSI::siUINoLabel, true );	
	item.PutLabelMinPixels( labelMinPixels*3 );
 	item.PutLabelPercentage( labelPercentage );
	
	ppg.EndRow();
	ppg.EndGroup();
/*	
	// define the import section
	ppg.AddGroup(L"Import");

	item = ppg.AddButton( L"ImportPolygonMeshButton", L"Import" ) ;
	item.PutAttribute( XSI::siUICX, (LONG)100 );

	item = ppg.AddItem( L"ImportFile" ) ;
	item.PutAttribute( XSI::siUINoLabel, true );	
	item.PutLabelMinPixels( labelMinPixels );
 	item.PutLabelPercentage( labelPercentage );
	
	ppg.EndRow();
	ppg.EndGroup();
*/	
	return XSI::CStatus::OK;
}

//*****************************************************************************
/*!	Callback that handles UI events (such as button clicks) on property page.
	page.
	\param in_ctxt The context that encapsulates the custom property property
	page event.
 */
//*****************************************************************************

XSIPLUGINCALLBACK
XSI::CStatus A3DExportProp_PPGEvent( const XSI::CRef& in_ctxt )
{	
	XSI::Application app;

	XSI::PPGEventContext ctxt( in_ctxt ) ;

	XSI::PPGEventContext::PPGEvent eventID = ctxt.GetEventID() ;
	if ( eventID == XSI::PPGEventContext::siButtonClicked )
	{
		XSI::CValue buttonPressed = ctxt.GetAttribute( L"Button" ) ;	
		
		if ( buttonPressed.GetAsText() == L"ExportMeshButton" )
		{
			XSI::CustomProperty in_pset = ctxt.GetSource();
			
			XSI::CValueArray args(1);
			args[0] = in_pset.GetParameterValue( L"ExportFile" );
			
			XSI::CValue retVal;
			XSI::CStatus status = app.ExecuteCommand( L"A3DExport", args, retVal ) ;
			return status;
		}
		else if ( buttonPressed.GetAsText() == L"ImportPolygonMeshButton" )
		{	
			XSI::CustomProperty in_pset = ctxt.GetSource();

			XSI::CValueArray args(1);
			args[0] = in_pset.GetParameterValue( L"ImportFile" );

			XSI::CValue retVal;
			XSI::CStatus status = app.ExecuteCommand( L"A3DImport", args, retVal ) ;
			return status;
		}
	}

	return XSI::CStatus::OK ;
}
