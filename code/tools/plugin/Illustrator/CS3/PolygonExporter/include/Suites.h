
#ifndef SUITES_H
#define SUITES_H

#pragma pack(push, 4)
#include "ATETextSuitesImportHelper.h"
#include "AIPathFinder.h"
#pragma pack(pop)

typedef struct 
{	
	char *name;		
	long version;
	void **suite;
} ImportBunch;

// suite globals
extern "C" 
{

	SPBlocksSuite*			sSPBlocks			= NULL;
	SPAccessSuite*			sSPAccess			= NULL;

	AIFileFormatSuite*		sAIFileFormat		= NULL;
	AIArtSuite*				sAIArt				= NULL;
	AIPathSuite*			sAIPath				= NULL;
	AIMatchingArtSuite*		sAIMatchingArt		= NULL;

	ADMBasicSuite10*		sADMBasic			= NULL;
	ADMDialogSuite*			sADMDialog			= NULL;
	ADMItemSuite7*			sADMItem			= NULL;
	ADMNotifierSuite2*		sADMNotifier		= NULL;

	AIUnicodeStringSuite*	sAIUnicodeString	= NULL;
	AIFilePathSuite*		sAIFilePath			= NULL;
	AICustomColorSuite*		sAICustomColor		= NULL;
	AIPathStyleSuite*		sAIPathStyle		= NULL;
	AIGradientSuite*		sAIGradient			= NULL;
	AIBlendStyleSuite*		sAIBlendStyle		= NULL;
	AIPathfinderSuite*		sAIPathFinder		= NULL;

	AIAppContextSuite*		sAIAppContext		= NULL;
	AIPreferenceSuite*		sAIPreference		= NULL;
	ADMDialogGroupSuite*	sADMDialogGroup		= NULL;
	AINotifierSuite*		sAINotifier			= NULL;
	AIMenuSuite*			sAIMenu				= NULL;
	AIDocumentSuite*		sAIDocument			= NULL;
	AIDictionarySuite*		sAIDictionary		= NULL;

	ADMEntrySuite*			sADMEntry			= NULL;
	ADMIconSuite2*			sADMIcon2			= NULL;
	ADMListSuite*			sADMList			= NULL;
	ADMTrackerSuite*		sADMTracker			= NULL;

	EXTERN_TEXT_SUITES 
}

