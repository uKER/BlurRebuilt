
#include "Defines.h"

#include "..\Resource.h"

#include "NotifierHandler.h"
#include "DialogHandler.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

extern AIErr addNotifiers( SPInterfaceMessage *message ) 
{
	AIErr error = kNoErr;

	// This notifier is needed because we create ADM dialogs when we recieve this notifier
	error = sAINotifier->AddNotifier ( message->d.self, "notifierName", kAIApplicationStartedNotifier, &gGlobals->appStartedNotifier );

	error = sAINotifier->AddNotifier ( message->d.self, "notifierName", kAIDocumentOpenedNotifier, &gGlobals->openedNotifier );
	error = sAINotifier->AddNotifier ( message->d.self, "notifierName", kAIDocumentWritePreprocessNotifier, &gGlobals->savingNotifier );

	return error;
}


AIErr goNotifier( AINotifierMessage *message ) 
{
	AIErr error = kNoErr;

	if ( message->notifier == gGlobals->appStartedNotifier ) 
	{
		// Creates an ADM Non-Modal dialog, but does not necessarily show it
		createADMDialog( message );
	}
	else
	if ( message->notifier == gGlobals->openedNotifier )
	{
		ai::FilePath filePath;
		sAIDocument->GetDocumentFileSpecification( filePath );

		// Called just before data is saved.
		AIDictionaryRef ref;
		AIErr error = sAIDocument->GetNonRecordedDictionary( &ref );

		AIDictKey key1 = sAIDictionary->Key( "CurveStep" );
		AIDictKey key2 = sAIDictionary->Key( "FixedSpaceCurve" );
		AIDictKey key3 = sAIDictionary->Key( "DisableFinning" );

		// Could be a new doc or first time loading so enties might not exist so want to use defaults.
		if ( sAIDictionary->IsKnown( ref, key1 ) && sAIDictionary->IsKnown( ref, key2 ) && sAIDictionary->IsKnown( ref, key3 ) )
		{
			ASReal curveStep;
			ASBoolean fixedSpaceSpline;
			ASBoolean disableFinning;

			sAIDictionary->GetRealEntry( ref, key1, &curveStep );
			sAIDictionary->GetBooleanEntry( ref, key2, &fixedSpaceSpline );
			sAIDictionary->GetBooleanEntry( ref, key3, &disableFinning );

			// Update the export options from the settings.
			ADMItemRef editBox = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_EDIT1 );
			ADMItemRef checkBox1 = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_CHECK1 );
			ADMItemRef checkBox2 = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_CHECK2 );

			sADMItem->SetFloatValue( editBox, (F32)curveStep );
			sADMItem->SetBooleanValue( checkBox1, fixedSpaceSpline );
			sADMItem->SetBooleanValue( checkBox2, disableFinning );
		}

		sAIDictionary->Release( ref );
	}
	else
	if ( message->notifier == gGlobals->savingNotifier )
	{
		// Called just before data is saved.
		AIDictionaryRef ref;
		AIErr error = sAIDocument->GetNonRecordedDictionary( &ref );

		AIDictKey key1 = sAIDictionary->Key( "CurveStep" );
		AIDictKey key2 = sAIDictionary->Key( "FixedSpaceCurve" );
		AIDictKey key3 = sAIDictionary->Key( "DisableFinning" );

		// Save the export options to the document.
		ADMItemRef editBox = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_EDIT1 );
		ADMItemRef checkBox1 = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_CHECK1 );
		ADMItemRef checkBox2 = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_CHECK2 );

		F32 curveStep = ( editBox ) ? sADMItem->GetFloatValue( editBox ) : DEFAULT_CURVE_STEP;
		bool fixedSpaceSpline = ( checkBox1 ) ? ( ( sADMItem->GetBooleanValue( checkBox1 ) ) ? true : false ) : false;
		bool disableFinning = ( checkBox2 ) ? ( ( sADMItem->GetBooleanValue( checkBox2 ) ) ? true : false ) : false;

		sAIDictionary->SetRealEntry( ref, key1, (ASReal)curveStep );
		sAIDictionary->SetBooleanEntry( ref, key2, fixedSpaceSpline );
		sAIDictionary->SetBooleanEntry( ref, key3, disableFinning );

		sAIDictionary->Release( ref );
	}
	return error;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", on ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif