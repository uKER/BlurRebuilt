#include "stdafx.h"
#include "Lux/SurfaceIterator.h"
#include "Lux/ShadowRayBatch.h"
#include "Lux/DebugDraw.h"

// constructor
ShadowRayBatch::ShadowRayBatch(SurfaceIterator *pIterator, const SD::SpatialHierarchy *pHierarchy) :
						m_pIterator(pIterator),
						m_pHierarchy(pHierarchy),
						m_iCurRay(0),
						m_iCurValue(0)
{
	m_batch.Clear();
}

// destructor
ShadowRayBatch::~ShadowRayBatch()
{
	// IMPORTANT: if this asserts it means the batch wasn't flushed before it was destroyed
	// make sure to flush the batch, and make sure the end the iterator AFTER the last batch is flushed
//	assert(m_iCurValue==0);
}

// add a single ray
void ShadowRayBatch::AddRay(const LightSample &sample, SD::Ray &ray)
{
//	AddLine(ray.origin, ray.origin + ray.direction * 0.01);
	float3 vColour = sample.vColour;

	// disabled because it messes up when passing through SH vectors, or samples with more than three elements
//	if (vColour.x+vColour.y+vColour.z<=0 || ray.length == 0)
	if (ray.length == 0)
	{
		AddSampleWithoutRay(sample);
		return;
	}

	// check the whole batch has matching ray direction signs
	if (m_iCurRay == 0)
		m_vDirection = ray.direction;	// first ray - remember it's direction
	else
	{
		// check this new ray's direction against the first ray
		int iDontMatch =	DIRECTION_BITS_DONT_MATCH(m_vDirection.x, ray.direction.x) |
							DIRECTION_BITS_DONT_MATCH(m_vDirection.y, ray.direction.y) |
							DIRECTION_BITS_DONT_MATCH(m_vDirection.z, ray.direction.z);
		if (iDontMatch)
		{
			Flush();	// signs don't match - flush the current batch
			m_vDirection = ray.direction; // this ray decides the new direction signs
		}
	}

	// add the ray to our list
	m_bRayOnList[m_iCurValue] = true;
	m_ray[m_iCurValue] = sample;
	assert(m_iCurRay == m_batch.m_nRays);
	m_batch.AddRay(ray);
/*
	assert(	m_vDirection.x == m_batch.m_Ray[0].direction.x && 
			m_vDirection.y == m_batch.m_Ray[0].direction.y && 
			m_vDirection.z == m_batch.m_Ray[0].direction.z);
*/
	assert(m_iCurRay < MAX_RAY_BATCH_SIZE);
	assert(m_iCurValue < MAX_RAY_BATCH_SIZE);

	// update counters and flush if neccesary
	m_iCurRay++;
	m_iCurValue++;
	if (m_iCurValue >= MAX_RAY_BATCH_SIZE)
	{
		Flush();
		return;
	}
}

// add a single black ray
void ShadowRayBatch::AddSampleWithoutRay(const LightSample &sample)
{
	m_bRayOnList[m_iCurValue] = false;
	m_ray[m_iCurValue] = sample;
	if (++m_iCurValue >= MAX_RAY_BATCH_SIZE)
		Flush();
}

// process the rays
void ShadowRayBatch::Flush()
{
	if (m_iCurValue == 0)
		return;

	// raytrace this batch of samples
	m_pHierarchy->RayTrace(m_batch);

	// get results and add them to the image
	for (int i=0, iBatch=0; i<m_iCurValue; i++)
	{
		// if we shot a ray, and is didn't hit anything then add the light contribution
		if (m_bRayOnList[i] && m_batch.GetResult(iBatch++).bHit)
		{
			m_ray[i].vColour = float3(0,0,0);
//			AddLine(m_batch.m_Ray[i].origin, m_batch.m_Ray[i].origin + m_batch.m_Ray[i].direction * 0.01);
		}
		else
		{
			int j = 1;
//			AddLine(m_batch.m_Ray[i].origin, m_batch.m_Ray[i].origin + m_batch.m_Ray[i].direction);
		}

		// send results to iterator
		m_pIterator->AddSample(m_ray[i]);
	}

	m_iCurRay = m_iCurValue = 0;
	m_batch.Clear();
}

// occlusion ray tracing flush function is slightly different
void OcclusionRayBatch::Flush()
{
	if (!m_bUnoccludedRay)
	{
		// raytrace this batch of samples
		m_pHierarchy->RayTrace(m_batch);

		// get results and add them to the image
		for (int iBatch=0; iBatch<m_batch.m_nRays; iBatch++)
		{
			if (!m_batch.GetResult(iBatch).bHit)
			{
				SD::Ray *pRay = &m_batch.m_Ray[iBatch];

				m_bUnoccludedRay = true;
//				AddLine(pRay->origin, pRay->origin + pRay->direction * pRay->length);
				return;
			}
		}
	}

	m_iCurRay = m_iCurValue = 0;
	m_batch.Clear();
}

