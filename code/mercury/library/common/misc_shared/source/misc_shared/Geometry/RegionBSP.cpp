#include "MiscSharedPch.h"

#include <core/core.h>
#include <system/DebugLog.h>
#include <misc_shared/geometry/RegionBSP.h>
#include <misc_shared/geometry/regionbsp.inl>

namespace RegionBSP
{

Debug::Logger &g_Log = Debug::GetLogger("mercury.misc_shared.regionbsp");

// C_Polyhedron

C_Polyhedron::C_Polyhedron()
{
}

C_Polyhedron::~C_Polyhedron()
{
}

S32	C_Polyhedron::AddVertex(const Maths::HVec3& pos)
{
	for(S32 i=0; i<m_VertexList.Size(); i++)
	{
		if( m_VertexList[i].Distance(pos) < 0.001f)
		{
			return i;
		}
	}

	S32 index = m_VertexList.Size();
	m_VertexList.PushBack(pos);
	return index;
}

C_Polyhedron::C_PolyhedronPolygon& C_Polyhedron::AddPolygon()
{
	S32 index = m_PolygonList.Size();
	m_PolygonList.PushBack();
	return m_PolygonList[index];
}

void C_Polyhedron::AddVertexToPolygon(C_Polyhedron::C_PolyhedronPolygon& polygon, const S32 vertexId)
{
	polygon.PushBack(vertexId);
}

class C_Edge
{
	public:
		S32 m_IndexA;
		S32 m_IndexB;
};

const bool C_Polyhedron::IsClosed()
{

	Ut::Vector<C_Edge> edgeList;

	for(S32 polyIndex=0; polyIndex<m_PolygonList.Size(); polyIndex++)
	{
		C_PolyhedronPolygon& polygon = m_PolygonList[polyIndex];
		const S32 polyVertexCount = polygon.Size();

		if(polyVertexCount > 0)
		{
			S32 prevIndex = polygon[polyVertexCount - 1];

			for(S32 polyVertexIndex=0; polyVertexIndex<polyVertexCount; polyVertexIndex++)
			{
				S32 nextIndex = polygon[polyVertexIndex];
				
				S32 foundEdge = -1;
				for(S32 edgeIndex=0; edgeIndex<edgeList.Size(); edgeIndex++)
				{
					C_Edge& testEdge = edgeList[edgeIndex];
					if	(
							( (testEdge.m_IndexA == prevIndex) && (testEdge.m_IndexB == nextIndex) ) ||
							( (testEdge.m_IndexA == nextIndex) && (testEdge.m_IndexB == prevIndex) )
						)
					{
						foundEdge = edgeIndex;
						break;
					}
				}

				if(foundEdge == -1)
				{
					// edge not found, add new edge
					S32 newEdgeIndex = edgeList.Size();
					edgeList.PushBack();
					edgeList[newEdgeIndex].m_IndexA = prevIndex;
					edgeList[newEdgeIndex].m_IndexB = nextIndex;
				}

				prevIndex = nextIndex;
			}
		}
	}

	S32 noEdges = edgeList.Size();
	S32 noFaces = m_PolygonList.Size();
	S32 noVertices = m_VertexList.Size();

	if((noVertices + noFaces - noEdges) != 2)
	{
		return false;
	}

	return true;
}

void C_Polyhedron::DebugRender( const Maths::HMat43& mat, DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour )
{
	const S32 polygonCount = m_PolygonList.Size();
	for(S32 polygonNo=0; polygonNo<polygonCount; polygonNo++)
	{
		C_Polyhedron::C_PolyhedronPolygon& polyhedronPolygon = m_PolygonList[polygonNo]; // peter polyhedron picked a peck of pickled polygons

		const S32 vertexCount = polyhedronPolygon.Size();
		Maths::HVec3 prevPos = m_VertexList[polyhedronPolygon[vertexCount-1]] * mat;
		for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
		{
			Maths::HVec3 currPos = m_VertexList[polyhedronPolygon[vertexNo]] * mat;

			debugRenderPtr->Draw3dLine(prevPos, currPos, colour);
			prevPos = currPos;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Building Code
/////////////////////////////////////////////////////////////////////////////


/////////////////////
// Build Class Definitions
/////////////////////

// C_BuildRegion

enum E_BuildState
{
	E_BuildState_Empty = 0, // region represents empty unclassified space
	E_BuildState_Visited, // region has been visited during the polyhedron insertion, potential for flagging leaf node
	E_BuildState_Set // region now represents a set polyhedron and should not be modified further
};

// BSP tree defined by a root region optionally containing split planes which subdivide space into two sub-regions
// leaf regions represent a convex region of space
class C_BuildRegion
{
friend class C_BuildSplitPlane;
friend class C_BuildTree;
public:
	C_BuildRegion();
	~C_BuildRegion();

	void							AddPolyhedron( C_Polyhedron& polyhedron, const C_RegionId regionId );
	void							Insert( SimplePoly::C_Polygon& polygon );
	void							LabelLeafNodes( const C_RegionId regionId );

public:	
	C_BuildTree*					m_pTree;
	C_RegionId						m_RegionId;
	C_BuildSplitPlane*				m_pSplitPlane;
	E_BuildState					m_BuildState;
};


// C_BuildSplitPlane

class C_BuildSplitPlane
{
public:
	C_BuildTree*						m_pTree;
	S32									m_Index;
    Maths::C_Plane                      m_Plane;
	C_BuildRegion						m_Front;
	C_BuildRegion						m_Back;
};


/////////////////////
// Building Implementation
/////////////////////

// C_BuildRegion

C_BuildRegion::C_BuildRegion()
	:	m_pTree(NULL),
		m_RegionId(-1),
		m_pSplitPlane(NULL),
		m_BuildState(E_BuildState_Empty)
{
}

C_BuildRegion::~C_BuildRegion()
{
	SAFE_DELETE_PTR( m_pSplitPlane );
}

void C_BuildRegion::AddPolyhedron( C_Polyhedron& polyhedron, const C_RegionId regionId )
{
	// add faces one by one to bsp tree...
	const S32 polygonCount = polyhedron.m_PolygonList.Size();
	for(S32 polygonNo=0; polygonNo<polygonCount; polygonNo++)
	{
		C_Polyhedron::C_PolyhedronPolygon& polyhedronPolygon = polyhedron.m_PolygonList[polygonNo]; // peter polyhedron picked a peck of pickled polygons
		const S32 vertexCount = polyhedronPolygon.Size();

		SimplePoly::C_Polygon clipPolygon;
		for(S32 vertexNo=0; vertexNo<vertexCount; vertexNo++)
		{
			S32 polyhedronPolyVertexIndex = polyhedronPolygon[vertexNo];
			clipPolygon.NewVertex( polyhedron.m_VertexList[polyhedronPolyVertexIndex] );
		}

		clipPolygon.CalculatePlane();
		Insert( clipPolygon );
	}

	LabelLeafNodes( regionId );
}

void C_BuildRegion::LabelLeafNodes( const C_RegionId regionId )
{	
	if(m_pSplitPlane)
	{
		// dont label split nodes
		m_RegionId = -1;

		m_pSplitPlane->m_Front.LabelLeafNodes( regionId );
		m_pSplitPlane->m_Back.LabelLeafNodes( regionId );

		if(m_BuildState == E_BuildState_Visited)
		{
			m_BuildState = E_BuildState_Empty;
		}
	}
	else
	{
		if(m_BuildState == E_BuildState_Visited)
		{
			m_RegionId = regionId;
			m_BuildState = E_BuildState_Set;
		}
	}

}

void C_BuildRegion::Insert( SimplePoly::C_Polygon& polygon )
{
	if(m_BuildState == E_BuildState_Set)
	{
		return;
	}

	if(!m_pSplitPlane)
	{
		// reached leaf region, set this polygon's plane as the split plane
		m_pSplitPlane = m_pTree->NewSplitPlane();
		m_pSplitPlane->m_pTree = m_pTree;
	
		m_pSplitPlane->m_Plane = polygon.Plane();
		//m_pSplitPlane->m_DebugPolygon = polygon;

		// set new split node regions to be parent region
		m_pSplitPlane->m_Front.m_RegionId = m_RegionId;
		m_pSplitPlane->m_Back.m_RegionId = m_RegionId;

		m_pSplitPlane->m_Front.m_pTree = m_pTree;
		m_pSplitPlane->m_Back.m_pTree = m_pTree;

		// label front leaf node, the poly normal defines the region we are building so we set the front region
		m_pSplitPlane->m_Front.m_BuildState = E_BuildState_Visited;
	}
	else
	{
		//F32 epsilon = 0.000001f;
		F32 epsilon = 0.001f;// default
		//F32 epsilon = 0.01f;
		//F32 epsilon = 0.5f;

		const SimplePoly::E_PolyPlaneTest polyPlaneResult = polygon.Classify( m_pSplitPlane->m_Plane, epsilon );

		switch(polyPlaneResult)
		{
			case SimplePoly::E_PolyPlaneTest_Coplanar:
			{
				// if back node not already split
				if(!m_pSplitPlane->m_Back.m_pSplitPlane)
				{
					// check normal of poly
					Maths::C_Normal polyNormal = polygon.Plane().Normal();
					Maths::C_Normal planeNormal = m_pSplitPlane->m_Plane.Normal();
					F32 dot = polyNormal | planeNormal;
					if(dot < 0.0f)
					{
						// poly faces other way to split plane, flag split plane back region if leaf

						// discard if region already flagged
						if(m_pSplitPlane->m_Back.m_BuildState != E_BuildState_Set)
						{
							m_pSplitPlane->m_Back.m_BuildState = E_BuildState_Visited;
						}
					}
				}
			}
			break;

			case SimplePoly::E_PolyPlaneTest_InFront:
				m_pSplitPlane->m_Front.Insert( polygon );
			break;

			case SimplePoly::E_PolyPlaneTest_Behind:
				m_pSplitPlane->m_Back.Insert( polygon );
			break;

			case SimplePoly::E_PolyPlaneTest_Straddled:
			{
				SimplePoly::C_Polygon frontPoly;
				SimplePoly::C_Polygon backPoly;        
				
				polygon.Split( m_pSplitPlane->m_Plane, epsilon, frontPoly, backPoly );
			    
				if(frontPoly.m_VertexList.Size() >= 3)
				{
					m_pSplitPlane->m_Front.Insert( frontPoly );
				}

				if(backPoly.m_VertexList.Size() >= 3)
				{
					m_pSplitPlane->m_Back.Insert( backPoly );
				}
			}
			break;
		}
	}
}

// C_BuildTree

C_BuildTree::C_BuildTree()
{
	m_pRootRegion = NULL;
}

C_BuildTree::~C_BuildTree()
{
	SAFE_DELETE_PTR(m_pRootRegion);
}

void C_BuildTree::Initialise( const C_RegionId regionId )
{
	m_pRootRegion = new C_BuildRegion();
	m_pRootRegion->m_RegionId = regionId;
	m_pRootRegion->m_pTree = this;
}

void C_BuildTree::AddPolyhedron( C_Polyhedron& polyhedron, const C_RegionId regionId )
{
	assert(m_pRootRegion);
	m_pRootRegion->AddPolyhedron( polyhedron, regionId );
}

C_BuildSplitPlane* C_BuildTree::NewSplitPlane()
{
	C_BuildSplitPlane* pNewBuildSplitPlane = new C_BuildSplitPlane();
	pNewBuildSplitPlane->m_Index = m_pSplitPlanes.Size();
	m_pSplitPlanes.PushBack(pNewBuildSplitPlane);
	return pNewBuildSplitPlane;
}


C_RaytraceParameters::C_RaytraceParameters()
{
}

C_RaytraceParameters::C_RaytraceParameters(	
	const Maths::C_Vertex&  worldStart,
	const Maths::C_Vertex&  worldEnd,
	const Maths::C_Vector&  worldVector,
	const Maths::C_Normal&  worldNormal,
	const Maths::HF32&      length,
	const C_RegionId		solidRegion)
	:	m_WorldStart(worldStart),
		m_WorldEnd(worldEnd),
		m_WorldVector(worldVector),
		m_WorldNormal(worldNormal),
		m_Length(length),
		m_SolidRegion(solidRegion)
{
}

C_RaytraceResults::C_RaytraceResults()
	:	m_Hit(false)
{
}

// C_Ray

C_Ray::C_Ray()
{
}

C_Ray::~C_Ray()
{
}

C_Ray::C_Ray( const Maths::C_Vertex& rayWorldStart, const Maths::C_Vertex& rayWorldEnd, const C_RegionId solidRegion )
	:	m_Parameters(	rayWorldStart,
                  		rayWorldEnd,
                  		rayWorldEnd - rayWorldStart,
						Maths::C_Normal( rayWorldEnd - rayWorldStart ),
                  		( rayWorldEnd - rayWorldStart ).Length(),
                  		solidRegion)
{
}

C_Ray::C_Ray( const Maths::C_Vertex& rayWorldStart, const Maths::C_Vector& rayWorldVector, const C_RegionId solidRegion )
	:	m_Parameters(	rayWorldStart,
                  		rayWorldStart + rayWorldVector,
                  		rayWorldVector,
                  		Maths::C_Normal( rayWorldVector ),
                  		rayWorldVector.Length(),
                  		solidRegion)
{
}

C_Ray::C_Ray( const Maths::C_Vertex& rayWorldStart, const Maths::C_Normal& rayWorldNormal, const Maths::HF32& rayLength, const C_RegionId solidRegion )
	:	m_Parameters(	rayWorldStart,
						rayWorldStart + rayWorldNormal * rayLength,
						rayWorldNormal * rayLength,
                  		rayWorldNormal,
                  		rayLength,
                  		solidRegion)
{
}



// Main Tree Implementation

C_Tree::C_Tree()
{
	m_RootRegion.m_RegionId = -1;
	m_RootRegion.m_pSplitPlane = NULL;
}

C_Tree::~C_Tree()
{
}

void C_Tree::Build( C_BuildTree& buildTree )
{
	m_RootRegion.m_RegionId = buildTree.m_pRootRegion->m_RegionId;
	const S32 splitPlaneCount = buildTree.m_pSplitPlanes.Size();
	m_SplitPlaneList.Resize( splitPlaneCount );

	if(buildTree.m_pRootRegion->m_pSplitPlane)
	{
		m_RootRegion.m_pSplitPlane = &m_SplitPlaneList[buildTree.m_pRootRegion->m_pSplitPlane->m_Index];
	}
	else
	{
		m_RootRegion.m_pSplitPlane = NULL;
	}

	for(S32 splitPlaneNo=0; splitPlaneNo<splitPlaneCount; splitPlaneNo++)
	{
		C_SplitPlane& splitPlane = m_SplitPlaneList[splitPlaneNo];
		C_BuildSplitPlane& srcSplitPlane = *buildTree.m_pSplitPlanes[splitPlaneNo];

		splitPlane.m_Plane = srcSplitPlane.m_Plane;

		splitPlane.m_Front.m_RegionId = srcSplitPlane.m_Front.m_RegionId;
		splitPlane.m_Back.m_RegionId = srcSplitPlane.m_Back.m_RegionId;

		if(srcSplitPlane.m_Front.m_pSplitPlane)
		{
			splitPlane.m_Front.m_pSplitPlane = &m_SplitPlaneList[srcSplitPlane.m_Front.m_pSplitPlane->m_Index];
		}
		else
		{
			splitPlane.m_Front.m_pSplitPlane = NULL;
		}

		if(srcSplitPlane.m_Back.m_pSplitPlane)
		{
			splitPlane.m_Back.m_pSplitPlane = &m_SplitPlaneList[srcSplitPlane.m_Back.m_pSplitPlane->m_Index];
		}
		else
		{
			splitPlane.m_Back.m_pSplitPlane = NULL;
		}
	}
}

const C_RegionId C_Tree::ClassifyPoint( const Maths::HVec3& pos )
{
	return ClassifyPoint( m_RootRegion, pos );
}

void C_Tree::ClassifyPolygon( SimplePoly::C_Polygon& polygon, Ut::Vector<C_RegionId>& regions)
{
	ClassifyPolygon( m_RootRegion, polygon, regions );
}

const C_RegionId C_Tree::ClassifyPoint( const C_Region& region, const Maths::HVec3& pos )
{
	C_SplitPlane* pSplitPlane = region.m_pSplitPlane;
	if(!pSplitPlane)
	{
		return region.m_RegionId;
	}
	else
	{
		if(pSplitPlane->m_Plane.Distance( pos ) > 0.0f)
		{
			return  ClassifyPoint(pSplitPlane->m_Front, pos);
		}
		else
		{
			return  ClassifyPoint(pSplitPlane->m_Back, pos);
		}
	}
}

void C_Tree::ClassifyPolygon( C_Region& region, SimplePoly::C_Polygon& polygon, Ut::Vector<C_RegionId>& regions)
{
	C_SplitPlane* pSplitPlane = region.GetSplitPlane();
	if( !pSplitPlane )
	{
		// add m_RegionId to the region list
		const S32 regionCount = regions.Size();
		for(S32 regionNo=0; regionNo<regionCount; regionNo++)
		{
			if( regions[regionNo] == region.GetRegionId())
			{
				return;
			}
		}

		regions.PushBack(region.GetRegionId());
	}
	else
	{
		SimplePoly::C_Polygon frontPoly;
		SimplePoly::C_Polygon backPoly;        
		
		F32 epsilon = 0.0001f;
		polygon.Split( pSplitPlane->GetPlane(), epsilon, frontPoly, backPoly );
		
		if(frontPoly.m_VertexList.Size() >= 3)
		{
			ClassifyPolygon(pSplitPlane->GetFront(), frontPoly, regions);
		}

		if(backPoly.m_VertexList.Size() >= 3)
		{
			ClassifyPolygon(pSplitPlane->GetBack(), backPoly, regions);
		}
	}
}

void C_Tree::Raytrace( C_Ray& ray, C_Region& region, const Maths::HVec3& startPos, const Maths::HVec3& endPos, const Maths::C_Normal& hitNormal )
{
	if( startPos.Distance( endPos ) < 0.0001f )
	{
		return;
	}

	C_SplitPlane* pSplitPlane = region.GetSplitPlane();

	// if region not split
	if(pSplitPlane == NULL)
	{
		if(region.m_RegionId == ray.m_Parameters.m_SolidRegion)
		{
			ray.m_Results.m_Hit = true;
			ray.m_Results.m_WorldPos = startPos;
			ray.m_Results.m_WorldNormal = hitNormal;
			ray.m_Results.m_Distance = ray.m_Parameters.m_WorldStart.Distance( startPos );
		}
		else
		{
			ray.m_Results.m_FinalRegion = region.m_RegionId;
		}
	}
	else
	{
		const Maths::C_Plane plane = pSplitPlane->GetPlane();
		Maths::C_Normal planeNormal = plane.Normal();		

		// region has splitplane, trace either side
		const Maths::HF32 startDist = plane.Distance( startPos );
		const Maths::HF32 endDist = plane.Distance( endPos );

		C_Region* pThisSide = NULL;
		C_Region* pOtherSide = NULL;

		if(startDist >= 0.0f)
		{
			pThisSide = &pSplitPlane->m_Front;
		}
		else
		{
			pThisSide = &pSplitPlane->m_Back;

			planeNormal = planeNormal * -1.0f;
		}

		if(endDist >= 0.0f)
		{
			pOtherSide = &pSplitPlane->m_Front;
		}
		else
		{
			pOtherSide = &pSplitPlane->m_Back;
		}

		if(pThisSide == pOtherSide)
		{
			// we dont span the plane, send the entire ray down this side of splitplane
			Raytrace( ray, *pThisSide, startPos, endPos, hitNormal );
		}
		else
		{
			// we span the plane
            const Maths::HF32 t = (startDist) / (startDist - endDist);
            const Maths::HVec3 midPos = startPos + (endPos - startPos) * t;

		    //DbUtil::I_DebugRenderPtr debugRenderPtr = DbUtil::DefaultDebugRenderSys()->Immediate();
			//debugRenderPtr->Draw3dPlane(plane, startPos, 0.25f, Maths::RGBA(0x00, 0xff, 0xff, 0xff));
			//debugRenderPtr->Draw3dPlaneCross(midPos, plane.Normal(), 0.25f, Maths::RGBA(0x00, 0xff, 0xff, 0xff));

			// raytrace near side
			Raytrace( ray, *pThisSide, startPos, midPos, hitNormal );

			// if not hit on near side, raytrace far side
			if(!ray.m_Results.m_Hit)
			{				
				Raytrace( ray, *pOtherSide, midPos, endPos, planeNormal );
			}
		}
	}
}

void C_Tree::Raytrace( C_Ray& ray )
{
	Maths::C_Normal hitNormal( ray.m_Parameters.m_WorldNormal * -1.0f );
	Maths::C_Vertex start( ray.m_Parameters.m_WorldStart );
	Maths::C_Vertex end( ray.m_Parameters.m_WorldEnd );

	ray.m_Results.m_WorldNormal = ray.m_Parameters.m_WorldNormal;
	ray.m_Results.m_WorldPos = ray.m_Parameters.m_WorldEnd;
	ray.m_Results.m_FinalRegion = ray.m_Parameters.m_SolidRegion; 
	ray.m_Results.m_Distance = ray.m_Parameters.m_Length;

	Raytrace( ray, m_RootRegion, start, end, hitNormal );
}


void C_Tree::BuildDebugMesh( const C_RegionId regionId, Ut::Vector<SimplePoly::C_Polygon>& resultPolys )
{
	resultPolys.Clear();

	Ut::Vector<Maths::C_Plane> planeList;

	BuildDebugMesh(regionId, m_RootRegion, planeList, resultPolys);
}

const S32 C_Tree::GetSplitPlaneIndex( C_SplitPlane* pSplitPlane ) const
{
	if(pSplitPlane == NULL)
		return -1;

	const S32 splitPlaneCount = m_SplitPlaneList.Size();
	for(S32 splitPlaneNo=0; splitPlaneNo<splitPlaneCount; splitPlaneNo++)
	{
		if(&m_SplitPlaneList[splitPlaneNo] == pSplitPlane)
			return splitPlaneNo;
	}

	return -1;
}


void C_Tree::BuildDebugMesh( const C_RegionId regionId, C_Region& region, Ut::Vector<Maths::C_Plane>& planeList, Ut::Vector<SimplePoly::C_Polygon>& resultPolys )
{
	//C_Mesh mesh;

	//mesh.AddPlane( Maths::C_Plane( Maths::C_Normal( 0.0f, 1.0f, 0.0f ), Maths::C_Vertex( 0.0f, -1.0f, 0.0f ) ) );
	//mesh.AddPlane( Maths::C_Plane( Maths::C_Normal( 0.0f, -1.0f, 0.0f ), Maths::C_Vertex( 0.0f, 1.0f, 0.0f ) ) );

	//mesh.AddPlane( Maths::C_Plane( Maths::C_Normal( 1.0f, 0.0f, 0.0f ), Maths::C_Vertex( -1.0f, 0.0f, 0.0f ) ) );
	//mesh.AddPlane( Maths::C_Plane( Maths::C_Normal( -1.0f, 0.0f, 0.0f ), Maths::C_Vertex( 1.0f, 0.0f, 0.0f ) ) );

	//mesh.AddPlane( Maths::C_Plane( Maths::C_Normal( 0.0f, 0.0f, 1.0f ), Maths::C_Vertex( 0.0f, 0.0f, -1.0f ) ) );
	//mesh.AddPlane( Maths::C_Plane( Maths::C_Normal( 0.0f, 0.0f, -1.0f ), Maths::C_Vertex( 0.0f, 0.0f, 1.0f ) ) );

	//mesh.Build();
	//mesh.DebugRender( debugRenderPtr, colour );
	
	C_SplitPlane* pSplitPlane = region.GetSplitPlane();

	// if region not split
	if(pSplitPlane == NULL)
	{
		if(region.m_RegionId == regionId)
		{
			// render mesh generated from planeList
			C_Mesh mesh;

			for(S32 i=0; i<planeList.Size(); i++)
			{
				mesh.AddPlane( planeList[i] );
			}

			mesh.Build();

			//mesh.DebugRender( debugRenderPtr, colour );

			for(S32 i=0; i<mesh.m_polys.Size(); i++)
			{
				resultPolys.PushBack(mesh.m_polys[i]);
			}
		}
	}
	else
	{
		planeList.PushBack(pSplitPlane->m_Plane);
		BuildDebugMesh(regionId, pSplitPlane->GetFront(), planeList, resultPolys );
		planeList.PopBack();

		planeList.PushBack(-pSplitPlane->m_Plane);
		BuildDebugMesh(regionId, pSplitPlane->GetBack(), planeList, resultPolys );
		planeList.PopBack();
	}
}



// C_Mesh


void C_Mesh::AddPlane(Maths::C_Plane& plane)
{
	m_planes.PushBack(plane);
}

void C_Mesh::Build()
{
	using namespace Maths;

	for (int iPlane=0; iPlane<m_planes.Size(); iPlane++)
	{
		const C_Plane &plane = m_planes[iPlane];
		HVec3 vecNormal(plane.Normal());
		HVec3 vCentre = plane.OnPlane( C_Origin() );
		HVec3 p,q;

		vecNormal.PerpendicularAxes(p, q);
		
		p = p * 1000.f; // TODO: calculate the necessary scaling factor from the AABB
		q = q * 1000.f;

		// create polygon in the plane

		SimplePoly::C_Polygon polygon;
		C_Vertex vtx;

		vtx = vCentre + p + q;        polygon.NewVertex(vtx);
		vtx = vCentre + p - q;        polygon.NewVertex(vtx);
		vtx = vCentre - p - q;        polygon.NewVertex(vtx);
		vtx = vCentre - p + q;        polygon.NewVertex(vtx);

		// clip the polygon against all the planes
		for (int i=0; i<m_planes.Size(); i++)
		{
			if (i == iPlane)
				continue;

			SimplePoly::C_Polygon front;
			polygon.Clip(m_planes[i], front);
			polygon = front;

			if (polygon.m_VertexList.Size() < 3 )
				  break;
		}

		// if the plane has no vertices for it's polygon, delete it
		if (polygon.m_VertexList.Size() < 3 )
			  continue;

		// store the polygon
		m_polys.PushBack(polygon);
	}
}

void C_Mesh::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA& colour )
{
	for (int i=0; i<m_polys.Size(); i++)
	{
		m_polys[i].DebugRender(debugRenderPtr, colour);
	}
}

} // namespace RegionBSP
