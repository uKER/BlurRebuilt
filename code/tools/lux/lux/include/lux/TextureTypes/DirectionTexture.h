#pragma once

struct FilterResponse;
#include "Lux/TextureTypes/SHTexture.h"
#include "Lux\SurfaceIterator.h"
#include "Lux\TextureTypes\ITexture.h"

class ImageFilter;

namespace LUX
{
	class RenderChannel;
}

class DirectionTexture : public SHTexture<2>
{
public:
				DirectionTexture(int x, int y, ImageFilter *pFilter) : SHTexture<2>(x,y,pFilter) { Create(x,y); }
	virtual		~DirectionTexture() { Free(); };

	// generate output format data
	U32		   *GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool bAlphaScaling, bool bSwizzleSH);
	//
	int			GetNumLayers() { return 3; }
};

