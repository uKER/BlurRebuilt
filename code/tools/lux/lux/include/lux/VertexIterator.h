#ifndef __VERTEX_ITERATOR_H__
#define __VERTEX_ITERATOR_H__

#include "Lux/SurfaceIterator.h"

// iterates over a pre-rasterized triangle using RasterizedTriInfo class
class VertexIterator : public SurfaceIterator
{
public:
	Aftk::I_SceneMeshNode	*m_pMesh;
	int					m_nSamples;

	// when this is mode is set, the R,G,B,A values define a set of 2nd order SH coefficients representing the incoming light
	void				SetSHMode(bool bOn);

	// iteration variables
	int					m_iVertex;
	int					m_iPolygon;
	int					m_iOutSample;
	int					m_iOutVertex;
	int					m_iOutPolygon;
	Aftk::Matrix44		m_mWorldMatrix;
	int					m_vertexColourChannel;
	Aftk::C_SceneMeshNode	*m_pDataMesh;

	// iteration functions
						VertexIterator(Aftk::I_SceneMeshNode *p);
	void				Go(t_mode mode, int nSamples, SurfaceIteratorCallbackPtr pCallback, void *pContext);
	void				AddSample(const LightSample &sample);
	void				End(LUX::RenderChannel *);

	void				ApplyBrightnessAndGammaToVertexColours(float fBrightness, float fGamma);

private:
	struct VtxSample
	{
		float4	vSample[3];
	};

	Ut::Vector<VtxSample>	m_buffer;

	bool					m_bSHMode;

	void				CreateVertexDataChannel(Aftk::C_SceneMeshNode **ppMeshNode, int *piVertexColourChannel);
};

#endif
