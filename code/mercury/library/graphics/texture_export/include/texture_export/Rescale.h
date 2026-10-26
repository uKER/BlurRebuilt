#ifndef RESCALE_H__INCLUDED
#define RESCALE_H__INCLUDED

#include "ngl/defs.h"

#define CSTR_UNDEFINED   "UNDEFINED"
#define CSTR_POINT       "POINT"
#define CSTR_BOX         "BOX"
#define CSTR_TRIANGLE    "TRIANGLE"
#define CSTR_HERMITE     "HERMITE"
#define CSTR_HANNING     "HANNING"
#define CSTR_HAMMING     "HAMMING"
#define CSTR_BLACKMAN    "BLACKMAN"
#define CSTR_GAUSSIAN    "GAUSSIAN"
#define CSTR_QUADRATIC   "QUADRATIC"
#define CSTR_CUBIC       "CUBIC"
#define CSTR_CATROM      "CATROM"
#define CSTR_MITCHELL    "MITCHELL"
#define CSTR_LANCZOS     "LANCZOS"
#define CSTR_BESSEL      "BESSEL"
#define CSTR_SINC        "SINC"

namespace ActTools 
{

enum FilterType
{
	FT_UNDEFINED,
	FT_POINT,
	FT_BOX,
	FT_TRIANGLE,
	FT_HERMITE,
	FT_HANNING,
	FT_HAMMING,
	FT_BLACKMAN,
	FT_GAUSSIAN,
	FT_QUADRATIC,
	FT_CUBIC,
	FT_CATROM,
	FT_MITCHELL,
	FT_LANCZOS,
	FT_BESSEL,
	FT_SINC,

	FT_INVALID = -1,

	FT_FIRST = FT_POINT,
	FT_LAST = FT_SINC,
};

enum EdgeFilter
{
	EDGE_CLAMP = 1,
	EDGE_MIRROR,
	EDGE_TILE,

	EDGE_INVALID = -1
};

ActTools::FilterType GetFilterType( const char* aFilterName );
const char* GetFilterName( int aFilter );
ACTRESULT RescaleA8R8G8B8( u8* pDstImage, int iDstWidth, int iDstHeight,
                     const u8* pSrcImage, int iSrcWidth, int iSrcHeight,
                     ActTools::FilterType aColorFilter, double aColorBlur,
                     ActTools::FilterType aAlphaFilter, double aAlphaBlur,
                     ActTools::EdgeFilter aEdgeFilterX, ActTools::EdgeFilter aEdgeFilterY );

class Surface;

ACTRESULT RescaleSurface( ActTools::Surface* aDstSurface,
                    const ActTools::Surface* aSrcSurface,
                    ActTools::FilterType aColorFilter, double aColorBlur,
                    ActTools::FilterType aAlphaFilter, double aAlphaBlur,
                    ActTools::EdgeFilter aEdgeFilterX, ActTools::EdgeFilter aEdgeFilterY );

}

#endif //RESCALE_H__INCLUDED

