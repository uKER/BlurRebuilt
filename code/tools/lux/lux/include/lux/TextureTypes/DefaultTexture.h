#ifndef __DEFAULT_TEXTURE_H__
#define __DEFAULT_TEXTURE_H__

struct FilterResponse;
#include "Lux\SurfaceIterator.h"
#include "Lux\TextureTypes\ITexture.h"

class ImageFilter;

namespace LUX
{
	class RenderChannel;
}

// TODO: does this class still have a purpose? possibly only used for instances?
class DefaultTexture : public ITexture
{
protected:
	// texel structure for this texture type
	struct SLMapPixel
	{
		float3  vColour;
		float	fWeight;
	};

public:
				DefaultTexture(int x, int y, ImageFilter *pFilter ) : ITexture(x,y,pFilter) { Create(x,y); }
	virtual		~DefaultTexture() { Free(); };

	// generate output format data
	virtual U32	*GenerateRGBAData(int iLayer, float fGamma, float fBrightness) { assert(0); return NULL; }

	// clear the contents of the texture
	void		Clear();
	//
	int			GetNumLayers() { return 1; }
	// calculate per 4x4 texel block scale value and put it in the alpha
	void		CalculatAlphaScaling();

	// dummy functions - we don't implement the rest of the itexture interface
	// TODO: do not inherit this from itexture anymore
	virtual void		AddSample(float2 t, const LightSample &sample, const HMat33 &mtxTangent) { assert(0); }
	// add a sample to the dicontinuity buffer
	virtual void		AddDiscontinuitySample(int iX, int iY, const LightSample &samples, const HMat33 &mtxTangent)  { assert(0); }
	// add multiple samples to the normal buffer
	virtual void		AddSample(float2 t, const Ut::Vector<LightSample> &samples, const HMat33 &mtxTangent)  { assert(0); }
	// add multiple samples to the discontinuity buffer
	virtual void		AddDiscontinuitySample(int iX, int iY, const Ut::Vector<LightSample> &samples, const HMat33 &mtxTangent)  { assert(0); }
	// convolve all added samples into the intermediate buffer
	virtual void		Resolve()  { assert(0); }
	// set a subwindow within the texture. rendering will be clamped to this window
	virtual void		SetWindow(int x, int y, int cx, int cy) { assert(0); }
	// merge the values of two texels. this function is used by the anti chart seam code
	virtual void		MergeTexels(float2 t1, ITexture *pOtherTexture, float2 t2,
									int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)  { assert(0); }

protected:
	SLMapPixel *m_pData;
	SLMapPixel *m_pDData;
	float4	   *m_pIntermediateBuffer;

	int			m_iPass;

	void			Create(int x, int y);
	void			Free();
	SLMapPixel		&GetPixel(int x, int y)		{ assert(x>=0 && x<m_cX && y>=0 && y<m_cY); return m_pData[x + y*m_cX]; }
	SLMapPixel		&GetPixel(int i)			{ return m_pData[i]; }
};

#endif
