#include "stdafx.h"

#include "texture_export/Surface.h"
#include "texture_export/Texture.h"

using namespace ActTools;

void Surface::Create( size_t aWidth, size_t aHeight )
{
	assert( GetFormat() );

	theWidth = aWidth;
	theHeight = aHeight;

	theSize = GetFormat()->Bytes( theWidth, theHeight );
	theTexels = new u8 [ theSize ];
}

void Surface::Init()
{
	theTexture = 0;
	theFormat = 0;

	theWidth = 0;
	theHeight = 0;

	theSize = 0;
	theTexels = 0;
}

void Surface::Resize( size_t aSize )
{
	theSize = aSize;
	delete theTexels;
	theTexels = new u8[ aSize ];
}

const ActTools::TexelFormat* Surface::GetFormat() const
{
	if (theTexture)
		return theTexture->GetFormat();
	else if (theFormat)
		return theFormat;
	else
		return 0;
}

U32 Surface::GetAlphaPresence() const 
{
	return theTexture->GetAlphaPresence();
}

void Surface::Destroy()
{
	theSize = 0;
	theWidth = 0;
	theHeight = 0;
	delete theFormat;
	theFormat = 0;

	if ( theTexels )
	{
		delete theTexels;
		theTexels = 0;
	}
}

void Surface::Acquire( Surface* aSurface )
{
	if( aSurface == this ) 
		return;

	Destroy();

	if (theTexture) // If associated with a texture, can't change the format.
		assert(*GetFormat() == *aSurface->GetFormat());
	else if (aSurface->GetFormat()) // Copy format from the other surface.
		theFormat = aSurface->GetFormat()->Clone();
	else
		theFormat = 0;

	theWidth = aSurface->theWidth; aSurface->theWidth = 0;
	theHeight = aSurface->theHeight; aSurface->theHeight = 0;

	theSize = aSurface->theSize; aSurface->theSize = 0;
	theTexels = aSurface->theTexels; aSurface->theTexels = 0;
}

Surface::Surface(const Surface& surface)
{
	Init();
	CopyFrom(surface);
}

Surface& Surface::operator = (const Surface& surface)
{
	if (&surface != this)
		CopyFrom(surface);

	return *this;
}

Surface& Surface::CopyFrom( const Surface& aSurface )
{
	delete theFormat;

	if (theTexture) // If associated with a texture, can't change the format.
		assert(*GetFormat() == *aSurface.GetFormat());
	else if (aSurface.GetFormat()) // Copy format from the other surface.
		theFormat = aSurface.GetFormat()->Clone();
	else
		theFormat = 0;

	theWidth = aSurface.GetWidth();
	theHeight = aSurface.GetHeight();
	theSize = aSurface.theSize;

	if( aSurface.theTexels )
	{
		if ( theTexels )
			delete theTexels;

		theTexels = new u8[theSize];
		memcpy( theTexels, aSurface.theTexels, theSize );
	}
	else
	{
		if ( theTexels )
		{
			delete theTexels;
			theTexels = 0;
		}
	}

	return *this;
}

