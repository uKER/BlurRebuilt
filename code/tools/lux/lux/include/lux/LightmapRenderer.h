#pragma once

#include "CServ/ClusterServHeaders.h"
#include "Lux/RasterizedTriIterator.h"

class RasterizedTriIterator;
class ITexture;

namespace LUX
{

// class that contains one chart of a lightmap job
class LightmapChartJob
{
public:
	Ut::Vector<NmRawTriangle> m_TriList;

	bool	AddFace(Aftk::I_SceneMeshNode *m_pMesh, int polygonIndex, Aftk::Matrix44 *pWorldMatrix, Aftk::Matrix44 *pNormalMatrix, int m_iLMapUVSet, bool bTangents);

	int		m_cX;
	int		m_cY;
	int		m_iChart;
	int		m_iCost;
	box3f	m_bbox;
};

inline ost::Engine& operator << (ost::Engine& ar, LightmapChartJob &T)
{
	ar << T.m_cX << T.m_cY << T.m_iChart << T.m_TriList << T.m_bbox.vmin << T.m_bbox.vmax << T.m_iCost;
    return ar;
}

inline ost::Engine& operator >> (ost::Engine& ar, LightmapChartJob &T)
{
	ar >> T.m_cX >> T.m_cY >> T.m_iChart >> T.m_TriList >> T.m_bbox.vmin >> T.m_bbox.vmax >> T.m_iCost;
    return ar;
}

// a seam structure to be passed to the client
struct SeamEdge
{
	int iChart1, iChart2;
	U16 x1,y1,w1,h1,x2,y2,w2,h2;
	
	float2 uv1a, uv1b, uv2a, uv2b;
};

inline ost::Engine& operator << (ost::Engine& ar, SeamEdge &T)
{
	ar << T.iChart1 << T.iChart2 << T.uv1a << T.uv1b << T.uv2a << T.uv2b;
	ar << T.x1 << T.y1 << T.w1 << T.h1 << T.x2 << T.y2 << T.w2 << T.h2;
    return ar;
}

inline ost::Engine& operator >> (ost::Engine& ar, SeamEdge &T)
{
	ar >> T.iChart1 >> T.iChart2 >> T.uv1a >> T.uv1b >> T.uv2a >> T.uv2b;
	ar >> T.x1 >> T.y1 >> T.w1 >> T.h1 >> T.x2 >> T.y2 >> T.w2 >> T.h2;
    return ar;
}

// class that contains information for a lightmap job
// also contains a list of potential seams, so the client can stitch them together
class LightmapJob : public BaseObject
{
public:
	DECLARE_PERSISTENCE(LightmapJob);

	Ut::Vector<LightmapChartJob> m_Charts;
	Ut::Vector<SeamEdge> m_Seams;
	Ut::BitArray128	m_iLightingPassBits;
	int		m_iLodLevel;
	int		m_iRenderMode;
	int		m_iTextureType;
	float	m_fOutputGamma;
	float	m_fBrightness;
	bool	m_bAlphaScaling;
	bool	m_bSwizzleSH;
	bool	m_bBouncePass;
	bool	m_bAntiLightLeaking;
	bool	m_bInstance;

	// process the job and return the result
	BaseObject *Process(SurfaceIntegrator *pIntegrator);

	void write(ost::Engine &ar)
	{
		ar << m_Charts << m_Seams << m_iLightingPassBits << m_iRenderMode << m_iTextureType << m_bInstance << m_iLodLevel;
		ar << m_fOutputGamma << m_fBrightness << m_bAlphaScaling << m_bSwizzleSH << m_bBouncePass << m_bAntiLightLeaking;
	}

	void read(ost::Engine &ar)
	{
		ar >> m_Charts >> m_Seams >> m_iLightingPassBits >> m_iRenderMode >> m_iTextureType >> m_bInstance >> m_iLodLevel;
		ar >> m_fOutputGamma >> m_fBrightness >> m_bAlphaScaling >> m_bSwizzleSH >> m_bBouncePass >> m_bAntiLightLeaking;
	}
};

// class containing the result of a lightmap job; a group of textures, one for each chart in a seam group
class LightmapResult : public BaseObject
{
	DECLARE_PERSISTENCE(LightmapResult);

	~LightmapResult();

	Ut::Vector<RGBTexture*> m_Textures;

	void write(ost::Engine &ar);
	void read(ost::Engine &ar);
};

// class that renders a lightmap job and returns the result
class LightmapRenderer : public I_JobRenderer
{
public:
						// constructor
						LightmapRenderer(LightmapJob *pJob);

						// process and return result job to be sent back to the server
	ost::BaseObject		*Process(SurfaceIntegrator *pIntegrator);

						// as above but new version that works with flexible datatypes
	template<typename T>
	ost::BaseObject		*Process2(SurfaceIntegrator *pIntegrator);

private:
	LightmapJob			*m_pJob;
	SurfaceIntegrator	*m_pIntegrator;
};

}
