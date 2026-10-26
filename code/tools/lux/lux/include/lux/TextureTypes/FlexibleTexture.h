#ifndef __FLEXIBLETEXTURE_H__
#define __FLEXIBLETEXTURE_H__

struct FilterResponse;
#include "Lux\SurfaceIterator.h"
#include "Lux\TextureTypes\ITexture.h"

class ImageFilter;
namespace LUX
{
	class SurfaceIntegrator;
}

template<typename T>
class FlexibleTexture : public ITexture
{
	// texel structure for this texture type
	struct SLMapPixel : public T
	{
		float	fWeight;
	};

	typename T::ContextDataType	m_context;

public:
				FlexibleTexture(int x, int y, LUX::SurfaceIntegrator *pI);
				~FlexibleTexture() { Free(); };

	// generate output format data
	U32		   *GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool *pbFoundQNAN);
	// retrieve context data
	void		GetCustomData(Ut::Vector<U8> &data);

	// clear the contents of the texture
	void		Clear();
	// add a sample
	void		AddSample(float2 t, const LightSample &vSample, const HMat33 &mtxTangent);
	// add multiple samples
	void		AddSample(float2 t, const Ut::Vector<LightSample> &sample, const HMat33 &mtxTangent);
	// add a sample to the discontinuity buffer
	void		AddDiscontinuitySample(int iX, int iY, const LightSample &vSample,  const HMat33 &mtxTangent);
	// add multiple samples to the discontinuity buffer
	void		AddDiscontinuitySample(int iX, int iY, const Ut::Vector<LightSample> &sample, const HMat33 &mtxTangent);
	// convolve all added samples into the intermediate buffer
	void		Resolve();
	// merge the values of two texels. this function is used by the anti chart seam code
	void		MergeTexels(float2 t1, ITexture *pOtherTexture, float2 t2,
							int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
	// return the number of texture layers (depends on T)
	int			GetNumLayers();
	// returns true if this texture needs tangent space to render (depends on T)
	bool		NeedTangentSpace();

protected:
	// add a single sample in T format
	void		AddSample(float2 t, const T &sample, const HMat33 &mtxTangent);
	// add a single discontinuity sample in T format
	void		AddDiscontinuitySample(int iX, int iY, const T &sample, const HMat33 &mtxTangent);

	SLMapPixel	*m_pAccBuf;				// temporary accumulation buffer for normal sample data
	T			*m_pTotalBuf;			// total accumulation buffer

	bool		m_bInitialized;
	float		*m_pRAWBuffer;			// buffer containing raw data
	int			m_iStride;

	void		Create(int x, int y);
	void		Free();
	SLMapPixel	&GetPixel(int x, int y)		{ assert(x>=0 && x<m_cX && y>=0 && y<m_cY); return m_pAccBuf[x + y*m_cX]; }
	SLMapPixel	&GetPixel(int i)			{ return m_pAccBuf[i]; }
};

#endif

#include "FlexibleTexture.inl"
