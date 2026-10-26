#ifndef __SAMPLEPOINTITERATOR_H__
#define __SAMPLEPOINTITERATOR_H__

namespace LUX
{

class SamplePointJob;

template<typename T>
class SamplePointIterator : public SurfaceIterator, public I_JobRenderer
{
public:
	typename T::ContextDataType	m_context;

	struct SamplePointAndResult : public T
	{
		float3	m_vPos;
	};

	struct AccumulationItem : public T
	{
		float	m_fnSamples;
	};

						/// constructor
						SamplePointIterator();
						/// construct with job data
						SamplePointIterator(SamplePointJob *pJob);
						/// destructor
	virtual				~SamplePointIterator();
						/// run the iterator and call the callback for each sample
	void				Go(t_mode mode, int nSamples, SurfaceIteratorCallbackPtr pCallback, void *pContext);
						/// sample callback function for one channel
	void				AddSample(const LightSample &sample);
						/// resolve function for one channel
	void				End(LUX::RenderChannel *pChannel);
						/// returns true if callback normals are valid, false = no normals
	bool				SurfaceNormals() { return false; }
						/// sample callback function for multiple channels
	void				AddSample(const Ut::Vector<LightSample> &samples);
						/// resolve function for multiple channels
	void				End(const Ut::Vector<LUX::RenderChannel*> &channels);
						/// clear m_Samples
	void				Clear();
						/// retrieve the result data
	BaseObject			*Process(SurfaceIntegrator *pIntegrator);

protected:
						/// clear buffers
	void				ClearAccumulationBuffer();

public:
	// calculated samples
	Ut::Vector<SamplePointAndResult>	m_Samples;
protected:
	// sample point accumulation buffer
	Ut::Vector<AccumulationItem>		m_AccumulationBuffer;

	int					m_iOutSample;
	int					m_iOutPoint;
	int					m_nSamples;
	int					m_iSamplePointIdx;

	float				m_fInvGamma;
	float				m_fBrightness;
};

// class that contains information for a samplepoint rendering job
class SamplePointJob : public BaseObject
{
public:
	DECLARE_PERSISTENCE(SamplePointJob);

	Ut::Vector<float3> m_SampleList;
	int	m_iSamplePointIdx;
	float m_fInvGamma, m_fBrightness;
	int m_iRenderMode;
	Ut::BitArray128 m_iLightingPassBits;
	bool m_bBouncePass;

	void write(ost::Engine &ar)
	{
        ar << m_SampleList << m_iSamplePointIdx << m_fInvGamma << m_fBrightness << m_iRenderMode << m_bBouncePass << m_iLightingPassBits;
	}
	void read(ost::Engine &ar)
	{
		ar >> m_SampleList >> m_iSamplePointIdx >> m_fInvGamma >> m_fBrightness >> m_iRenderMode >> m_bBouncePass >> m_iLightingPassBits;
	}
};

// class that contains the sample point job rendering results
class SamplePointResult : public BaseObject
{
public:
	DECLARE_PERSISTENCE(SamplePointResult);

	Ut::Vector<float> m_SampleResults;
	int m_nSamples;

	int m_iSamplePointIdx;

	void write(ost::Engine &ar)
	{
		ar << m_SampleResults << m_iSamplePointIdx << m_nSamples;
	}		
	void read(ost::Engine &ar)
	{
		ar >> m_SampleResults >> m_iSamplePointIdx >> m_nSamples;
	}
};

}

#include "Lux\SamplePointIterator.inl"

#endif
