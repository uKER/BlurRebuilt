
#include "Defines.h"

#include "DialogHandler.h"
#include "PolygonExporter.h"

#pragma pack(push, 4)
#include "ADMResource.h"
#pragma pack(pop)

#include "..\Resource.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

static void ASAPI ExportButtonUp( ADMItemRef item, ADMNotifierRef notifier);
static void ASAPI EditBoxChange( ADMItemRef item, ADMNotifierRef notifier);
static void ASAPI CheckBoxChange( ADMItemRef item, ADMNotifierRef notifier);


// Create the Non-modal dialog. This does not necessarily show the dialog on 
// the screen. If the dialog was hidden at last shutdown, it will not be shown 
// until sADMDialog->Show() is called. 
// Note: the init proc, DlgInit, will be called immediately following sADMDialog->Create()
AIErr createADMDialog( AINotifierMessage *message ) 
{
	AIErr error = kNoErr;

	gGlobals->nonModalDialog = sADMDialog->Create( message->d.self, "PolygonExporter", IDD_DIALOG1,
												   kADMTabbedFloatingDialogStyle, DlgInit, nil, 0 );

	return error;
}

// Initialize dialog items and notification procedures for the ADM dialog
ASErr ASAPI DlgInit(ADMDialogRef dlg)
{
	AIAppContextHandle		AppContext;
	SPPluginRef				pluginRef;

	// Set up the application context, so that suite calls will work
	pluginRef = sADMDialog->GetPluginRef(dlg);
	sAIAppContext->PushAppContext(pluginRef, &AppContext);

	// Acquire yourself to stay in memory
	sSPAccess->AcquirePlugin( pluginRef, &gGlobals->accessRef );
	
	// Acquire suites to stay in memory
	acquireSuites( gGlobals->pBasic );

	sADMDialog->SetDestroyProc( dlg, DlgDestroyProc );

	// Set initial states of items.
	//sADMDialog->Show( gGlobals->nonModalDialog, !sADMDialog->IsVisible( gGlobals->nonModalDialog ) );
	ADMItemRef editBox = sADMDialog->GetItem( dlg, IDC_EDIT1 );
	if ( editBox )	
	{
		sADMItem->SetFloatValue( editBox, DEFAULT_CURVE_STEP );
		sADMItem->SetNotifyProc( editBox, EditBoxChange );
	}

	ADMItemRef checkBox1 = sADMDialog->GetItem( dlg, IDC_CHECK1 );
	if ( checkBox1 )	
	{
		sADMItem->SetBooleanValue( checkBox1, false );
		sADMItem->SetNotifyProc( checkBox1, CheckBoxChange );
	}

	ADMItemRef checkBox2 = sADMDialog->GetItem( dlg, IDC_CHECK2 );
	if ( checkBox2 )	
	{
		sADMItem->SetBooleanValue( checkBox2, false );
		sADMItem->SetNotifyProc( checkBox2, CheckBoxChange );
	}

	// Attach the callbacks for the beep buttons
	ADMItemRef buttonRef = sADMDialog->GetItem( dlg, IDC_BUTTON1 );
	if ( buttonRef )
		sADMItem->SetNotifyProc( buttonRef, ExportButtonUp );

	/*--------------------------------------------------------------------------------------
		Reset the position
	--------------------------------------------------------------------------------------*/

	char		groupName[64];
	ADMPoint	location, size;
	ADMRect		rect, dimensions, boundsRect;
	long		positionCode = 0x00001c00; // Default: no dock, no tab group, front tab, zoom down, visible

	// Get the last known Docking group and Docking code out of the Prefs file
	//sASLib->strcpy( groupName, kLayersPaletteDockGroup ); // is this a default value?
	sAIPreference->GetStringPreference( "PolygonExporter", "kIPF_DPDockGroupStr", groupName );
	sAIPreference->GetIntegerPreference( "PolygonExporter", "kIPF_DPDockCodeStr", &positionCode );

	// Pick a default location in case it has never come up before on this machine
	sADMDialog->GetBoundsRect( dlg, &boundsRect );
	sADMBasic->GetPaletteLayoutBounds( &dimensions );

	// Get the last known location out of the Prefs file
	location.h = dimensions.right - (boundsRect.right - boundsRect.left);
	location.v = dimensions.bottom - (boundsRect.bottom - boundsRect.top);
	sAIPreference->GetPointPreference( "PolygonExporter", "kIPF_DPLocationStr", &location );

	// Get the last known size out of the Prefs file
	size.h = 180;	// minimum width (which governs the inner client rect)
	size.v = 120;
	sAIPreference->GetPointPreference( "PolygonExporter", "kIPF_DPSizeStr", &size );

	rect.left = location.h;
	rect.right = location.h + size.h;
	rect.top = location.v;
	rect.bottom = location.v + size.v;

	// restore the size and location of the dialog
	sADMDialog->SetBoundsRect(dlg, &rect);
	// restore the position code of the dialog
	sADMDialogGroup->SetDialogGroupInfo(dlg, groupName, positionCode );

	// Clean up the application context and return
	sAIAppContext->PopAppContext(AppContext);

	return kNoErr;
}


