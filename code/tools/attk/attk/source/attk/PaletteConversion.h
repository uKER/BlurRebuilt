#ifndef ATTK_PALETTE_CONVERSION_H
#define ATTK_PALETTE_CONVERSION_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	class C_PaletteConversion
	{
	public:
		static C_Status CopyToInterchangeFormat(
			const U8* pSource,
			E_PaletteFormat sourceFormat,
			C_PixelR32G32B32A32F* pDestination,
			U32 nPixels);

		static C_Status CopyFromInterchangeFormat(
			const C_PixelR32G32B32A32F* pSource,
			U8* pDestination,
			E_PaletteFormat destinationFormat,
			U32 nPixels);
	};
};

#endif

