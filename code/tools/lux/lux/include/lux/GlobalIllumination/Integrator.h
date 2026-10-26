#pragma once

// Surface integrator using instant radiosity
class ImageFilter;
class Sampler;
class ChartTexelIterator;
class SurfaceIterator;
class ShadowRayBatch;
struct NmRawTriangle;

#define NUM_BOUNCE_LIGHT_SETS 9
#define RAND_SEQ_SIZE (288)
#define _INTEGRATOR_H_

namespace LUX
{
class Light;
class DirectionalLight;
class GlobalIlluminationProperties;
class RenderChannel;
class BounceChart;
class BounceTexData;
}
#include <spatial_hierarchy\Light-Tree.h>
#include "Lux\SurfaceIterator.h"
#include "Lux\IIntegrator.h"

struct LightGroupItem
{
	LUX::Light *pLight;
	int nSamples;
};

bool operator <(const LightGroupItem &t, const LightGroupItem &other);

typedef Ut::Vector<LUX::Light*> LightPtrVector;
typedef Ut::Vector<LUX::RenderChannel*> TRenderChannelVector;
typedef Ut::Vector<LUX::BounceChart> TBounceChartList;

namespace LUX
{

class GlobalIlluminationModel;

struct SurfaceIntegratorSetup
{
			SurfaceIntegratorSetup() {};
			SurfaceIntegratorSetup(float fMinDist, float fIndirectScale, float fRR, float fCT, int nSPT, float fTexelDensity, bool bAntiLightLeaking, float fEpsilon) : 
								m_fMinDist(fMinDist),				// distance at which the pointlight luminance is clamped
								m_fIndirectScale(fIndirectScale),	// indirect lighting scale amount
								m_fRRThreshold(fRR*fIndirectScale),	// skip shadow ray with russian roulette if luminance<fRR
								m_fCutoffThreshold(fCT*fIndirectScale),	// skip VPL altogether if luminance<fRR
								m_nSamplerPerTexel(nSPT),			// number of samples per texel		
								m_fTexelDensity(fTexelDensity),
								m_bAntiLightLeaking(bAntiLightLeaking), // light leaking threshold ( 0 = off )
								m_fEpsilon(fEpsilon)				// raytracing epsilon
								{};
	float	m_fMinDist;
	float	m_fIndirectScale;
	float	m_fRRThreshold;
	float	m_fCutoffThreshold;
	float	m_fTexelDensity;
	float	m_fEpsilon;
	int		m_nSamplerPerTexel;
	float	m_bAntiLightLeaking;
};

class SurfaceIntegrator : public IIntegrator
{
	DECLARE_PERSISTENCE(SurfaceIntegrator);

	void write( ost::Engine &ar);
	void read( ost::Engine &ar);

public:
				SurfaceIntegrator() {};	// default constructor for deserializing
				SurfaceIntegrator(const SurfaceIntegratorSetup &settings);
				~SurfaceIntegrator();

	void		SampleLightList(LightPtrVector *pLightList);
	void		SampleLightListParallel(LightPtrVector *pLightList);
	void		SampleLightListSorted(LightGroupItem *pItem, int nItems);
	void		RenderLight(Light *pLight);

	void		LightData(SurfaceIterator *);

	void		Preprocess();

	void		SetDirectionalLight(DirectionalLight *pLight) { m_pDirectionalLight = pLight; }
	void		SetGlobalLightList(LightPtrVector *pList) {  m_pGlobalLightList = pList; }
	void		SetSpatialHierarchy(const LUX::SpatialHierarchy *pHierarchy) { m_pHierarchy = pHierarchy; }
	void		SetLightTree(LightTree *p) { m_pLightTree = p; }
	void		SetBounceTextures(BounceTexData *p) { m_pBounceTextures = p; }
	void		SetRenderChannels(TRenderChannelVector *p) { m_pChannels = p; }
	void		SetIterator(SurfaceIterator *pIterator);
	void		GetLights(const box3f &box, Ut::Vector<NmRawTriangle> *pTriList);
	bool		TestLight(Light *pLight, Ut::Vector<NmRawTriangle> *pTriList);
	void		PrecalculateTriangleSamplePoints();
	void		RenderBounceGatherLight(Light *pLight);
	RenderChannel *GetRenderChannel(int iChannel);
	void		GetRenderChannelTarget(int iChannel, LightSample &sample);

	// callback iterator
	static void SampleCallbackParallelPoint		(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample);
	static void SampleCallbackParallel			(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample);
	static void	SampleCallbackFlat				(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample);
	static void	SampleCallbackLambert			(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample);
	static void SampleCallbackPoint				(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample);
	static void SampleCallbackWhite				(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample);
	static void SampleCallbackBounceGather		(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample);
	static void SampleCallbackBounceGatherPoint	(void *pThis, const float3 &vPos, const float3 &vNormal, int iCurSample);

	static const int m_iMaxTriangleSamples;

	ShadowRayBatch		*m_pBatch;
	Light				*m_pLight;
	SurfaceIterator		*m_pIterator;


	int					m_iBounceChannel;
	int					m_nVertexOutputChannels;
	float				m_fTotalSamples;

	SurfaceIntegratorSetup		m_settings;

	float		m_fMinDistSq;
	int			m_fLightPathsPerLight;
	int			m_nLightPaths;
	bool		m_bUseFiltering;
	int			m_nSamplerPerTexel;
	bool		m_bInitializedThroughDeserialization;
	float		m_fTexelDensity;
	float		m_bAntiLightLeaking;
	float		m_fEpsilon;
	int			m_iProgBouncePass;

	LightGroupItem	*m_pLightGroupItem;
	int				m_nLightGroupItems;

	// sample storage
	int			m_nSamples;
	int			m_iLightSet;

	// the lights
	DirectionalLight		*m_pDirectionalLight;
	Ut::Vector<LUX::Light*>	*m_pGlobalLightList;
	LightTree				*m_pLightTree;
	// render channels
	TRenderChannelVector	*m_pChannels;

	// collision tree
	const LUX::SpatialHierarchy	*m_pHierarchy;
	// bounce textures
	BounceTexData				*m_pBounceTextures;

	// filters
	ImageFilter *m_pGaussianFilter;
	ImageFilter *m_p3x3BoxFilter;
	Sampler		*m_pSampler;

	// triangle samples (as barycentric coordinates)
	Ut::Vector<float3> m_TriangleSamples;

	// sampling
	float2			m_Sample[RAND_SEQ_SIZE];
	const float2	&GetSample(int i);
	void			GenerateSequence();
};

}
