#include "stdafx.h"

#include <core/containers/bitvector.h>
#include "vis_core/PolygonCluster.h"


static const F32 g_maxClusterArea = 2500.f;	//50mx50m
static const F32 g_maxClusterRadius = 50.f;	//50m

struct ClusterData
{
	ClusterData() : m_index(-1),m_area(0.f),m_pResults(NULL) { }

	void AddPolygon(U32 polyIndex, F32 area, const Aftk::C_Bounds& bounds)
	{
		assert(m_pResults);
		assert(m_index!=-1);

		(*m_pResults)[polyIndex] = m_index;
		m_area += area;
		m_bounds += bounds;
	}

	S32						m_index;
	F32						m_area;
	Aftk::C_Bounds			m_bounds;
	PolygonClusterMap*	m_pResults;
};

struct PolyPair
{
	U32 startPoly;
	U32 adjacentPoly;

	PolyPair(U32 s, U32 a) : startPoly(s), adjacentPoly(a) {};
};


Aftk::C_Bounds GetPolygonBounds(const Aftk::C_SceneMeshNode *pMesh, const Aftk::C_Polygon& p)
{
	Aftk::C_Bounds bounds;
	for (U32 i=0; i<p.GetNumPolygonVertices(); ++i)
	{
		const Aftk::C_PolygonVertex& pv = p.GetPolygonVertex(i);
		Position3 pos = pMesh->GetVertexPosition(pv.GetPositionIndex());

		bounds += Aftk::ApplyMatrix(pMesh->GetWorldMatrix(), pos);
	}

	return bounds;
}

bool CanAddPolygon(const Aftk::C_SceneMeshNode *pMesh, U32 currentPoly, U32 candidatePoly, const ClusterData& data)
{
	const Aftk::C_Polygon& p1 = pMesh->GetPolygon(currentPoly);
	const Aftk::C_Polygon& p2 = pMesh->GetPolygon(candidatePoly);

	//do not merge across material boundaries
	//if (p1.GetMaterialMappingIndex() != p2.GetMaterialMappingIndex() )
	//	return false;

	//allow if orginated from the same base polygon pre-triangulation
	if (p1.GetBasePolygonIndex() == p2.GetBasePolygonIndex() && p2.GetBasePolygonIndex() != -1)
		return true;

	//does adding the poly exceed the max allowable cluster area?
	if (p2.GetArea() + data.m_area > g_maxClusterArea)
		return false;

	//compare face normals
	//Aftk::Vector3F32 n1=(Aftk::Vector3F32&)p1.GetNormal(), n2=(Aftk::Vector3F32&)p2.GetNormal();
	
	//F32 dot = n1|n2;
	//return dot > cosf(Maths::DegToRad(15.f));

	if (data.m_bounds.GetMin().x > data.m_bounds.GetMax().x)
		return true;	//empty
	else
	{
		Aftk::C_Bounds bounds = data.m_bounds + GetPolygonBounds(pMesh, p2);
		if (bounds.GetBoundingBoxRadiusSquared() > g_maxClusterRadius*g_maxClusterRadius)
			return false;
	}

	return true;
}

U32 GrowCluster(const Aftk::C_SceneMeshNode *pSceneMesh, ClusterData& clusterData, U32 startPolyIdx)
{
	Ut::BitVector processed;
	processed.Resize(pSceneMesh->GetNumPolygons(0));
	processed.ClearBits();

	processed.SetBit(startPolyIdx);

	U32 nPolysAdded = 1;

	//breadth first traversal through adjacent polys...
	//initialise stack
	Ut::UtList< PolyPair > polyStack;

	const Aftk::C_Polygon& startPoly = pSceneMesh->GetPolygon(0, startPolyIdx);
	for (U32 i=0; i<startPoly.GetNumPolygonVertices(); ++i)
	{
		U32 idx = startPoly.GetNumPolygonVertices()-i-1;
		S32 adjPolyIdx = startPoly.GetAdjacentPolygonIndex(idx);
		if (adjPolyIdx != -1 && (*clusterData.m_pResults)[adjPolyIdx] == -1)
			polyStack.PushBack(PolyPair(startPolyIdx, adjPolyIdx));
	}

	while (polyStack.Size())
	{
		PolyPair top = polyStack.Front();
		polyStack.PopFront();

		if (processed.GetBitValue(top.adjacentPoly))
			continue;	//already considered the poly for cluster

		processed.SetBit(top.adjacentPoly);
		if ((*clusterData.m_pResults)[top.adjacentPoly] != -1)
			continue;	//already belongs to a cluster

		if (CanAddPolygon(pSceneMesh, top.startPoly, top.adjacentPoly, clusterData))
		{
			const Aftk::C_Polygon &poly = pSceneMesh->GetPolygon(0, top.adjacentPoly);
			clusterData.AddPolygon(top.adjacentPoly, poly.GetArea(), GetPolygonBounds(pSceneMesh, poly));

			nPolysAdded++;

			//push further polys onto the stack
			for (U32 i=0; i<poly.GetNumPolygonVertices(); ++i)
			{
				U32 idx = poly.GetNumPolygonVertices()-i-1;
				S32 adjPolyIdx = poly.GetAdjacentPolygonIndex(idx);
				if (adjPolyIdx != -1 && (*clusterData.m_pResults)[adjPolyIdx] == -1 && !processed.GetBitValue(adjPolyIdx))
					polyStack.PushBack(PolyPair(top.adjacentPoly, adjPolyIdx));
			}
		}
	}

	return nPolysAdded;
}

U32 BuildPolygonClusterMap(const Aftk::C_SceneMeshNode *pSceneMesh, PolygonClusterMap& results)
{
	if (!pSceneMesh->GetNumPolygons(0))
		return 0;

	results.Resize(pSceneMesh->GetNumPolygons(0));
	memset(&results[0], -1, sizeof(S32)*pSceneMesh->GetNumPolygons(0));

	U32 nClusters = 0;
	U32 nPolysProcessed = 0;
	for (U32 i=0; i<pSceneMesh->GetNumPolygons(0); ++i)
	{		
		if (results[i] != -1)
			continue;

		ClusterData clusterData;
		clusterData.m_index = nClusters++;
		clusterData.m_pResults = &results;

		const Aftk::C_Polygon &poly = pSceneMesh->GetPolygon(0, i);	
		clusterData.AddPolygon(i, poly.GetArea(), GetPolygonBounds(pSceneMesh, poly));

		nPolysProcessed += GrowCluster(pSceneMesh, clusterData, i);
	}

	return nClusters;
}