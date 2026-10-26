#ifndef __TEXTURE_EXPORT__HDRCOMPRESSION_H__
#define __TEXTURE_EXPORT__HDRCOMPRESSION_H__

namespace Texutils
{

/// compress an HDR texture
ACTRESULT HDRCompress5BPPBlur( ActTools::Texture* aTexture, ActTools::Texture*& pTextureLDR, ActTools::Texture*& pTextureHDR );
ACTRESULT HDRCompress5BPP( ActTools::Texture* aTexture, ActTools::Texture*& pTextureLDR, ActTools::Texture*& pTextureHDR );
ACTRESULT HDRCompress8BPP( ActTools::Texture* aTexture, ActTools::Texture*& pTextureLDR, ActTools::Texture*& pTextureHDR );

}

#endif