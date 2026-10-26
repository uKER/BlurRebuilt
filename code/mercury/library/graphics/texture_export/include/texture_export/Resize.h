#ifndef RESIZE_H__INCLUDED
#define RESIZE_H__INCLUDED

#include "ActToolsLib/defs.h"

#define CSTR_UNDEFINED    "UNDEFINED"
#define CSTR_TopLeft      "TopLeft"
#define CSTR_TopMiddle    "TopMiddle"
#define CSTR_TopRight     "TopRight"

#define CSTR_MiddleLeft   "MiddleLeft"
#define CSTR_MiddleMiddle "MiddleMiddle"
#define CSTR_MiddleRight  "MiddleRight"

#define CSTR_BottomLeft   "BottomLeft"
#define CSTR_BottomMiddle "BottomMiddle"
#define CSTR_BottomRight  "BottomRight"

namespace ActTools 
{

class Surface;


enum AnchorType
{
	AT_TOPLEFT = 1,
	AT_TOPMIDDLE,
	AT_TOPRIGHT,

	AT_MIDDLELEFT,
	AT_MIDDLEMIDDLE,
	AT_MIDDLERIGHT,

	AT_BOTTOMLEFT,
	AT_BOTTOMMIDDLE,
	AT_BOTTOMRIGHT,

	AT_INVALID = -1,

	AT_FIRST = AT_TOPLEFT,
	AT_LAST = AT_BOTTOMRIGHT,
};

ActTools::AnchorType GetAnchorType( const char* aAnchorName );
const char* GetAnchorName( int aAnchor );
ACTRESULT ResizeA8R8G8B8( u8* pDstImage, size_t aDstWidth, size_t aDstHeight,
                    const u8* pSrcImage, size_t aSrcWidth, size_t aSrcHeight,
                    ActTools::AnchorType aAnchor,
                    unsigned aRed = 255, unsigned aGreen = 255, unsigned aBlue = 255, unsigned aAlpha = 255 );

ACTRESULT ResizeSurface( ActTools::Surface* aDstSurface,
                   const ActTools::Surface* aSrcSurface,
                   ActTools::AnchorType aAnchor,
                   unsigned aRed = 255, unsigned aGreen = 255, unsigned aBlue = 255, unsigned aAlpha = 255 );

}

#endif //RESIZE_H__INCLUDED

