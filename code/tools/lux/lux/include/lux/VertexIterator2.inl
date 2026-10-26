#ifndef __VERTEXITERATOR_INL_H__
#define __VERTEXITERATOR_INL_H__

namespace LUX
{

template<typename T>
VertexIterator2<T>::VertexIterator2(VtxLightingJob *pJob) :
	m_pJob(pJob)
{
	// reserve buffer to store samples
	m_nVertices = pJob->m_Vertices.Size();
	m_fBrightness = pJob->m_fBrightness;
	m_fInvGamma = pJob->m_fInvGamma;
	m_bbox = pJob->m_bbox;

	m_Result.Resize(m_nVertices);
	memset(&m_Result[0], 0, sizeof(T) * m_nVertices);

	ClearAccumulationBuffer();

	m_SHMode = E_SHMode_RGB;
}

// clear accumulation buffer
template<typename T>
void VertexIterator2<T>::ClearAccumulationBuffer()
{
	m_DData.Resize(m_nVertices);
	memset(&m_DData[0], 0, sizeof(T) * m_nVertices);
}

// run the iterator
template<typename T>
void VertexIterator2<T>::Go(t_mode mode, int nSamples, SurfaceIteratorCallbackPtr pCallback, void *pContext)
{
	float3 vPos, vNormal;

	if (mode == DiscontinuitySampling)
	{
		m_nSamples = nSamples * 8;
	}
	else
		m_nSamples = 1;

	// reset sample counters
	m_iOutVertex = 0;
	m_iOutSample = 0;

	int nVertices = m_pJob->m_Vertices.Size();

	// for each vertex
	for (Ut::Vector<VtxLightingJob::Vertex>::Iterator i=m_pJob->m_Vertices.Begin(); i!=m_pJob->m_Vertices.End(); ++i)
	{
		// for each sample
		for (int iSample=0; iSample<m_nSamples; iSample++)
		{
			(*pCallback)(pContext, i->vPos, i->vNormal, iSample);
		}
	}
}

// add a single channel sample
template<typename T>
void VertexIterator2<T>::AddSample(const LightSample &sample)
{
	HMat33 dummy;	
	
	T tmp = 0;
	tmp.SetSample( sample, dummy, m_context );

	m_DData[m_iOutVertex] = m_DData[m_iOutVertex] + tmp;

	// go to the next sample
	if (++m_iOutSample>=m_nSamples)
	{
		m_iOutSample = 0;
		++m_iOutVertex;
	}
}

// add multi-channel sample
template<typename T>
void VertexIterator2<T>::AddSample(const Ut::Vector<LightSample> &samples)
{
	HMat33 dummy;

	T tmp = 0;
	for (int i=0; i<samples.Size(); i++)
		tmp.SetSample( samples[i], dummy, m_context );

	m_DData[m_iOutVertex] = m_DData[m_iOutVertex] + tmp;

	// go to the next sample
	if (++m_iOutSample>=m_nSamples)
	{
		m_iOutSample = 0;
		++m_iOutVertex;
	}
}

// add result to the vertices
template<typename T>
void VertexIterator2<T>::End()
{
	float fScale = 1.0f/(float)m_nSamples;

	// read colour data from buffer and store in job result class
	for (int i=0; i<m_Result.Size(); i++)
		m_Result[i] = m_Result[i] + m_DData[i] * fScale;

	ClearAccumulationBuffer();
}

// end a single channel sample
template<typename T>
void VertexIterator2<T>::End(LUX::RenderChannel *pChannel)
{
	End();
}

/// resolve multiple channels in one go
template<typename T>
void VertexIterator2<T>::End(const Ut::Vector<LUX::RenderChannel*> &channels)
{
	End();
}

/// set SH mode (obsolete; TODO: factor out)
template<typename T>
void VertexIterator2<T>::SetSHMode(Enum_SHMode mode)
{
	m_SHMode = mode;
}

/// copies the result into the given buffer
template<typename T>
void VertexIterator2<T>::GetResult(float *pBuffer)
{
	for (int i=0; i<m_Result.Size(); i++)
		m_Result[i].GetData(pBuffer, m_context);
}

/// return the size of the result buffer in floats
template<typename T>
int VertexIterator2<T>::GetDataSize()
{
	return T::Scalars() * m_nVertices;
}

// retrieve context data for this object
template<typename T>
void VertexIterator2<T>::GetCustomData(Ut::Vector<U8> &data)
{
	int n = m_context.GetSize();
	if (n == 0)
		return;
	data.Resize(n);
	U8 *p = &data[0];
	m_context.GetData(p);
}

/// returns a pointer to a vertex job result to send back to the server
template<typename T>
BaseObject *VertexIterator2<T>::Process(SurfaceIntegrator *pIntegrator)
{
	// render the lighting
	pIntegrator->GetLights(m_bbox, NULL);
	pIntegrator->LightData(this);

	// retrieve result
	VtxLightingResult *pResult = new VtxLightingResult;

	for (int i=0; i<m_Result.Size(); i++)
	{
		m_Result[i] = m_Result[i] * m_fBrightness;
		m_Result[i].ApplyGammaCorrection( m_fInvGamma, m_context );
	}

	pResult->m_nVertices = m_nVertices;
	pResult->m_iStride = T::Scalars();
	pResult->m_Data.Resize(sizeof(float) * GetDataSize());

	float *pData = &pResult->m_Data[0];
	GetResult(pData);

	// retrieve custom data
	GetCustomData(pResult->m_CustomData);

	return pResult;
}


}; // namespace LUX

#endif
