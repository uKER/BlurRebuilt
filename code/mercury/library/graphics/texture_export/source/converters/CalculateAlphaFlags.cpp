#include "stdafx.h"

#include "texture_export/Texture.h"

using namespace ActTools;

namespace Texutils
{

ACTRESULT CalculateAlphaFlags( ActTools::Texture* aTexture )
{
	ACTRESULT err = ACT_OK;

	assert( aTexture->GetFormat() );
	Surface* surface = aTexture->GetMipmap( 0, 0 );
	assert( surface ); 

	A8R8G8B8 a8r8g8b8;
	P8 p8;
	P4 p4;
	A32R32G32B32F a32r32g32b32f;
	if ( !a8r8g8b8.Compare( aTexture->GetFormat() ) ||
	     !p8.Compare( aTexture->GetFormat() ) ||
	     !p4.Compare( aTexture->GetFormat() ) )
	{
		size_t alphas[256] = { 0 };
		size_t num_texels, i;
		if ( !a8r8g8b8.Compare( aTexture->GetFormat() ) )
		{
			i = num_texels = surface->GetWidth() * surface->GetHeight();
			for ( const u32* texel = reinterpret_cast<u32*>(surface->GetTexels()); i--; ++texel )
				++alphas[*texel >> 24];
		}
		else
		{
			assert( aTexture->GetPalette() );
			int r, g, b, a;
			i = num_texels = aTexture->GetPalette()->theNumEntries;
			while ( i-- )
			{
				aTexture->GetPalette()->Get( i, &r, &g, &b, &a );
				++alphas[a];
			}
		}
		if ( num_texels == alphas[255] )
		{
			aTexture->SetAlphaPresence( Texture::ALPHA_WHITE );
		}
		else if ( num_texels == alphas[0] )
		{
			aTexture->SetAlphaPresence( Texture::ALPHA_NONE );
		}
		else if ( num_texels == alphas[255] + alphas[0] )
		{
			aTexture->SetAlphaPresence( Texture::ALPHA_BLACKWHITE );
			aTexture->SetAlphaNumBits( 1 );
		}
		else
		{
			int n = 0;
			for ( i = 0; i < 256; ++i )
				n += 0!=alphas[i];
			int nbits;
			for ( nbits = 1; ((1<<nbits)-1) < n; ++nbits ) {}
			aTexture->SetAlphaPresence( Texture::ALPHA_GREYSCALE );
			aTexture->SetAlphaNumBits( n );
		}
	}
	else if ( !a32r32g32b32f.Compare( aTexture->GetFormat() ) )
	{

	}
	else 
	{

		err = ACT_ERROR;
	}

	return err;
}

}