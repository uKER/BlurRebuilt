#ifndef __TEXTURE_EXPORT__HEIGHTMAPTONORMALMAP_H__
#define __TEXTURE_EXPORT__HEIGHTMAPTONORMALMAP_H__

namespace Texutils
{

/// convert the given texture from a heightmap to a normal map
ACTRESULT HeightMapToNormalMap( ActTools::Texture* aTexture );

/// invert the texture's red channel
ACTRESULT InvertNormalMapX( ActTools::Texture* aTexture );

/// premultiply alpha
ACTRESULT PreMultiplyAlpha( ActTools::Texture* aTexture );

}

#endif