static void ASAPI DlgDestroyProc(ADMDialogRef dlg) 
{
	ADMRect		boundsRect;
	ADMPoint 	location, size;
	const char* pGroupName;
	long		positionCode;

	// saving the palette position, tab/dock preference.
	sADMDialogGroup->GetDialogGroupInfo( dlg, &pGroupName, &positionCode );
	sADMDialog->GetBoundsRect( dlg, &boundsRect );

	location.h = boundsRect.left;
	location.v = boundsRect.top;
	size.h = boundsRect.right - boundsRect.left;
	size.v = boundsRect.bottom - boundsRect.top; 

	sAIPreference->PutIntegerPreference( "PolygonExporter", "kIPF_DPDockCodeStr", positionCode );
	sAIPreference->PutStringPreference( "PolygonExporter", "kIPF_DPDockGroupStr", pGroupName );
	sAIPreference->PutPointPreference( "PolygonExporter", "kIPF_DPLocationStr", &location );
	sAIPreference->PutPointPreference( "PolygonExporter", "kIPF_DPSizeStr", &size );

	gGlobals->nonModalDialog = NULL;
	sSPAccess->ReleasePlugin( gGlobals->accessRef );
	
	// release suites from memory
	releaseSuites( gGlobals->pBasic );	
}


static void ASAPI ExportButtonUp( ADMItemRef item, ADMNotifierRef notifier )
{
	// dispatch the notifier type
	if ( sADMNotifier->IsNotifierType( notifier, kADMUserChangedNotifier ) )
	{
		SPPlatformFileSpecification startingDir;
		SPPlatformFileSpecification resultFile;

		static ADMPlatformFileTypesSpecification3 filter = 
		{
			"Illustrator Polygon File (*.ipf)\0"
			"*.ipf\0"
		};
		ADMBoolean result = sADMBasic->StandardPutFileDialog( "Export Filename", &filter, &startingDir, "", &resultFile );

		// check that user wants to save?
		if ( result )
		{
			ADMItemRef editBox = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_EDIT1 );
			ADMItemRef checkBox1 = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_CHECK1 );
			ADMItemRef checkBox2 = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_CHECK2 );

			F32 curveStep = ( editBox ) ? sADMItem->GetFloatValue( editBox ) : DEFAULT_CURVE_STEP;
			bool fixedSpaceSpline = ( checkBox1 ) ? ( ( sADMItem->GetBooleanValue( checkBox1 ) ) ? true : false ) : false;
			bool disableFinning = ( checkBox2 ) ? ( ( sADMItem->GetBooleanValue( checkBox2 ) ) ? true : false ) : false;

			CreatePolygonFile( resultFile.path, curveStep, fixedSpaceSpline, disableFinning );
		}

		// Dirty the doc so artists can save the export settings.
		sAIDocument->SetDocumentModified( true );
	}
}


static void ASAPI EditBoxChange( ADMItemRef item, ADMNotifierRef notifier ) 
{
	// dispatch the notifier type
	if ( sADMNotifier->IsNotifierType(notifier, kADMUserChangedNotifier) ) 
	{
		// Dirty the doc so artists can save the export settings.
		sAIDocument->SetDocumentModified( true );
	}
}

static void ASAPI CheckBoxChange( ADMItemRef item, ADMNotifierRef notifier ) 
{
	// dispatch the notifier type
	if ( sADMNotifier->IsNotifierType(notifier, kADMUserChangedNotifier) ) 
	{
		// Dirty the doc so artists can save the export settings.
		sAIDocument->SetDocumentModified( true );
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", on ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

