#ifndef __BOUNCE_TEXTURE_H__
#define __BOUNCE_TEXTURE_H__

struct FilterResponse;
#include "Lux\SurfaceIterator.h"
#include "Lux\TextureTypes\ITexture.h"

class ImageFilter;

namespace LUX
{
	class RenderChannel;
}

class BounceTexture : public DefaultTexture
{
public:
				BounceTexture(int x, int y, ImageFilter *pFilter) : DefaultTexture(x,y,pFilter) { }
	virtual		~BounceTexture() { ClearChannels(); }

	// generate output format data
	U32		   *GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool bAlphaScaling, bool bSwizzleSH);
	// convolve all added discontinuity samples into the intermediate buffer
	void		Filter(LUX::RenderChannel *pChannel);
	// convolve all added discontinuity samples into the intermediate buffer - multi channel
	void		Filter(const Ut::Vector<LUX::RenderChannel*> &channels);
	// add a sample to the dicontinuity buffer
	void		AddDiscontinuitySample(int iX, int iY, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent);
	// add discontinuity buffer sample - multi channel
	void		AddDiscontinuitySample(int iX, int iY, const Ut::Vector<float3> &samples, const float3 &vNormal, const HMat33 &mtxTangent);
	// add a sample to the normal buffer
	void		AddSample(float2 t, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent);
	// convolve all added samples into the intermediate buffer
	void		UpdateWeights(LUX::RenderChannel *pChannel);

private:
	SLMapPixel	&GetPixel(int iChannel, int x, int y)	{ assert(x>=0 && x<m_cX && y>=0 && y<m_cY); return m_DData[iChannel][x + y*m_cX]; }
	void		AddDiscontinuitySample(int iX, int iY, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent, int iChannel);
	void		InitializeChannels(int nChannels);
	void		ClearChannels();
	void		UpdateWeights(LUX::RenderChannel *pChannel, int iChannel);

	Ut::Vector<SLMapPixel*> m_DData;
};

#endif
