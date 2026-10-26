#ifndef PALETTE_H__INCLUDED
#define PALETTE_H__INCLUDED

#include "texture_export/TexelFormat.h"

namespace ActTools 
{

class Palette
{
public:
	Palette()
	{
		Init();
	}
	~Palette()
	{
		Destroy();
	}

	void Init()
	{
		theFormat = 0;
		theNumEntries = 0;
		theSize = 0;
		theTexels = 0;
	}
	void Destroy();

	void Create( const ActTools::TexelFormat* aFormat, size_t aNumEntries );
	Palette& CopyFrom( const Palette& aPalette );
	void Resize( size_t aSize );

	void Get( size_t aIndex, int* aRed, int* aGreen, int* aBlue, int* aAlpha ) const
	{
		theFormat->Get( &theTexels[theFormat->Bytes( aIndex, 1 )], aRed, aGreen, aBlue, aAlpha );
	}
	void Set( size_t aIndex, int aRed, int aGreen, int aBlue, int aAlpha )
	{
		theFormat->Set( &theTexels[theFormat->Bytes( aIndex, 1 )], aRed, aGreen, aBlue, aAlpha );
	}

public:
	const ActTools::TexelFormat* theFormat;
	size_t theNumEntries;
	size_t theSize;
	u8* theTexels;
};

}
#endif // PALETTE_H__INCLUDED

