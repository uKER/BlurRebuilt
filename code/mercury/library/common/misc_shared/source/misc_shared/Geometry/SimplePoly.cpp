#include "MiscSharedPch.h"

#include <core/core.h>
#include <system/DebugLog.h>
#include <misc_shared/geometry/SimplePoly.h>
#include <misc_shared/geometry/SimplePoly.inl>

namespace SimplePoly
{
Debug::Logger &g_Log = Debug::GetLogger("mercury.misc_shared.simplepoly");

enum E_PointPlaneTest
{
	E_PointPlaneTest_Behind = 0,
	E_PointPlaneTest_On,
	E_PointPlaneTest_InFront
};

const E_PointPlaneTest ClassifyPoint( const Maths::HF32& dist, const Maths::HF32& epsilon )
{
	if( dist >= epsilon )
	{
		return E_PointPlaneTest_InFront;
	}
	else
	if( dist <= -epsilon )
	{
		return E_PointPlaneTest_Behind;
	}

	return E_PointPlaneTest_On;
}

const E_PointPlaneTest ClassifyPoint( const Maths::HF32& dist )
{
	if( dist >= 0.0f )
	{
		return E_PointPlaneTest_InFront;
	}

	return E_PointPlaneTest_Behind;
}


__forceinline const Maths::HVec3 Intersect( const Maths::HVec3& aPos, const Maths::HF32& aDist, const Maths::HVec3& bPos, const Maths::HF32& bDist )
{
	const Maths::HF32 t = (aDist) / (aDist - bDist);
	return aPos + (bPos - aPos) * t;
}

void C_Polygon::Classify( const Maths::C_Plane& plane, const F32 epsilon, S32& frontCount, S32& backCount )
{
	const S32 vertexCount = m_VertexList.Size();
	const Maths::HF32 hfEpsilon( epsilon );
	
	frontCount = 0;
	backCount = 0;

	for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
    {
		const Maths::HVec3 bPos = m_VertexList[vertexNo];
        const Maths::HF32 bDist = plane.Distance(bPos);
		const E_PointPlaneTest bSide = ClassifyPoint( bDist, hfEpsilon );

		switch(bSide)
		{
			case E_PointPlaneTest_Behind:
				backCount++;
			break;

			case E_PointPlaneTest_On:
			break;

			case E_PointPlaneTest_InFront:
				frontCount++;
			break;
		}
	}
}

const E_PolyPlaneTest C_Polygon::Classify( const Maths::C_Plane& plane, const F32 epsilon )
{
	S32 frontCount;
	S32 backCount;

	Classify(plane, epsilon, frontCount, backCount);

	if((frontCount > 0) && (backCount == 0))
	{
		return E_PolyPlaneTest_InFront;
	}

	if((frontCount == 0) && (backCount > 0))
	{
		return E_PolyPlaneTest_Behind;
	}

	if((frontCount > 0) && (backCount > 0))
	{
		return E_PolyPlaneTest_Straddled;
	}

	if((frontCount == 0) && (backCount == 0))
	{
		return E_PolyPlaneTest_Coplanar;
	}

	assert(false);

	return E_PolyPlaneTest_Coplanar;
}

// Cases for clipping a polygon against a "thick" plane
// from "Real time collision detection" - Christer Ericson

// A (prev)			B (curr)			Front Output	Back Output
// ----------------------------------------------------------------
// In front			In front			B				None
// On				In front			B				None
// Behind			In front			I,B				I
// In front			On					B				None
// On				On					B				None
// Behind			On					B				B
// In front			Behind				I				I, B
// On				Behind				None			A, B
// Behind			Behind				None			B

// Note: A->B clipped against plane produces different results to B->A clipped against plane
// to avoid this, we consistently clip using front->back

void C_Polygon::Split( const Maths::C_Plane& plane, const F32 epsilon, C_Polygon& front, C_Polygon& back )
{
	const S32 vertexCount = m_VertexList.Size();

	const Maths::HF32 hfEpsilon( epsilon );

	front.m_VertexList.Clear();
	back.m_VertexList.Clear();

	// get last vertex
    Maths::HVec3 aPos = m_VertexList[vertexCount - 1];
    Maths::HF32 aDist = plane.Distance(aPos);
	E_PointPlaneTest aSide = ClassifyPoint( aDist, hfEpsilon );

	for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
    {
		const Maths::HVec3 bPos = m_VertexList[vertexNo];
        const Maths::HF32 bDist = plane.Distance(bPos);
		const E_PointPlaneTest bSide = ClassifyPoint( bDist, hfEpsilon );

		if( bSide == E_PointPlaneTest_InFront )
		{
			if( aSide == E_PointPlaneTest_Behind )
			{
				// edge (a, b) straddles plane, output intersection to both sides
				// (intersect (b, a) so we consistently intersect from front->behind)
				const Maths::HVec3 newPos = Intersect( bPos, bDist, aPos, aDist );
				if ( !((epsilon == 0.0f) || ClassifyPoint( plane.Distance(newPos), hfEpsilon) == E_PointPlaneTest_On ))
				{
					LOGWARN(g_Log, "Problem classifying point in C_Polygon::Split() bFront, aBehind, epsilon=%f\n", epsilon);
				}
				front.NewVertex( newPos );
				back.NewVertex( newPos );
			}

			// in all 3 cases, output b to the front side
			front.NewVertex( bPos );
		}
		else
		if( bSide == E_PointPlaneTest_Behind )
		{
			if( aSide == E_PointPlaneTest_InFront )
			{
				// edge (a, b) straddles plane, output intersection
				const Maths::HVec3 newPos = Intersect( aPos, aDist, bPos, bDist );
				if ( !( (epsilon == 0.0f) || ClassifyPoint( plane.Distance(newPos), hfEpsilon) == E_PointPlaneTest_On ))
				{
					LOGWARN(g_Log, "Problem classifying point in C_Polygon::Split() aFront, bBehind, epsilon=%f\n", epsilon);
				}
				front.NewVertex( newPos );
				back.NewVertex( newPos );
			}
			else
			if( aSide == E_PointPlaneTest_On )
			{
				// output a when edge (a, b) goes from 'on' to 'behind' plane
				back.NewVertex( aPos );
			}

			// in all 3 cases, output b to the back side
			back.NewVertex( bPos );
		}
		else
		{
			// b is on the plane. In all three cases output b to the front side
			front.NewVertex( bPos );

			if( aSide == E_PointPlaneTest_Behind )
			{
				// in one case, also output b to back side
				back.NewVertex( bPos );
			}
		}
                
        aPos = bPos;
        aDist = bDist;
		aSide = bSide;
    }    
    
    front.m_Plane = m_Plane;
    back.m_Plane = m_Plane;
}

void C_Polygon::Clip( const Maths::C_Plane& plane, C_Polygon& result )
{
	const S32 vertexCount = m_VertexList.Size();

	result.m_VertexList.Clear();

	// get last vertex
    Maths::HVec3 aPos = m_VertexList[vertexCount - 1];
    Maths::HF32 aDist = plane.Distance(aPos);
	E_PointPlaneTest aSide = ClassifyPoint( aDist );

	for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
    {
		const Maths::HVec3 bPos = m_VertexList[vertexNo];
        const Maths::HF32 bDist = plane.Distance(bPos);
		const E_PointPlaneTest bSide = ClassifyPoint( bDist );

		if( ( aSide == E_PointPlaneTest_Behind ) && ( bSide == E_PointPlaneTest_Behind ) )
		{
			// output nothing
		}
		else
		if( ( aSide == E_PointPlaneTest_InFront ) && ( bSide == E_PointPlaneTest_Behind ) )
		{
			// edge (a, b) straddles plane, output intersection
			// always clip from front->back
			const Maths::HVec3 newPos = Intersect( aPos, aDist, bPos, bDist );				
			result.NewVertex( newPos );
		}
		else
		if( ( aSide == E_PointPlaneTest_Behind ) && ( bSide == E_PointPlaneTest_InFront ) )
		{
			// edge (a, b) straddles plane, output intersection
			// always clip from front->back
			const Maths::HVec3 newPos = Intersect( bPos, bDist, aPos, aDist );				
			result.NewVertex( newPos );

			// output B
			result.NewVertex( bPos );
		}
		else
		if( ( aSide == E_PointPlaneTest_InFront ) && ( bSide == E_PointPlaneTest_InFront ) )
		{
			// output B
			result.NewVertex( bPos );
		}
     
        aPos = bPos;
        aDist = bDist;
		aSide = bSide;
    }    
    
    result.m_Plane = m_Plane;
}

void C_Polygon::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour )
{
	const S32 vertexCount = m_VertexList.Size();
	
	if(vertexCount < 3)
		return;

	Maths::HVec3 prevPos = m_VertexList[vertexCount-1];
	for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
	{
		Maths::HVec3 currPos = m_VertexList[vertexNo];

		debugRenderPtr->Draw3dLine(prevPos, currPos, colour);
		prevPos = currPos;
	}
}

