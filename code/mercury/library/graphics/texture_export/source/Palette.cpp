#include "stdafx.h"

#include "ngl/system.h"
#include "texture_export/Palette.h"
#include "texture_export/Surface.h"

using namespace ActTools;

void Palette::Destroy()
{
	delete theFormat;
	delete theTexels;
}

void Palette::Create( const ActTools::TexelFormat* aFormat, size_t aNumEntries )
{
	theFormat = aFormat;

	theNumEntries = aNumEntries;

	theSize = theFormat->Bytes( theNumEntries, 1 );
	theTexels = new u8[ theSize ];
}

Palette& Palette::CopyFrom( const Palette& aPalette )
{
	theNumEntries = aPalette.theNumEntries;
	theSize = aPalette.theSize;

	if( aPalette.theTexels )
	{
		delete theTexels;
		theTexels = new u8[ theSize ];
		memcpy( theTexels, aPalette.theTexels, theSize );
	}
	else
	{
		if ( theTexels )
		{
			delete theTexels;
			theTexels = 0;
		}
	}

	if( aPalette.theFormat )
	{
		if( !theFormat )
			delete theFormat;
		theFormat = aPalette.theFormat->Clone();
	}
	else
	{
		if( theFormat )
		{
			delete theFormat;
			theFormat = 0;
		}
	}

	return *this;
}

void Palette::Resize( size_t aSize )
{
	u8 *pOld = theTexels;
	theTexels = new u8[ aSize ];
	memcpy(theTexels, pOld, Maths::Min(aSize, theSize) );
	theSize = aSize;
}