// property globals
ImportBunch imports[] = 
{
	kSPBlocksSuite,			kSPBlocksSuiteVersion,			(void**)&sSPBlocks,
	kSPAccessSuite,			kSPAccessSuiteVersion,			(void**)&sSPAccess,

	kAIFileFormatSuite,		kAIFileFormatVersion,			(void**)&sAIFileFormat,
	kAIArtSuite,			kAIArtSuiteVersion,				(void**)&sAIArt,
	kAIPathSuite,			kAIPathVersion,					(void**)&sAIPath,
	kAIMatchingArtSuite,	kAIMatchingArtVersion,			(void**)&sAIMatchingArt,
	kAIUnicodeStringSuite,	kAIUnicodeStringSuiteVersion,	(void**)&sAIUnicodeString,
	kAIFilePathSuite,		kAIFilePathSuiteVersion,		(void**)&sAIFilePath,

	kADMBasicSuite,			kADMBasicSuiteVersion10,		(void**)&sADMBasic,
	kADMDialogSuite,		kADMDialogSuiteVersion10,		(void**)&sADMDialog,
	kADMItemSuite,			kADMItemSuiteVersion7,			(void**)&sADMItem,
	kADMNotifierSuite,		kADMNotifierSuiteVersion2,		(void**)&sADMNotifier,

	kAICustomColorSuite,	kAICustomColorSuiteVersion,		(void**)&sAICustomColor,
	kAIPathStyleSuite,		kAIPathStyleVersion,			(void**)&sAIPathStyle,
	kAIGradientSuite,		kAIGradientSuiteVersion,		(void**)&sAIGradient,
	kAIBlendStyleSuite,		kAIBlendStyleVersion,			(void**)&sAIBlendStyle,
	kAIPathfinderSuite,		kAIPathfinderSuiteVersion,		(void**)&sAIPathFinder,

	kAIAppContextSuite,		kAIAppContextSuiteVersion,		(void**)&sAIAppContext,
	kAIPreferenceSuite,		kAIPreferenceSuiteVersion,		(void**)&sAIPreference,
	kADMDialogGroupSuite,	kADMDialogGroupSuiteVersion,	(void**)&sADMDialogGroup,
	kAINotifierSuite,		kAINotifierVersion,				(void**)&sAINotifier,
	kAIMenuSuite,			kAIMenuSuiteVersion,			(void**)&sAIMenu,
	kAIDocumentSuite,		kAIDocumentSuiteVersion,		(void**)&sAIDocument,
	kAIDictionarySuite,		kAIDictionarySuiteVersion,		(void**)&sAIDictionary,

	kADMEntrySuite,			kADMEntrySuiteVersion,			(void**)&sADMEntry,
	kADMTrackerSuite,		kADMTrackerSuiteVersion,		(void**)&sADMTracker,
	kADMIconSuite,			kADMIconSuiteVersion2,			(void**)&sADMIcon2,
	kADMListSuite,			kADMListSuiteVersion,			(void**)&sADMList,

	// I couldn't use the macro IMPORT_TEXT_SUITES because I need the (void**)
	kApplicationPaintSuite, kApplicationPaintSuiteVersion, (void**)&sApplicationPaint,\
		kCompFontSuite, kCompFontSuiteVersion, (void**)&sCompFont,\
		kCompFontClassSuite, kCompFontClassSuiteVersion, (void**)&sCompFontClass,\
		kCompFontClassSetSuite, kCompFontClassSetSuiteVersion, (void**)&sCompFontClassSet,\
		kCompFontComponentSuite, kCompFontComponentSuiteVersion, (void**)&sCompFontComponent,\
		kCompFontSetSuite, kCompFontSetSuiteVersion, (void**)&sCompFontSet,\
		kGlyphRunSuite, kGlyphRunSuiteVersion, (void**)&sGlyphRun,\
		kGlyphRunsIteratorSuite, kGlyphRunsIteratorSuiteVersion, (void**)&sGlyphRunsIterator,\
		kMojiKumiSuite, kMojiKumiSuiteVersion, (void**)&sMojiKumi,\
		kMojiKumiSetSuite, kMojiKumiSetSuiteVersion, (void**)&sMojiKumiSet,\
		kTextFrameSuite, kTextFrameSuiteVersion, (void**)&sTextFrame,\
		kTextFramesIteratorSuite, kTextFramesIteratorSuiteVersion, (void**)&sTextFramesIterator,\
		kTextLineSuite, kTextLineSuiteVersion, (void**)&sTextLine,\
		kTextLinesIteratorSuite, kTextLinesIteratorSuiteVersion, (void**)&sTextLinesIterator,\
		kTextResourcesSuite, kTextResourcesSuiteVersion, (void**)&sTextResources,\
		kApplicationTextResourcesSuite, kApplicationTextResourcesSuiteVersion, (void**)&sApplicationTextResources,\
		kDocumentTextResourcesSuite, kDocumentTextResourcesSuiteVersion, (void**)&sDocumentTextResources,\
		kArrayApplicationPaintRefSuite, kArrayApplicationPaintRefSuiteVersion, (void**)&sArrayApplicationPaintRef,\
		kArrayRealSuite, kArrayRealSuiteVersion, (void**)&sArrayReal,\
		kArrayBoolSuite, kArrayBoolSuiteVersion, (void**)&sArrayBool,\
		kArrayIntegerSuite, kArrayIntegerSuiteVersion, (void**)&sArrayInteger,\
		kArrayLineCapTypeSuite, kArrayLineCapTypeSuiteVersion, (void**)&sArrayLineCapType,\
		kArrayFigureStyleSuite, kArrayFigureStyleSuiteVersion, (void**)&sArrayFigureStyle,\
		kArrayLineJoinTypeSuite, kArrayLineJoinTypeSuiteVersion, (void**)&sArrayLineJoinType,\
		kArrayWariChuJustificationSuite, kArrayWariChuJustificationSuiteVersion, (void**)&sArrayWariChuJustification,\
		kArrayStyleRunAlignmentSuite, kArrayStyleRunAlignmentSuiteVersion, (void**)&sArrayStyleRunAlignment,\
		kArrayAutoKernTypeSuite, kArrayAutoKernTypeSuiteVersion, (void**)&sArrayAutoKernType,\
		kArrayBaselineDirectionSuite, kArrayBaselineDirectionSuiteVersion, (void**)&sArrayBaselineDirection,\
		kArrayLanguageSuite, kArrayLanguageSuiteVersion, (void**)&sArrayLanguage,\
		kArrayFontCapsOptionSuite, kArrayFontCapsOptionSuiteVersion, (void**)&sArrayFontCapsOption,\
		kArrayFontBaselineOptionSuite, kArrayFontBaselineOptionSuiteVersion, (void**)&sArrayFontBaselineOption,\
		/*	kArrayVerticalUnderlinePositionSuite, kArrayVerticalUnderlinePositionSuiteVersion, (void**)&sArrayVerticalUnderlinePosition,\*/
		kArrayParagraphJustificationSuite, kArrayParagraphJustificationSuiteVersion, (void**)&sArrayParagraphJustification,\
		kArrayArrayRealSuite, kArrayArrayRealSuiteVersion, (void**)&sArrayArrayReal,\
		kArrayBurasagariTypeSuite, kArrayBurasagariTypeSuiteVersion, (void**)&sArrayBurasagariType,\
		kArrayPreferredKinsokuOrderSuite, kArrayPreferredKinsokuOrderSuiteVersion, (void**)&sArrayPreferredKinsokuOrder,\
		kArrayKinsokuRefSuite, kArrayKinsokuRefSuiteVersion, (void**)&sArrayKinsokuRef,\
		kArrayMojiKumiRefSuite, kArrayMojiKumiRefSuiteVersion, (void**)&sArrayMojiKumiRef,\
		kArrayMojiKumiSetRefSuite, kArrayMojiKumiSetRefSuiteVersion, (void**)&sArrayMojiKumiSetRef,\
		kArrayTabStopsRefSuite, kArrayTabStopsRefSuiteVersion, (void**)&sArrayTabStopsRef,\
		kArrayLeadingTypeSuite, kArrayLeadingTypeSuiteVersion, (void**)&sArrayLeadingType,\
		kArrayFontRefSuite, kArrayFontRefSuiteVersion, (void**)&sArrayFontRef,\
		kArrayGlyphIDSuite, kArrayGlyphIDSuiteVersion, (void**)&sArrayGlyphID,\
		kArrayRealPointSuite, kArrayRealPointSuiteVersion, (void**)&sArrayRealPoint,\
		kArrayRealMatrixSuite, kArrayRealMatrixSuiteVersion, (void**)&sArrayRealMatrix,\
		kCharFeaturesSuite, kCharFeaturesSuiteVersion, (void**)&sCharFeatures,\
		kCharInspectorSuite, kCharInspectorSuiteVersion, (void**)&sCharInspector,\
		kCharStyleSuite, kCharStyleSuiteVersion, (void**)&sCharStyle,\
		kCharStylesSuite, kCharStylesSuiteVersion, (void**)&sCharStyles,\
		kCharStylesIteratorSuite, kCharStylesIteratorSuiteVersion, (void**)&sCharStylesIterator,\
		kFindSuite, kFindSuiteVersion, (void**)&sFind,\
		kFontSuite, kFontSuiteVersion, (void**)&sFont,\
		kGlyphSuite, kGlyphSuiteVersion, (void**)&sGlyph,\
		kGlyphsSuite, kGlyphsSuiteVersion, (void**)&sGlyphs,\
		kGlyphsIteratorSuite, kGlyphsIteratorSuiteVersion, (void**)&sGlyphsIterator,\
		kKinsokuSuite, kKinsokuSuiteVersion, (void**)&sKinsoku,\
		kKinsokuSetSuite, kKinsokuSetSuiteVersion, (void**)&sKinsokuSet,\
		kParaFeaturesSuite, kParaFeaturesSuiteVersion, (void**)&sParaFeatures,\
		kParagraphSuite, kParagraphSuiteVersion, (void**)&sParagraph,\
		kParagraphsIteratorSuite, kParagraphsIteratorSuiteVersion, (void**)&sParagraphsIterator,\
		kParaInspectorSuite, kParaInspectorSuiteVersion, (void**)&sParaInspector,\
		kParaStyleSuite, kParaStyleSuiteVersion, (void**)&sParaStyle,\
		kParaStylesSuite, kParaStylesSuiteVersion, (void**)&sParaStyles,\
		kParaStylesIteratorSuite, kParaStylesIteratorSuiteVersion, (void**)&sParaStylesIterator,\
		kSpellSuite, kSpellSuiteVersion, (void**)&sSpell,\
		kStoriesSuite, kStoriesSuiteVersion, (void**)&sStories,\
		kStorySuite, kStorySuiteVersion, (void**)&sStory,\
		kTabStopSuite, kTabStopSuiteVersion, (void**)&sTabStop,\
		kTabStopsSuite, kTabStopsSuiteVersion, (void**)&sTabStops,\
		kTabStopsIteratorSuite, kTabStopsIteratorSuiteVersion, (void**)&sTabStopsIterator,\
		kTextRangeSuite, kTextRangeSuiteVersion, (void**)&sTextRange,\
		kTextRangesSuite, kTextRangesSuiteVersion, (void**)&sTextRanges,\
		kTextRangesIteratorSuite, kTextRangesIteratorSuiteVersion, (void**)&sTextRangesIterator,\
		kTextRunsIteratorSuite, kTextRunsIteratorSuiteVersion, (void**)&sTextRunsIterator,\
		kWordsIteratorSuite, kWordsIteratorSuiteVersion, (void**)&sWordsIterator,\

};
#endif // SUITES_