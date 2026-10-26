#include "stdafx.h"

#include "texture_export/Texture.h"

#include <minmax.h>

#include "texture_export/Surface.h"
#include "texture_export/Texture.h"
#include "texture_export/Rescale.h"

#include "texture_export/Converters/RescaleSurface.h"

using namespace ActTools;

void Texture::Create( Type aType, const ActTools::TexelFormat* aFormat, size_t aWidth, size_t aHeight, size_t aDepth, unsigned aNumMipmaps )
{
	assert( aFormat && INVALID != aType );
	assert( aDepth > 0 );
	assert( aType == VOLUMETEXTURE || aType == CUBETEXTURE || aDepth == 1);

	theType = aType;
	theWidth = aWidth;
	theHeight = aHeight;
	theDepth = aDepth;
	bSRGBTexture = 0;

	theNumMipmaps = aNumMipmaps;
	size_t num_surfaces = aNumMipmaps * theDepth;

	theFormat = aFormat->Clone();

	theSurfaces.Resize(num_surfaces);

	for (int i = 0; i < theSurfaces.Size(); i++)
		theSurfaces[i].SetTexture(this);
}

void Texture::CreateTexture( const ActTools::TexelFormat* aFormat, size_t aWidth, size_t aHeight, unsigned aNumMipmaps )
{
	Texture::Create( TEXTURE, aFormat, aWidth, aHeight, 1, aNumMipmaps);
}

void Texture::CreateVolumeTexture( const ActTools::TexelFormat* aFormat, size_t aWidth, size_t aHeight, unsigned aDepth, unsigned aNumMipmaps )
{
	Texture::Create( VOLUMETEXTURE, aFormat, aWidth, aHeight, aDepth, aNumMipmaps);
}

void Texture::CreateCubeTexture( const ActTools::TexelFormat* aFormat, size_t aWidth, unsigned aNumMipmaps )
{
	Texture::Create( CUBETEXTURE, aFormat, aWidth, aWidth, 6, aNumMipmaps);
}

/*void Texture::CreateAnimated( const ActTools::TexelFormat* aFormat, unsigned aNumFrames )
{
	theType = ANIMATED;
	theDepth = 0;
	theWidth = 0;
	theHeight = 0;

	theFormat = aFormat;

	theNumFrames = aNumFrames;
	theFrames = new TextureFrame[ theNumFrames ];
}*/

void Texture::Init()
{
	theName.Init();
	theFilePath.Init();

	theType = INVALID;
	theWidth = 0;
	theHeight = 0;
	theDepth = 0;
	theAlphaFlags = 0;
	theNumMipmaps = 0;
	theNumFrames = 0;
	theFormat = 0;
	thePalette = 0;
	bSRGBTexture = 0;
	//theFrames = 0;
}

void Texture::Destroy()
{
	theName.Destroy();
	theFilePath.Destroy();

	theSurfaces.Destroy();

	delete thePalette;

	delete theFormat;

	//delete [] theFrames;
}

Texture& Texture::CopyFrom( const Texture& aTexture )
{
	if (&aTexture == this)
		return *this;

	Destroy();
	Init();

	theName = aTexture.theName;
	theFilePath = aTexture.theFilePath;
	theType = aTexture.theType;
	theWidth = aTexture.theWidth;
	theHeight = aTexture.theHeight;
	theDepth = aTexture.theDepth;
	theAlphaFlags = aTexture.theAlphaFlags;
	theNumMipmaps = aTexture.theNumMipmaps;
	theNumFrames = aTexture.theNumFrames;
	bSRGBTexture = aTexture.bSRGBTexture;

	if( aTexture.theFormat )
		theFormat = aTexture.theFormat->Clone();

	theSurfaces.Resize(aTexture.theSurfaces.Size());

	for(int i = 0; i < theSurfaces.Size(); ++i )
	{
		theSurfaces[i].SetTexture(this);
		theSurfaces[i] = aTexture.theSurfaces[i];
	}

	if( aTexture.thePalette )
	{
		thePalette = new Palette;
		thePalette->Init();
		thePalette->CopyFrom( *(aTexture.thePalette) );
	}

	/*if( aTexture.theFrames )
	{
		delete [] theFrames;
		theFrames = new TextureFrame[ theNumFrames ];
		for( size_t i = 0; i < aTexture.theNumFrames; ++i )
		{
			theFrames[i].theName = aTexture.theFrames[i].theName;
			theFrames[i].theTime = aTexture.theFrames[i].theTime;
		}
	}*/

	return *this;
}

