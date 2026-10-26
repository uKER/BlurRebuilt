#ifndef __TEXTURE_EXPORT_GEN_MIPMAPS_H__
#define __TEXTURE_EXPORT_GEN_MIPMAPS_H__

#include "texture_export/rescale.h"

namespace Texutils
{

ACTRESULT GenerateMipmaps( ActTools::Texture* aTexture,
						   ActTools::FilterType aColorFilter, float aColorBlur,
                           ActTools::FilterType aAlphaFilter, float aAlphaBlur,
						   ActTools::EdgeFilter aEdgeFilterX, ActTools::EdgeFilter aEdgeFilterY,
                           unsigned aNumMipmaps );

}

#endif

