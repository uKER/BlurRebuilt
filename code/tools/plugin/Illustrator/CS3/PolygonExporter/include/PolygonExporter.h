
#ifndef POLYGON_EXPORTER
#define POLYGON_EXPORTER

#pragma pack(push, 4)
#include "AIFileFormat.h"
#pragma pack(pop)

// TODO : Short this crap out!!!!
extern AIErr addFileFormat( SPInterfaceMessage *message );
extern AIErr goFileFormat( AIFileFormatMessage *message );

AIErr CreatePolygonFile( const char* pFileName, const F32 fCurveStep, const bool bFixedSpaceCurve, const bool bDisableFinning );

#endif // POLYGON_EXPORTER