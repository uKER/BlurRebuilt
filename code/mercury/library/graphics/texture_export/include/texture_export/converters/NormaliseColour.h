#ifndef __TEXTURE_EXPORT__NORMALISECOLOUR_H__
#define __TEXTURE_EXPORT__NORMALISECOLOUR_H__

namespace Texutils
{

/// convert the given RGB texture into an RGBA, with a normalised RGB and length A
ACTRESULT NormaliseColour( ActTools::Texture* aTexture );

}

#endif