void C_Polygon::DebugRenderSolid( DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour )
{
	const S32 vertexCount = m_VertexList.Size();

	if(vertexCount < 3) 
		return;

	Maths::HVec3 firstVertex = m_VertexList[0];
	Maths::HVec3 prevVertex = m_VertexList[1];

	for(S32 i=2; i<vertexCount; i++)
    {
		Maths::HVec3 currVertex = m_VertexList[i];

        debugRenderPtr->Draw3dTriangle( firstVertex, prevVertex, currVertex, colour);
                            
        prevVertex = currVertex;
    }
}

void C_Polygon::ClosestPoint( const Maths::HVec3& pos, C_ClosestPointInfo& result )
{
	assert(m_VertexList.Size() >= 3);

	//DbUtil::I_DebugRenderPtr debugRenderPtr = DbUtil::DefaultDebugRenderSys()->Deferred();

	const S32 vertexCount = m_VertexList.Size();

	Maths::HVec3 posB = m_VertexList[vertexCount - 1];
	Maths::HVec3 posA = m_VertexList[vertexCount - 2];

	for(S32 edgeNo=0; edgeNo<vertexCount;edgeNo++)
	{
		const Maths::HVec3 posC = m_VertexList[edgeNo];

		// We are testing edge AB and vertex B

		//debugRenderPtr->Draw3dLine( posA, posB, Maths::RGBA(0xff, 0x00, 0xff, 0xff));
		//debugRenderPtr->Draw3dSphereOutline( posB, 0.5f, Maths::RGBA(0xff, 0x00, 0xff, 0xff));

		// get edge direction vectors
		const Maths::C_Vector edgeAB = Maths::C_Vector(posB - posA);
		const Maths::C_Normal edgeDirAB(edgeAB);

		const Maths::C_Vector edgeBC = Maths::C_Vector(posC - posB);
		const Maths::C_Normal edgeDirBC(edgeBC);

		// calculate edge-vertex voronoi planes
		const Maths::C_Plane edgeABvertexAPlane( edgeDirAB, posA );
		const Maths::C_Plane edgeABvertexBPlane( -edgeDirAB, posB );

		const Maths::C_Plane edgeBCvertexBPlane( edgeDirBC, posB );
		// dont need edgeBC vertexC plane

		// calculate edge-face voronoi planes
		const Maths::C_Plane edgeABedgePlane( Maths::C_Normal(edgeDirAB ^ m_Plane.Normal()), posA );

		//debugRenderPtr->Draw3dPlane( edgeABvertexAPlane, posA, 1.0f, Maths::RGBA(0x00, 0xff, 0xff,0xff));
		//debugRenderPtr->Draw3dPlane( edgeABvertexBPlane, posA, 1.0f, Maths::RGBA(0x00, 0xff, 0xff,0xff));
		//debugRenderPtr->Draw3dPlane( edgeBCvertexBPlane, posB, 1.0f, Maths::RGBA(0x00, 0xff, 0xff,0xff));

		//debugRenderPtr->Draw3dPlane( edgeABedgePlane, (posA + posB) * 0.5f, 1.0f, Maths::RGBA(0x00, 0xff, 0xff,0xff));

		// get voronoi plane distances
		const Maths::HF32 edgeABvertexADist = edgeABvertexAPlane.Distance( pos );
		const Maths::HF32 edgeABvertexBDist = edgeABvertexBPlane.Distance( pos );
		const Maths::HF32 edgeBCvertexBDist = edgeBCvertexBPlane.Distance( pos );

		const Maths::HF32 edgeABedgeDist = edgeABedgePlane.Distance( pos );

		// check vertex B planes
		if((edgeABvertexBDist <= Maths::HF32(0.0f)) & (edgeBCvertexBDist <= Maths::HF32(0.0f)))
		{
			// vertex B is closest feature
			result.m_Pos = posB;
			result.m_Feature = E_Feature_Vertex;
			// vertex B index
			result.m_FeatureIndex = (edgeNo + m_VertexList.Size() - 1) % m_VertexList.Size();
			return;
		}
		else
		{
			// check edge AB
			if((edgeABvertexADist > Maths::HF32(0.0f)) && (edgeABvertexBDist > Maths::HF32(0.0f)) && (edgeABedgeDist <= Maths::HF32(0.0f)))
			{
				// edge AB is closest feature
				//result.m_Pos = (posA + posB) * 0.5f;
				result.m_Pos = posA + edgeDirAB * (edgeDirAB | (pos - posA));
				result.m_Feature = E_Feature_Edge;
				// vertex B index
				result.m_FeatureIndex = edgeNo;
				return;
			}
		}

		posA = posB;
		posB = posC;
	}

	// none of the vertices or planes were the closest feature, the face itself must be closest

	result.m_Pos = m_Plane.OnPlane( pos );
	result.m_Feature = E_Feature_Face;
	result.m_FeatureIndex = 0;

	return;
}

