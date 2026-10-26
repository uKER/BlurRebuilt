
#ifndef DEFINES_H
#define DEFINES_H


#include <core/Core.h>
#include <core/Containers.h>
#include <core/Maths.h>

#include "IllustratorSDK.h"

#include "AIPathFinder.h"
#include "AIExpand.h"

#ifndef GLOBALSTRUCT
#define GLOBALSTRUCT

typedef struct 
{
	AIFileFormatHandle fileFormatAll;
	AIFileFormatHandle fileFormatSelected;
} Globals;

#endif


// general globals
extern Globals *g;
extern SPAccessRef gSelfAccess;
extern SPAccessSuite *sSPAccess;
extern "C" SPBasicSuite *sSPBasic;

// suite stuff
extern SPBlocksSuite *sSPBlocks;
extern SPAccessSuite *sSPAccess;

extern AIMdMemorySuite *sAIMDMemory;
extern AIUserSuite *sAIUser;
extern AIArtSetSuite *sAIArtSet;
extern AIFileFormatSuite *sAIFileFormat;
extern AIDocumentSuite *sAIDocument;
extern AITextFrameSuite	*sAITextFrame;
extern AIArtSuite *sAIArt;
extern AIArtStyleSuite *sAIArtStyleSuite;
extern AIExpandSuite	*sAIExpandSuite;
extern AIPathSuite *sAIPath;
extern AIPathStyleSuite *sAIPathStyle;
extern AIGradientSuite *sAIGradient;
extern AIBlendStyleSuite *sAIBlendStyle;
extern AICustomColorSuite *sAICustomColor;
extern AIPathfinderSuite *sAIPathFinder;
extern AIMatchingArtSuite *sAIMatchingArt;
extern AIMdMemorySuite *sAIMdMemory;

extern ADMBasicSuite *sADMBasic;
extern "C" ADMDialogSuite5 *sADMDialog;
extern "C" ADMItemSuite7	*sADMItem;


/**-----------------------------------------------------------------------------
**
**	suite support
**
**/
extern	AIErr acquireSuites( SPBasicSuite *sBasic );
extern	AIErr releaseSuites( SPBasicSuite *sBasic );


#endif // DEFINES_H