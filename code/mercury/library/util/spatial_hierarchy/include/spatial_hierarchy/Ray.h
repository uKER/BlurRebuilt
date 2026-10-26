#ifndef RAY_H
#define RAY_H

#include "spatial_hierarchy/RayTraceResult.h"

#ifndef GCC
#pragma intrinsic( fabs )
#endif

namespace SD
{

/// A single ray, to be used as input for the SpatialHierarchy class
class Ray
{
public:
	Ray () {};

	/// the origin point of the ray
    Float3	origin;
	/// the normalised direction of the ray
    Float3	direction;
	/// indicates whether this ray is a shadow ray or not
	bool	bShadowRay;
	/// length of the ray along the direction
	float	length;

	/// initialize a ray using an origin and destination point, or origin and direction
	/** By default, the 'd' argument is interpreted as a destination. However, if bDirection is
		set to true, 'd' will be interpreted as the destination point. **/
	Ray (const Float3 &o, const Float3 &d, bool bDirection=false);
	/// copy constructor
    Ray (const Ray &r)
	{
		origin = r.origin;
		direction = r.direction;
		length = r.length;
    }
	/// initialize a ray using an origin point and a normalised direction
	void SetRay(const Float3 &vPos, const Float3 &vDir)
	{
		origin = vPos;
		direction = vDir;
		length = FLT_MAX;
	}
};

#define MAX_RAY_BATCH_SIZE 4

/// A batch of 4 shadow-rays, to be used for SSE raytracing
class RayBatch
{
public:
	/// array containing the actual rays in this batch
	Ray				m_Ray[MAX_RAY_BATCH_SIZE];
	/// array containing the a result structure for each ray in the batch
	RayTraceResult	m_Result[MAX_RAY_BATCH_SIZE];
	/// number of rays in this batch
	int				m_nRays;

					RayBatch() : m_nRays(0) {};

	/// add a single ray to the batch of rays
	void			AddRay(Ray &ray)		{ assert(m_nRays < MAX_RAY_BATCH_SIZE); m_Ray[m_nRays] = ray; m_nRays++; }
	/// retrieve the result of a particular ray after it has been raytraced
	RayTraceResult &GetResult(int i)		{ assert(i < m_nRays); return m_Result[i]; }
	/// clear the number of rays in the batch to 0
	void			Clear()					{ m_nRays = 0; }
};

}

#endif