void C_Polygon::CalculatePlane()
{
	const S32 vertexCount = m_VertexList.Size();
    if(vertexCount < 3)
        return;

	// use Newell's method to calculate polygon normal

	Maths::HVec3 normal( 0.0f );
	m_Centroid = Maths::HVec3( 0.0f );

    Maths::HVec3 prevPos = m_VertexList[vertexCount - 1];
	for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
	{
	    const Maths::HVec3 currPos = m_VertexList[vertexNo];

		Maths::HF32 nX = (currPos.Y() - prevPos.Y()) * (currPos.Z() + prevPos.Z());
		Maths::HF32 nY = (currPos.Z() - prevPos.Z()) * (currPos.X() + prevPos.X());
		Maths::HF32 nZ = (currPos.X() - prevPos.X()) * (currPos.Y() + prevPos.Y());

		normal += Maths::HVec3( nX, nY, nZ );

		m_Centroid += currPos;

		prevPos = currPos;
	}

	m_Centroid *= (1.0f / (F32)vertexCount);

    m_Plane = Maths::C_Plane( Maths::C_Normal( normal ), Maths::C_Vertex( m_Centroid ) ); 
}

bool C_Polygon::IsPlaner( const F32 epsilon )
{
	const S32 vertexCount = m_VertexList.Size();
	for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
	{
	    const Maths::HVec3 currPos = m_VertexList[vertexNo];

		if( Maths::Abs( m_Plane.Distance(currPos) ) > epsilon )
			return false;
	}

	return true;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize ("", off) // CFG_ALLOW_OPTIMIZE_OFF
#endif

//Create a grid of points in the plane of the polygon, clipped to the edges of the polygon. Optionally align to a specific
//vector in the plane of the polygon for the U direction, default is the first edge. V = UxN 
bool C_Polygon::CreatePointGridOnPoly(C_VertexList& points, F32 gridResolution, Maths::HVec3* pAlignGrid, F32 margin)
{
	using namespace Maths;
	if (m_VertexList.Size() < 3)
	{
		//can't make sense of poly with less than 3 verts, bail
		LOGERROR(g_Log, "Poly with less than 3 verts in C_Polygon::CreatePointGridOnPoly\n");
		return false;
	}
	
	//Get poly plane
	CalculatePlane();
	C_Plane plane(Plane());

	//Get U direction
	C_Normal uVec;
	C_Normal vVec;

	if (pAlignGrid)
	{
		//Make sure vector passed in is in plane of polygon
		uVec = plane.ParallelToPlane( C_Vector(*pAlignGrid));
	}
	else
	{
		//Get first edge and use as U direction
		uVec = m_VertexList[1] - m_VertexList[0];
	}

	//V direction is UxN
	vVec = uVec ^ plane.Normal();

	//centroid is u=v=0
	HVec3 centroid(Centroid());
	HVec3 xyzMin(centroid);
	HVec2 uvMax(-FLT_MAX, -FLT_MAX), uvMin(FLT_MAX, FLT_MAX);

	//Get bounds in UV space, dot each (vertex-centroid) with U and V vectors to find max and min

	for (int i=0; i<m_VertexList.Size(); i++)
	{
		HVec3 localVec(m_VertexList[i] - centroid);

		HF32 localU(localVec | uVec);
		HF32 localV(localVec | vVec);

		if (localU < uvMin[0]) (uvMin[0] = localU);
		if (localV < uvMin[1]) (uvMin[1] = localV);
		if (localU > uvMax[0]) (uvMax[0] = localU);
		if (localV > uvMax[1]) (uvMax[1] = localV);
	}

	//Get bounds expressed in 3D xyz

	xyzMin += (uVec*uvMin[0]);
	xyzMin += (vVec*uvMin[1]);

	//Create grid between bounds, and if within all edges of tri, add to list

	C_PlaneList edgePlanes;
	CreateEdgePlanes(edgePlanes);

	HF32 deltaU = uvMax[0] - uvMin[0];
	HF32 deltaV = uvMax[1] - uvMin[1];

	HF32 gridU = deltaU * HF32(gridResolution);
	HF32 gridV = deltaV * HF32(gridResolution);
	
	HF32 uOffset(0.0f), vOffset(0.0f);
	HVec3 pointPos;

	while (uOffset < deltaU)
	{
		while (vOffset < deltaV)
		{
			pointPos = xyzMin + (uVec * uOffset) + (vVec * vOffset);
			if (IsPointWithinEdgePlanes(pointPos, edgePlanes, margin))
			{
				points.PushBack(pointPos);
			}
			vOffset += gridV;
		}
		uOffset += gridU;
		vOffset = 0.0f;
	}

	return true;
}


void C_Polygon::CreateEdgePlanes(C_PlaneList& planes) 
{
	using namespace Maths;
	
	CalculatePlane();
	C_Plane plane(Plane());
	C_Normal planeNormal(plane.Normal());

	const S32 nVerts = m_VertexList.Size();

	for (S32 i=0; i<nVerts; i++)
	{
		C_Normal edgeDir(m_VertexList[(i+1)%nVerts] - m_VertexList[i]);
		C_Plane edgePlane(C_Normal(edgeDir ^ planeNormal), m_VertexList[i]);
		planes.PushBack(edgePlane);
	}
}

bool C_Polygon::IsPointWithinEdgePlanes(const Maths::HVec3& point, const C_PlaneList& planes, F32 margin) 
{
	using namespace Maths;
	for (int i=0; i<planes.Size(); i++)
	{
		if (planes[i].Distance(C_Vertex(point)) < HF32(margin))
		{
			return false;
		}
	}

	return true;
}

/// Return true if the point is inside the polygon.
const bool C_Polygon::ContainsPoint(const Maths::HVec3& rkPoint, const F32 fTolerance)
{
	C_PlaneList list;
	CreateEdgePlanes(list);
	const bool bWithinEdge = IsPointWithinEdgePlanes(rkPoint, list, fTolerance);
	if(!bWithinEdge)
		return false;

	if(m_Plane.Distance(rkPoint) < fTolerance)
		return true;
	else
		return false;
}

/// Reserve storage for a specified number of polygons.
void C_Polygon::Reserve(U32 iSize)
{
	m_VertexList.Reserve(iSize);
	return;
}

};
