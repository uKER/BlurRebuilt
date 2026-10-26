#ifndef __TEXTURE_EXPORT__INR0_H__
#define __TEXTURE_EXPORT__INR0_H__

namespace Texutils
{

/// convert the given texture from an IPR0 to an INR0, given a neon texture
ACTRESULT Ipr0ToInr0( ActTools::Texture* aTexture, ActTools::Texture* aNeonTexture );

}

#endif
