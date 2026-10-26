#ifndef __PHYSICS_COLLISION_UTIL_INL__
#define __PHYSICS_COLLISION_UTIL_INL__

namespace Impact
{

enum kPlaneMode
{
	k_iSect0,
	k_iSect1,
	k_iSect2,
	k_Invalid,
};

struct PlaneID
{
	int iMode, i1, i2;
};

// convert in/out bits to mode + isect planes indices
extern PlaneID iBitsToEdge[16];

// alternative clipping function
template<typename T>
void ClipPolyToPlanesFast(C_ClipPolygon &clippedPoly, T *pRefHull, C_ClipPolygon &incidentPoly, int iRefPlane)
{
	HMat44 planes;

	// load planes into SOA format
	int n = pRefHull->NumSidePlanes(iRefPlane);
	int i;
	for (i=0; i<n; i++)
		((HVec4*)&planes)[i] = pRefHull->GetSidePlane(iRefPlane,i);
	for (;i<4; i++)
		((HVec4*)&planes)[i] = HVec4(0,0,0, -FLT_MAX);
	planes.Transpose();

	// clip each segment
	n = incidentPoly.m_Vertices.Size();
	HVec4 vA = (HVec4)incidentPoly.m_Vertices[n-1];
	vA.W(1.f);
	HVec4 vABits = vA * planes;
	int iABits = _mm_movemask_ps(vABits);
	for (int i=0; i<n ;i++)
	{
		HVec4 vB = (HVec4)incidentPoly.m_Vertices[i];
		vB.W(1.f);
		HVec4 vBBits = vB * planes;
		int iBBits = _mm_movemask_ps(vBBits);
		int iXorBits = iABits ^ iBBits;
		// if vA is completely inside, add it
		if (iABits == 0)
		{
			clippedPoly.m_Vertices.PushBack(); 
			clippedPoly.m_Vertices.Back() = (HVec3)vA;
		}
		if (iXorBits)
		{
			// if this edge intersects one of the planes, add the intersection point(s)
		    HVec4 vAB = vB - vA;
			float *pfABits = (float*)&vABits;
			float *pfBBits = (float*)&vBBits;
			PlaneID id = iBitsToEdge[iXorBits];
			if (id.iMode == k_iSect2)
			{
				HF32 f = pfABits[id.i2] / (pfABits[id.i2] - pfBBits[id.i2]); 
				clippedPoly.m_Vertices.PushBack(); 
				clippedPoly.m_Vertices.Back() = (HVec3&)(vA + HVec4(f) * vAB);
			}
			HF32 f = pfABits[id.i1] / (pfABits[id.i1] - pfBBits[id.i1]); 
			clippedPoly.m_Vertices.PushBack(); 
			clippedPoly.m_Vertices.Back() = (HVec3&)(vA + f * vAB);
		}
	}

};



// default clipping function
template<typename T>
void ClipPolyToPlanes(C_ClipPolygon &clippedPoly, T *pRefHull, C_ClipPolygon &incidentPoly, int iRefPlane)
{
	C_ClipPolygon *pCurPoly = &incidentPoly, tmp, *pTmpPoly = &tmp, *pTmp;

	int n = pRefHull->NumSidePlanes(iRefPlane);
//	if (n <= 4)
//		return ClipPolyToPlanesFast(clippedPoly, pRefHull, incidentPoly, iRefPlane);

	for (int i=0; i<n; i++)
	{
		if (i == n-1)
			pTmpPoly = &clippedPoly;

		// get the side-plane from the convex hull
		ClipPlane plane = pRefHull->GetSidePlane(iRefPlane,i);
		plane.edge = (iRefPlane<<4) + i;

		// split the incident polygon along the side-plane
		SplitPolygonFastSIMD(*pCurPoly, plane, *pTmpPoly);

		// swap pCurPoly and pTmpPoly
		pTmp = pCurPoly; pCurPoly = pTmpPoly; pTmpPoly = pTmp;
	}
};

};

#endif
