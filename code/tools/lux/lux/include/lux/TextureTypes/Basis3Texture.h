#ifndef __BASIS3TEXTURE_H__
#define __BASIS3TEXTURE_H__

struct FilterResponse;
#include "Lux\SurfaceIterator.h"
#include "Lux\TextureTypes\ITexture.h"

class ImageFilter;

class Basis3Texture: public ITexture
{
	// texel structure for this texture type
	struct SLMapPixel
	{
		float4	f[4];
		float	fWeight;
	};

public:
				Basis3Texture(int x, int y, ImageFilter *pFilter, bool bHemi3v = false);
				~Basis3Texture() { Free(); };

	// generate output format data
	U32		   *GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool bAlphaScaling, bool bSwizzleSH);

	// clear the contents of the texture
	void		Clear();
	// add a sample to the normal buffer
	void		AddSample(float2 t, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent);
	// add a sample to the dicontinuity buffer
	void		AddDiscontinuitySample(int iX, int iY, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent);
	// add a sample to the dicontinuity buffer (multiple channels)
	void		AddDiscontinuitySample(int iX, int iY, const Ut::Vector<float3> &sample, const float3 &vNormal, const HMat33 &mtxTangent);
	// convolve all added samples into the intermediate buffer
	void		UpdateWeights(LUX::RenderChannel *pChannel);
	// convolve all added discontinuity samples into the intermediate buffer
	void		Filter(LUX::RenderChannel *pChannel);
	// convolve all added discontinuity samples into the intermediate buffer (multiple channels)
	void		Filter(const Ut::Vector<LUX::RenderChannel*> &channels);
	// get number of texture layers needed
	int			GetNumLayers();
	// return true
	bool		NeedTangentSpace();
	// merge the values of two texels. this function is used by the anti chart seam code
	void		MergeTexels(float2 t1, ITexture *pOtherTexture, float2 t2);

	const static float3 m_vAxis[3];

protected:
	// remove all the channels on the intermediate buffer
	void		ClearIntermediateBuffer();
	// initialize n layers on the intermediate buffer
	void		InitializeIntermediateBuffer(int nChannels);
	// 
	void		UpdateWeights(LUX::RenderChannel *pChannel, SLMapPixel *pData);

	SLMapPixel				*m_pData;				// temporary accumulation buffer for normal sample data
	Ut::Vector<SLMapPixel*>	m_DData;				// temporary accumulation buffer for discontinuity sample data
	SLMapPixel				*m_pIntermediateBuffer;	// total accumulation buffer

	bool		m_bInitialized;
	int			m_iPass;

	void		Create(int x, int y);
	void		Free();
	SLMapPixel	&GetPixel(int x, int y)		{ assert(x>=0 && x<m_cX && y>=0 && y<m_cY); return m_pData[x + y*m_cX]; }
	SLMapPixel	&GetPixel(int i)			{ return m_pData[i]; }

	void		CalculateAlphaScaling(int iChannel);

	bool		m_bHemi3v;
};

#endif
