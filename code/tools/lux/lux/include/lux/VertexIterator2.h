#ifndef __VERTEX_ITERATOR2_H__
#define __VERTEX_ITERATOR2_H__

#include "Lux/SurfaceIterator.h"

namespace LUX
{

class VtxLightingResult;
class VtxLightingJob;

enum Enum_SHMode
{
	E_SHMode_RGB,		// RGB colour	(3 components)
	E_SHMode_SH,		// greyscale SH (4 components)
	E_SHMode_RGBSH		// RGB SH (3x4 components)
};

// iterates over a set of vertices
template<typename T>
class VertexIterator2 : public SurfaceIterator, public I_JobRenderer
{
public:
	int					m_nSamples;

	typename T::ContextDataType	m_context;

	// iteration variables
	int					m_iVertex;
	int					m_iOutSample;
	int					m_iOutVertex;
	int					m_nVertices;

	// iteration functions

						/// constructor
						VertexIterator2(VtxLightingJob *pJob);

						/// start the iteration
	void				Go(t_mode mode, int nSamples, SurfaceIteratorCallbackPtr pCallback, void *pContext);

						/// add a single channel sample
	void				AddSample(const LightSample &sample);

						/// add a multi-channel sample
	void				AddSample(const Ut::Vector<LightSample> &samples);

						/// end a single channel sample
	void				End(LUX::RenderChannel *);

						/// resolve multiple channels in one go
	void				End(const Ut::Vector<LUX::RenderChannel*> &channels);

						/// set SH mode (obsolete; TODO: factor out)
	void				SetSHMode(Enum_SHMode mode);

						/// copies the result into the given buffer
	void				GetResult(float *pBuffer);

						/// return the size of the result buffer in floats
	int					GetDataSize();

						/// process the job and return a pointer to a vertex job result to send back to the server
	BaseObject*			Process(SurfaceIntegrator *pIntegrator);

						/// retrieve context data for this object
	void				GetCustomData(Ut::Vector<U8> &data);

private:
	void				ClearAccumulationBuffer();
	void				End();

	VtxLightingJob		*m_pJob;
	Enum_SHMode			m_SHMode;
	box3f				m_bbox;
	Ut::Vector<T>		m_DData;
	Ut::Vector<T>		m_Result;
	float				m_fBrightness;
	float				m_fInvGamma;
};

}

#endif

#include "Lux/VertexIterator2.inl"
