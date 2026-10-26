#ifndef __SAMPLEPOINTITERATOR_INL_H__
#define __SAMPLEPOINTITERATOR_INL_H__

namespace LUX
{

// reset all sample points data
template<typename T>
void SamplePointIterator<T>::Clear()
{
	for (int i=0; i<m_Samples.Size(); i++)
		(T&)m_Samples[i] = 0;
	ClearAccumulationBuffer();
}

/// constructor
template<typename T>
SamplePointIterator<T>::SamplePointIterator()
{
}

/// construct with job data
template<typename T>
SamplePointIterator<T>::SamplePointIterator(SamplePointJob *pJob)
{
	// copy over the sample positions into the sample point iterator
	// and calculate bounding box
	int n = pJob->m_SampleList.Size();
	m_Samples.Resize(n);
	Clear();
	for (int i=0; i<n; i++)
		m_Samples[i].m_vPos = pJob->m_SampleList[i];

	m_iSamplePointIdx = pJob->m_iSamplePointIdx;
}


/// destructor
template<typename T>
SamplePointIterator<T>::~SamplePointIterator()
{
}

/// clear the contents of the accumulation buffer
template<typename T>
void SamplePointIterator<T>::ClearAccumulationBuffer()
{
	m_AccumulationBuffer.Resize(m_Samples.Size());
	if (m_Samples.Size() > 0)
		memset(&m_AccumulationBuffer[0], 0, sizeof(AccumulationItem) * m_AccumulationBuffer.Size());
}

// iterator
template<typename T>
void SamplePointIterator<T>::Go(t_mode mode, int nSamples, SurfaceIteratorCallbackPtr pCallback, void *pContext)
{
	if (mode == SurfaceIterator::DiscontinuitySampling && nSamples<128) nSamples = 128;

	m_nSamples = nSamples;
	m_iOutSample = 0;
	m_iOutPoint = 0;

	int n = m_Samples.Size();

	// initialize accumulation buffer
	ClearAccumulationBuffer();

	// render the samples
	for (int iPoint=0; iPoint<n; iPoint++)
	{
		for (int iSample=0; iSample<nSamples; iSample++)
		{
			(*pCallback)(pContext, m_Samples[iPoint].m_vPos, float3(1,1,1), iSample);
		}
	}
}

/// sample callback function for one channel
template<typename T>
void SamplePointIterator<T>::AddSample(const LightSample &sample)
{
	HMat33 dummyMtx;

	T tmp; tmp = 0;
	tmp.SetSample(sample, dummyMtx, m_context);

	(T&)m_AccumulationBuffer[m_iOutPoint] = (T&)m_AccumulationBuffer[m_iOutPoint] + tmp;
	m_AccumulationBuffer[m_iOutPoint].m_fnSamples++;

	if (++m_iOutSample>=m_nSamples)
	{
		m_iOutSample = 0;
		m_iOutPoint++;
	}
}

/// add contents of accumulation buffer into result buffer
template<typename T>
void SamplePointIterator<T>::End(LUX::RenderChannel *pChannel)
{
	for (int i=0; i<m_Samples.Size(); i++)
	{
		if (m_AccumulationBuffer[i].m_fnSamples == 0)
			continue;

		float fScale = 1.0f / m_AccumulationBuffer[i].m_fnSamples;
		(T&)m_Samples[i] = (T&)m_Samples[i] + (T&)m_AccumulationBuffer[i] * fScale;
	}

	ClearAccumulationBuffer();
}

/// resolve function for multiple channels
template<typename T>
void SamplePointIterator<T>::End(const Ut::Vector<LUX::RenderChannel*> &channels)
{
	End(NULL);
}

/// sample callback function for multiple channels
template<typename T>
void SamplePointIterator<T>::AddSample(const Ut::Vector<LightSample> &samples)
{
	HMat33 dummyMtx;

	for (int i=0; i<samples.Size(); i++)
	{
		T tmp; tmp = 0;
		tmp.SetSample(samples[i], dummyMtx, m_context);
		(T&)m_AccumulationBuffer[m_iOutPoint] = (T&)m_AccumulationBuffer[m_iOutPoint] + tmp;
	}
	m_AccumulationBuffer[m_iOutPoint].m_fnSamples++;

	if (++m_iOutSample>=m_nSamples)
	{
		m_iOutSample = 0;
		m_iOutPoint++;
	}
}

/// retrieve the data from the result and store results into a SamplePointResult class
template<typename T>
BaseObject *SamplePointIterator<T>::Process(SurfaceIntegrator *pIntegrator)
{
	// calculate bounding box
	box3f bbox;
	int n = m_Samples.Size();
	for (int i=0; i<n; i++)
		bbox.UpdateBox(m_Samples[i].m_vPos);

	// get lights near these sample points
	pIntegrator->GetLights(bbox, NULL);

	// light data
	pIntegrator->LightData(this);

	SamplePointResult *pResult = new SamplePointResult;

	pResult->m_SampleResults.Resize(n * T::Scalars());
	if (pResult->m_SampleResults.Size() > 0)
	{
		float *p = &pResult->m_SampleResults[0];
		for (int i=0; i<n; i++)
			m_Samples[i].GetData(p, m_context);
	}

	pResult->m_iSamplePointIdx = m_iSamplePointIdx;
	pResult->m_nSamples = n;

	return pResult;
}


}

#endif
