
#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\CollisionUtil.h"
#include "Impact\ConvexBase.h"
#include "System/Debug/I_DebugRender.h"
#include "System/Misc/Random.h"
#ifdef PLATFORM_PC
#include <xmmintrin.h>
#endif

using namespace Maths;

namespace Impact
{

Debug::Bool g_bProjectSupportSIMD("Impact.Optimizations.ProjectSupportSIMD", true);

#ifdef PLATFORM_PC

/// calculate the point on the convex hull that is farthest along the support axis
HVec3 ConvexBase::ProjectSupportAxisSIMD8(const C_Normal &vNormal, int *pI) const
{
	// load points in SOA format
	HMat44 verts[2];
	((HVec4*)verts)[0] = m_Points[0];
	((HVec4*)verts)[1] = m_Points[1];
	((HVec4*)verts)[2] = m_Points[2];
	((HVec4*)verts)[3] = m_Points[3];
	((HVec4*)verts)[4] = m_Points[4];
	((HVec4*)verts)[5] = m_Points[5];
	((HVec4*)verts)[6] = m_Points[6];
	((HVec4*)verts)[7] = m_Points[7];
	verts[0].Transpose();
	verts[1].Transpose();

	static const __m128 idx[2] = { {0,1,2,3}, {4,5,6,7} };
	__m128 idx1=idx[0], idx2=idx[1], mask, bestidx, bestmax, max1, max2;

	// calculate maximum
	(HVec4&)max1 = vNormal.X() * verts[0].AX() + vNormal.Y() * verts[0].AY() + vNormal.Z() * verts[0].AZ();
	(HVec4&)max2 = vNormal.X() * verts[1].AX() + vNormal.Y() * verts[1].AY() + vNormal.Z() * verts[1].AZ();

	// calculate max of the 8 elements and carry over the correct indices
	mask = _mm_cmpgt_ps(max1, max2);
	bestidx = _mm_or_ps(_mm_and_ps(mask,idx1), _mm_andnot_ps(mask, idx2));
	bestmax = _mm_max_ps(max1, max2);

	// unpack
	max1 = _mm_unpacklo_ps(bestmax, bestmax); max2 = _mm_unpackhi_ps(bestmax, bestmax);
	idx1 = _mm_unpacklo_ps(bestidx, bestidx); idx2 = _mm_unpackhi_ps(bestidx, bestidx);

	// calculate max of the 4 elements and carry over the correct indices
	mask = _mm_cmpgt_ps(max1, max2);
	bestidx = _mm_or_ps(_mm_and_ps(mask,idx1), _mm_andnot_ps(mask, idx2));
	bestmax = _mm_max_ps(max1, max2);

	// unpack
	max1 = _mm_unpacklo_ps(bestmax, bestmax); max2 = _mm_unpackhi_ps(bestmax, bestmax);
	idx1 = _mm_unpacklo_ps(bestidx, bestidx); idx2 = _mm_unpackhi_ps(bestidx, bestidx);

	// calculate max of the 2 elements and carry over the correct indices
	mask = _mm_cmpgt_ps(max1, max2);
	bestidx = _mm_or_ps(_mm_and_ps(mask,idx1), _mm_andnot_ps(mask, idx2));

	// unpack index
	int i = _mm_cvtss_si32(bestidx);
	*pI = i;
	return m_Points[i];
}
#endif


/// calculate the point on the convex hull that is farthest along the support axis
HVec3 ConvexBase::ProjectSupportAxis(const C_Normal &vNormal, int *pI) const
{
	assert(m_Points.Size() > 0);
#if defined(PLATFORM_PC)
	if (m_Points.Size() == 8 && g_bProjectSupportSIMD)
		return ProjectSupportAxisSIMD8(vNormal, pI);
#endif

	HF32 fMax = vNormal | m_Points[0];
	int iMax = 0;
	for (int i=1; i<m_Points.Size(); i++)
	{
		HF32 f = vNormal | m_Points[i];
		if (f > fMax)
		{
			fMax = f;
			iMax = i;
		}
	}

	*pI = iMax;

	return m_Points[iMax];		// return actual point
}

/// calculate the extent of the convex hull in the given direction
HF32 ConvexBase::ProjectExtent(const C_Normal &vNormal) const
{
	assert(m_Points.Size() > 0);

	HF32 fMax = vNormal | m_Points[0];
	for (int i=1; i<m_Points.Size(); i++)
	{
		HF32 f = vNormal | m_Points[i];
		fMax = fMax.Max(f);
	}

	return fMax;
}

/// calculate bounding radius
void ConvexBase::CalculateBoundingRadius()
{
	HF32 fMax = 0;
	for (int i=0; i<m_Points.Size(); i++)
		fMax = fMax.Max(m_Points[i].Length());

	m_fRadius = fMax;
}

}; // namespace Impact
