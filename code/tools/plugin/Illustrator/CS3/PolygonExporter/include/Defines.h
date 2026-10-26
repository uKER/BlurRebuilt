
#ifndef DEFINES_H
#define DEFINES_H


#include <core/Core.h>
#include <core/Maths.h>
#include <core/containers/string.h>

#pragma pack(push, 4)
#include "IllustratorSDK.h"
#include "AIPathFinder.h"
#include "AIExpand.h"
#include "AIContext.h"
#include "IAIUnicodeString.h"
#pragma pack(pop)

#define DEFAULT_CURVE_STEP		6.0f
#define MAX_SEGMENTS			8192
#define	EXCLUDE_LAYER_NAME		"<exclude>"

#ifndef GLOBALSTRUCT
#define GLOBALSTRUCT

typedef struct 
{
	SPBasicSuite*		pBasic;

	AIFileFormatHandle	fileFormatSelected;
	AIMenuItemHandle	aboutPluginMenu;
	AIMenuItemHandle	showExportOptions;	

	ADMDialogRef		nonModalDialog;
	SPAccessRef			accessRef;
	AINotifierHandle	appStartedNotifier;

	AINotifierHandle	openedNotifier;
	AINotifierHandle	savingNotifier;
} Globals;

#endif


// general globals
extern Globals *gGlobals;

// suite stuff
extern "C" SPAccessSuite*		sSPAccess;

extern "C" AIFileFormatSuite*	sAIFileFormat;

extern "C" AIArtSuite*			sAIArt;
extern "C" AIArtStyleSuite*		sAIArtStyleSuite;
extern "C" AIExpandSuite*		sAIExpandSuite;
extern "C" AIPathSuite*			sAIPath;
extern "C" AIPathStyleSuite*	sAIPathStyle;
extern "C" AIGradientSuite*		sAIGradient;
extern "C" AIBlendStyleSuite*	sAIBlendStyle;
extern "C" AICustomColorSuite*	sAICustomColor;
extern "C" AIPathfinderSuite*	sAIPathFinder;
extern "C" AIMatchingArtSuite*	sAIMatchingArt;

extern "C" ADMBasicSuite10*		sADMBasic;
extern "C" ADMDialogSuite*		sADMDialog;
extern "C" ADMItemSuite7*		sADMItem;
extern "C" ADMNotifierSuite2*	sADMNotifier;

extern "C" AIAppContextSuite*	sAIAppContext;
extern "C" AIPreferenceSuite*	sAIPreference;
extern "C" ADMDialogGroupSuite*	sADMDialogGroup;
extern "C" AINotifierSuite*		sAINotifier;
extern "C" AIMenuSuite*			sAIMenu;
extern "C" AIDocumentSuite*		sAIDocument;
extern "C" AIDictionarySuite*	sAIDictionary;

extern "C" ADMEntrySuite*		sADMEntry;
extern "C" ADMIconSuite2*		sADMIcon2;
extern "C" ADMListSuite*		sADMList;
extern "C" ADMTrackerSuite*		sADMTracker;

/**-----------------------------------------------------------------------------
**
**	suite support
**
**/
extern	AIErr acquireSuites( SPBasicSuite *sBasic );
extern	AIErr releaseSuites( SPBasicSuite *sBasic );


#endif // DEFINES_H