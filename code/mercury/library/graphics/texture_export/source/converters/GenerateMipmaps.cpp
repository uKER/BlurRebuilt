#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <minmax.h>

#include "texture_export/Surface.h"
#include "texture_export/Texture.h"
#include "texture_export/Rescale.h"

#include "texture_export/Converters/RescaleSurface.h"

using namespace ActTools;

namespace Texutils
{

// todo : support floating point textures
ACTRESULT GenerateMipmaps( Texture* aTexture,
						   ActTools::FilterType aColorFilter, float aColorBlur,
                           ActTools::FilterType aAlphaFilter, float aAlphaBlur,
						   ActTools::EdgeFilter aEdgeFilterX, ActTools::EdgeFilter aEdgeFilterY,
                           unsigned aNumMipmaps )
{
	ACTRESULT err = ACT_OK;

	// count the number of possible mips down to 1x1.
	unsigned num_mips = 1;
	size_t dimension = max( aTexture->GetWidth(), aTexture->GetHeight() );
	while( dimension > 1 )
	{
		++num_mips;
		dimension = max(1, (dimension+1)/2);
	}

	if ( aNumMipmaps && aNumMipmaps < num_mips )
		num_mips = aNumMipmaps;
	if( aTexture->GetNumMipMaps() >= num_mips )
		return ACT_OK;
	else if ( _stricmp( aTexture->GetFormat()->GetId(), "A8R8G8B8" ) )
		return ACT_ERROR;

	unsigned old_num_mips = aTexture->GetNumMipMaps();
	aTexture->AddMipmaps( num_mips - aTexture->GetNumMipMaps() );
	for( unsigned i_depth = 0; i_depth < aTexture->GetDepth(); ++i_depth )
	{
		Surface* level0_surface = aTexture->GetMipmap( i_depth, 0 );;

		size_t w = max(1, (level0_surface->GetWidth()+1)/2);
		size_t h = max(1, (level0_surface->GetHeight()+1)/2);

		for( unsigned i = old_num_mips; i < num_mips; ++i )
		{
			Surface* mip_surface = aTexture->GetMipmap( i_depth, i );
			mip_surface->Create( w, h );
			RescaleSurface( mip_surface, level0_surface, aColorFilter, aColorBlur, aAlphaFilter, aAlphaBlur, aEdgeFilterX, aEdgeFilterY );
			w = max(1, (w+1)/2);
			h = max(1, (h+1)/2);
		}
	}

	return err;
}

}// namespace texutils