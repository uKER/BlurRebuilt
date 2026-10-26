
#ifndef SUITES_H
#define SUITES_H

#include "ATETextSuitesImportHelper.h"
#include "AIPathFinder.h"

typedef struct 
{	
	char *name;		
	long version;
	void **suite;
} ImportBunch;

// suite globals
SPBlocksSuite *sSPBlocks = NULL;
SPAccessSuite *sSPAccess = NULL;

AIMdMemorySuite*		sAIMDMemory = NULL;
AIUserSuite*			sAIUser = NULL;
AIArtSetSuite*			sAIArtSet = NULL;
AIFileFormatSuite*		sAIFileFormat = NULL;
AIDocumentSuite*		sAIDocument = NULL;
AITextFrameSuite*		sAITextFrame = NULL;
AIArtSuite*				sAIArt = NULL;
AIArtStyleSuite*		sAIArtStyleSuite = NULL;
AIPathSuite*			sAIPath = NULL;
AICustomColorSuite*		sAICustomColor = NULL;
AIPathStyleSuite*		sAIPathStyle = NULL;
AIGradientSuite*		sAIGradient = NULL;
AIBlendStyleSuite*		sAIBlendStyle = NULL;
AIExpandSuite*			sAIExpandSuite = NULL;
AIPathfinderSuite*		sAIPathFinder = NULL;
AIMatchingArtSuite*		sAIMatchingArt = NULL;
AIMdMemorySuite*		sAIMdMemory = NULL;

ADMBasicSuite *sADMBasic = NULL;
ADMDialogSuite5 *sADMDialog = NULL;
ADMItemSuite7 *sADMItem = NULL;

extern "C" { EXTERN_TEXT_SUITES }

// property globals
ImportBunch imports[] = {

	kSPBlocksSuite, kSPBlocksSuiteVersion, (void**)&sSPBlocks,
	kSPAccessSuite, kSPAccessSuiteVersion, (void**)&sSPAccess,
	kAIMdMemorySuite, kAIMdMemorySuiteVersion, (void**)&sAIMDMemory,
	kAIUserSuite, kAIUserSuiteVersion, (void**)&sAIUser,
	kAIArtSetSuite, kAIArtSetSuiteVersion, (void**)&sAIArtSet,
	kAIFileFormatSuite, kAIFileFormatVersion, (void**)&sAIFileFormat,
	kAIDocumentSuite, kAIDocumentVersion, (void**)&sAIDocument,
	kAITextFrameSuite, kAITextFrameVersion, (void**)&sAITextFrame,
	kAIArtSuite, kAIArtSuiteVersion, (void**)&sAIArt,
	kAIExpandSuite, kAIExpandSuiteVersion, (void**)&sAIExpandSuite,
	kAIArtStyleSuite, kAIArtStyleSuiteVersion, (void**)&sAIArtStyleSuite,
	kAIPathSuite, kAIPathVersion, (void**)&sAIPath,
	kAICustomColorSuite, kAICustomColorSuiteVersion, (void**)&sAICustomColor,
	kAIPathfinderSuite, kAIPathfinderSuiteVersion, (void**)&sAIPathFinder,
	kAIPathStyleSuite, kAIPathStyleVersion, (void**)&sAIPathStyle,
	kAIGradientSuite, kAIGradientSuiteVersion, (void**)&sAIGradient,
	kAIBlendStyleSuite, kAIBlendStyleVersion, (void**)&sAIBlendStyle,
	kAIMatchingArtSuite, kAIMatchingArtVersion, (void**)&sAIMatchingArt,
	kAIMdMemorySuite, kAIMdMemoryVersion, (void**)&sAIMdMemory,

	kADMBasicSuite, kADMBasicSuiteVersion, (void**)&sADMBasic,
	kADMDialogSuite, kADMDialogSuiteVersion5, (void**)&sADMDialog,
	kADMItemSuite, kADMItemSuiteVersion7, (void**)&sADMItem,

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
#endif // SUITES_H