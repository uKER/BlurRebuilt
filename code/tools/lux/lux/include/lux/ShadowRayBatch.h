#pragma once

#define DIRECTION_BITS_DONT_MATCH(X,Y) ((((*((int*)&(X))) ^ (*((int*)&(Y))))) & 0x80000000)

class SurfaceIterator;

class ShadowRayInfo
{
public:
	float3 m_vIntensity;
	float3 m_vDirection;
};

class ShadowRayBatch
{
public:
					ShadowRayBatch(SurfaceIterator *pIterator, const SD::SpatialHierarchy *pHierarchy);
					~ShadowRayBatch();
	void			AddRay(const LightSample &sample, SD::Ray &ray);
	void			AddSampleWithoutRay(const LightSample &sample);
	virtual void	Flush();

	SurfaceIterator				*m_pIterator;
	const SD::SpatialHierarchy	*m_pHierarchy;

	SD::RayBatch	m_batch;
	bool			m_bRayOnList[MAX_RAY_BATCH_SIZE];
	LightSample		m_ray[MAX_RAY_BATCH_SIZE];

	int				m_iCurRay;
	int				m_iCurValue;
	float3			m_vDirection;
};

class OcclusionRayBatch : public ShadowRayBatch
{
public:
			OcclusionRayBatch(const SD::SpatialHierarchy *pHierarchy)
			: ShadowRayBatch(NULL,pHierarchy), m_bUnoccludedRay(false) {};

	void	Flush();

	bool	m_bUnoccludedRay;
};
