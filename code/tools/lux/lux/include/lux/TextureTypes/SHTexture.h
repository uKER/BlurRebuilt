#pragma once

struct FilterResponse;
#include "Lux\SurfaceIterator.h"
#include "Lux\TextureTypes\ITexture.h"

class ImageFilter;

template <int iSHOrder>
class SHTexture: public ITexture
{
	// texel structure for this texture type
	struct SLMapPixel
	{
		float	fSH[3][iSHOrder * iSHOrder];
		float	fWeight;
	};

public:
				SHTexture(int x, int y, ImageFilter *pFilter) : ITexture(x,y,pFilter) { Create(x,y); }
				~SHTexture() { Free(); };

	// generate output format data
	U32		   *GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool bAlphaScaling, bool bSwizzleSH);

	// clear the contents of the texture
	void		Clear();
	// add a sample to the normal buffer
	void		AddSample(float2 t, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent);
	// add a sample to the normal buffer
	void		AddSample(float2 t, const Ut::Vector<float3> &samples, const float3 &vNormal, const HMat33 &mtxTangent);
	// add a sample to the dicontinuity buffer
	void		AddDiscontinuitySample(int iX, int iY, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent);
	// add a sample to the dicontinuity buffer - multichannel
	void		AddDiscontinuitySample(int iX, int iY, const Ut::Vector<float3> &samples, const float3 &vNormal, const HMat33 &mtxTangent);
	// convolve all added samples into the intermediate buffer
	void		UpdateWeights(LUX::RenderChannel *pChannel);
	// convolve all added discontinuity samples into the intermediate buffer
	void		Filter(LUX::RenderChannel *pChannel);
	// convolve all added discontinuity samples into the intermediate buffer
	void		Filter(const Ut::Vector<LUX::RenderChannel*> &channels);
	// get number of texture layers needed
	int			GetNumLayers();
	// merge the values of two texels. this function is used by the anti chart seam code
	void		MergeTexels(float2 t1, ITexture *pOtherTexture, float2 t2);

protected:
	SLMapPixel *m_pData;				// temporary accumulation buffer for normal sample data
	SLMapPixel *m_pDData;				// temporary accumulation buffer for discontinuity sample data
	SLMapPixel *m_pIntermediateBuffer;	// total accumulation buffer

	bool		m_bInitialized;
	int			m_iPass;

	void		Create(int x, int y);
	void		Free();
	SLMapPixel	&GetPixel(int x, int y)		{ assert(x>=0 && x<m_cX && y>=0 && y<m_cY); return m_pData[x + y*m_cX]; }
	SLMapPixel	&GetPixel(int i)			{ return m_pData[i]; }
};

#include "Lux\TextureTypes\SHTexture.inl"
