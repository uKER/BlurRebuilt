#include "stdafx.h"

#include <stdio.h>
#include <string.h>

#include "texture_export/Texture.h"
#include "texture_export/Surface.h"
#include "texture_export/Rescale.h"

#include "texture_export/rescale.h"
#include "texture_export/converters/RescaleSurface.h"

using namespace ActTools;

namespace Texutils
{

ACTRESULT RescaleSurface( Surface* aDstSurface,
                          const Surface* aSrcSurface,
                          const char* aColorFilter, float aColorBlur,
                          const char* aAlphaFilter, float aAlphaBlur,
                          const char* aEdgeFilterX, const char* aEdgeFilterY )
{
	FilterType color_filter_type = ActTools::GetFilterType( aColorFilter );
	FilterType alpha_filter_type = ActTools::GetFilterType( aAlphaFilter );

	const char* edge_filter_xy_cstr[2] = { aEdgeFilterX, aEdgeFilterY };
	EdgeFilter edge_filter_xy[2];
	for ( int i = 0; i < 2; ++i )
	{
		if ( 0 == _stricmp( edge_filter_xy_cstr[i], CSTR_CLAMP ) )
			edge_filter_xy[i] = EDGE_CLAMP;
		else if ( 0 == _stricmp( edge_filter_xy_cstr[i], CSTR_MIRROR ) )
			edge_filter_xy[i] = EDGE_MIRROR;
		else if ( 0 == _stricmp( edge_filter_xy_cstr[i], CSTR_TILE ) )
			edge_filter_xy[i] = EDGE_TILE;
		else
			return ACT_ERROR;
	}

	ACTRESULT err = ActTools::RescaleSurface( aDstSurface,
	                                          aSrcSurface,
	                                          color_filter_type, aColorBlur,
	                                          alpha_filter_type, aAlphaBlur,
	                                          edge_filter_xy[0], edge_filter_xy[1] );
	
	return err;
}

ACTRESULT ResizeTexture( ActTools::Texture* aTexture, int xSize, int ySize,
                          const char* aColorFilter, float aColorBlur,
                          const char* aAlphaFilter, float aAlphaBlur,
                          const char* aEdgeFilterX, const char* aEdgeFilterY )
{
	if( _stricmp( aTexture->GetFormat()->GetId(), "A8R8G8B8" ))
		return ACT_ERROR; 
	
	for( unsigned i_depth = 0; i_depth < aTexture->GetDepth(); ++i_depth )
	{
		for( size_t i = 0; i < aTexture->GetNumMipMaps(); ++i )
		{
			// Get the original surface
			Surface surface;
			surface.Acquire( aTexture->GetMipmap( i_depth, i ));

			size_t new_width = xSize;
			size_t new_height = ySize;
			aTexture->GetMipmap( i_depth, i )->Create( new_width, new_height );
			ACTRESULT err = RescaleSurface( aTexture->GetMipmap( i_depth, i ), &surface, aColorFilter, aColorBlur, 
				aAlphaFilter, aAlphaBlur, aEdgeFilterX, aEdgeFilterY );
			if( err == ACT_ERROR ) return ACT_ERROR;
		}
	}

	aTexture->SetNewSize(xSize, ySize);
	
	return ACT_OK;
}

ACTRESULT RescaleTexture( Texture* aTexture, float x_factor, float y_factor,
                          const char* aColorFilter, float aColorBlur,
                          const char* aAlphaFilter, float aAlphaBlur,
                          const char* aEdgeFilterX, const char* aEdgeFilterY )
{
	if( _stricmp( aTexture->GetFormat()->GetId(), "A8R8G8B8" ))
		return ACT_ERROR; 
	
	for( unsigned i_depth = 0; i_depth < aTexture->GetDepth(); ++i_depth )
	{
		for( size_t i = 0; i < aTexture->GetNumMipMaps(); ++i )
		{
			// Get the original surface
			Surface surface;
			surface.Acquire( aTexture->GetMipmap( i_depth, i ));

			size_t new_width = surface.GetWidth() * x_factor;
			size_t new_height = surface.GetHeight() * y_factor;
			aTexture->GetMipmap( i_depth, i )->Create( new_width, new_height );
			ACTRESULT err = RescaleSurface( aTexture->GetMipmap( i_depth, i ), &surface, aColorFilter, aColorBlur, 
				aAlphaFilter, aAlphaBlur, aEdgeFilterX, aEdgeFilterY );
			if( err == ACT_ERROR ) return ACT_ERROR;
		}
	}

	aTexture->SetNewSize(aTexture->GetWidth() * x_factor, aTexture->GetHeight() * y_factor);
	
	return ACT_OK;
}

}// namespace Texutils
