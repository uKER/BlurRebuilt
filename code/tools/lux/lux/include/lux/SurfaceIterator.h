#ifndef __LUX_SURFACEITERATOR_H__
#define __LUX_SURFACEITERATOR_H__

#include "Lux\LightSample.h"

class Sampler;

typedef void (*SurfaceIteratorCallbackPtr)(void *pContext, const float3 &vPos, const float3 &vNormal, int iSample);

/// virtual base class for surface iterators (e.g. vertex iterator, lightmap, SH node)
class SurfaceIterator
{
public:
	Sampler			*m_pSampler;
	int				m_iSample;

	enum t_mode
	{
		SampleFirst,		// iterate each sample for every texel
		TexelFirst,			// iterate once across the whole chart for each sample
		SampleFirstNoAA,
		DiscontinuitySampling,
		DiscontinuitySampling2,
	} m_mode;

	virtual			~SurfaceIterator() {};
	virtual void	Go(t_mode mode, int nSamples, SurfaceIteratorCallbackPtr pCallback, void *pContext) { assert(0); }
	virtual void	End(LUX::RenderChannel *) {};
	virtual void	End(const Ut::Vector<LUX::RenderChannel*> &channels) {};
	virtual void	AddSample(const LightSample &sample) = 0;
	virtual void	AddSample(const Ut::Vector<LightSample> &samples) {};
	virtual void	RememberSample() {};
	virtual bool	SurfaceNormals() { return true; } // true = callback normals are valid, false = no normals
	void			SetSampler(Sampler *pSampler) { m_pSampler = pSampler; }
};


#endif