void Texture::DetachSurface( unsigned aSubTexture, unsigned aMipLevel, Surface* aDetached )
{
	Surface* surface = GetMipmap( aSubTexture, aMipLevel );
	if ( aDetached )
	{
		aDetached->Acquire( surface );
	}
	else
	{
		surface->Destroy();
	}
}

void Texture::AttachSurface( unsigned aSubTexture, unsigned aMipLevel, Surface& aSurface, Surface* aDetached )
{
	Surface* surface = GetMipmap( aSubTexture, aMipLevel );
	if ( aDetached )
	{
		aDetached->Acquire( surface );
	}

	surface->Acquire(&aSurface);
}

void Texture::AddMipmaps( size_t aNumMipmapsToAdd )
{
	theNumMipmaps += aNumMipmapsToAdd;
	size_t num_surfaces = theNumMipmaps * theDepth;
	
	theSurfaces.Resize(num_surfaces);

	for (int i = 0; i < theSurfaces.Size(); i++ )
		theSurfaces[i].SetTexture(this);
}

void Texture::AttachPalette( ActTools::Palette* aPalette )
{
	delete thePalette;
	thePalette = aPalette;
}

void Texture::ConvertTo( const TexelFormat* aNewFormat )
{
	if ( !theFormat->Compare( aNewFormat ) )
		return;

	A8R8G8B8 a8r8g8b8;
	P8 p8;
	A32R32G32B32F a32r32g32b32f;

	TexelFormat* intermediate_format = NULL;
	if ( theFormat->GetClass() == TexelFormat::CLASS_INTEGER )
		intermediate_format = &a8r8g8b8;
	else if ( theFormat->GetClass() == TexelFormat::CLASS_PALETTE )
		intermediate_format = &p8;
	else if ( theFormat->GetClass() == TexelFormat::CLASS_FLOAT )
		intermediate_format = &a32r32g32b32f;

	ConvertTo(intermediate_format);

	Texture tmp_texture;
	tmp_texture.Create(theType, intermediate_format, theWidth, theHeight, 1, 1);
	tmp_texture.theAlphaFlags = theAlphaFlags;
	Surface& tmp_surface = tmp_texture.theSurfaces[0];

	for ( size_t i = 0; i < theNumMipmaps * theDepth; ++i )
	{
		Surface & surface = theSurfaces[i];

		// DXT compression doesn't work properly when the size is less than 4 pixels.
		U32 newWidth = max(surface.GetWidth(), aNewFormat->BlockSize());
		U32 newHeight = max(surface.GetHeight(), aNewFormat->BlockSize());

		tmp_surface.theWidth = newWidth;
		tmp_surface.theHeight = newHeight;
		
		if (newWidth != surface.GetWidth() || newHeight != surface.GetHeight())
		{
			tmp_surface.Resize(intermediate_format->Bytes(tmp_surface.GetWidth(), tmp_surface.GetHeight()) );
			RescaleSurface(&tmp_surface, &surface, ActTools::FT_LANCZOS, 1.f, ActTools::FT_LANCZOS, 1.f, ActTools::EDGE_TILE, ActTools::EDGE_TILE);
			surface.theWidth = newWidth;
			surface.theHeight = newHeight;
		}
		else
		{
			tmp_surface.theSize = surface.theSize;
			tmp_surface.theTexels = surface.theTexels; surface.theTexels = 0;
		}

		surface.Resize( aNewFormat->Bytes( surface.GetWidth(), surface.GetHeight() ) );
		
		if (intermediate_format->GetClass() == TexelFormat::CLASS_INTEGER)
			aNewFormat->CopyFromA8R8G8B8( &surface, &tmp_surface );
		else
		if (intermediate_format->GetClass() == TexelFormat::CLASS_PALETTE)
			theFormat->CopyFromP8( &surface, &tmp_surface );
		else
		if (intermediate_format->GetClass() == TexelFormat::CLASS_FLOAT)
			aNewFormat->CopyFromA32R32G32B32F( &surface, &tmp_surface );

		tmp_surface.Destroy();
	}

	delete theFormat;
	theFormat = aNewFormat->Clone();
}

S32 Texture::GetSize() const
{
	S32 size = 0;

	for (int i = 0; i < theSurfaces.Size(); i++)
		size += theSurfaces[i].GetSize();

	return size;
}

