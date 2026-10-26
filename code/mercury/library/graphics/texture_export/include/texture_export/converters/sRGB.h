#ifndef __TEXTURE_EXPORT__SRGB_H__
#define __TEXTURE_EXPORT__SRGB_H__

namespace Texutils
{

/// convert the given texture from linear (A8R8G8B8) to sRGB (Alphas are preserved)
ACTRESULT LinearToSRGB( ActTools::Texture* aTexture);

/// convert the given texture from sRGB (A8R8G8B8) to linear (Alphas are preserved)
ACTRESULT SRGBToLinear( ActTools::Texture* aTexture);

}

#endif
