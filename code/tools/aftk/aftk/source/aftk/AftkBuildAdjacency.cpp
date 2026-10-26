#include <stdafx.h>

#include <aftk/AftkBuildAdjacency.h>

using namespace Aftk;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

C_BuildAdjacency::C_BuildAdjacency()
{
	m_nLodLevels = 0;
	m_pMesh = 0;
}

// destroy intermediate data
C_BuildAdjacency::~C_BuildAdjacency()
{
	for (U32 i = 0; i < m_nLodLevels; i++)
	{
		delete [] m_ppVertexUsageList[i];
	}
	delete [] m_ppVertexUsageList;
}

// find a polygon that shares this edge
int C_BuildAdjacency::GetAdjacentPolyIndex(U32 lodLevel, U32 iThisPoly, U32 iVtx1, U32 iVtx2) const
{
	int iAdjacentPolyIndex = -1;
	int iIndexDifference = 0;

	// iterate thru polygons that share the first vertex of the edge
	for (int iList=0; iList < m_ppVertexUsageList[lodLevel][iVtx1].Size(); iList++)
	{
        U32 iOtherPoly = m_ppVertexUsageList[lodLevel][iVtx1][iList];

		if (iOtherPoly == iThisPoly) continue;

		const C_Polygon *pOtherPoly = &m_pMesh->GetPolygon(lodLevel, iOtherPoly);

		// does the other polygon contain the 2nd vertex as well?
		for (U32 k=0; k<pOtherPoly->GetNumPolygonVertices(); k++)
		{
			// get index of position of the other polygon
			U32 iPosVtx = pOtherPoly->GetPolygonVertex(k).GetPositionIndex();

			if (iPosVtx == iVtx2)
			{
				// if there's more than one adjacent polygon then return the one with the nearest index
				// TODO: take into account normals - try and find the best match adjacent polygon
				int diff = iOtherPoly - iThisPoly;
				if (diff<0) diff=-diff;
				if (iAdjacentPolyIndex == -1 || diff<iIndexDifference)
				{
					iAdjacentPolyIndex = iOtherPoly;
					iIndexDifference = diff;
				}
			}
		}
	}

	return iAdjacentPolyIndex; // return result
}

//
void C_BuildAdjacency::FindAdjacentPolys(U32 lodLevel, U32 polyIndex)
{
	// find polygons that share one vertex
	const C_Polygon *pPoly = &m_pMesh->GetPolygon(lodLevel, polyIndex);
	int n = m_pMesh->m_adjacencyList.Size();
	pPoly->SetAdjacencyIndex(n);

	U32 nVerts = pPoly->GetNumPolygonVertices();
	for (U32 j=0; j < nVerts; j++)
	{
		U32 iVtx1 = pPoly->GetPolygonVertex(j).GetPositionIndex();
		U32 iVtx2 = pPoly->GetPolygonVertex((j+1)%nVerts).GetPositionIndex();

		U32 iAdjacentPoly = GetAdjacentPolyIndex(lodLevel, polyIndex, iVtx1, iVtx2);
		m_pMesh->m_adjacencyList.PushBack( iAdjacentPoly );
	}
}

// 
bool C_BuildAdjacency::GenerateAdjacency(const C_SceneMeshNode *pMesh)
{
	m_pMesh			= pMesh;
	m_bValidMesh	= true;

	// the usage list allows us to search for adjacent polygons more quickly
	GenerateUsageList();

	for (U32 iLodLevel = 0; iLodLevel < pMesh->GetNumLodLevels(); iLodLevel++)
	{
		// for each polygon, find another polygon that shares two vertices
		for (U32 jPoly = 0; jPoly < pMesh->GetNumPolygons(iLodLevel); jPoly++)
		{
			FindAdjacentPolys(iLodLevel, jPoly);
		}
	}

	return true;
}

// make a list of face usage by vertex
void C_BuildAdjacency::GenerateUsageList()
{
	m_nLodLevels = m_pMesh->GetNumLodLevels();
	U32 nPosVerts = m_pMesh->GetNumVertexPositions();
	
	m_ppVertexUsageList = new intlist*[m_nLodLevels];
	for (U32 iLodLevel = 0; iLodLevel < m_nLodLevels; iLodLevel++)
	{
		m_ppVertexUsageList[iLodLevel] = new intlist[nPosVerts];

		for (U32 i = 0; i < nPosVerts; i++)
		{
			m_ppVertexUsageList[iLodLevel][i].Reserve(10);
		}

		// find polygons that share one vertex
		for (U32 jPoly = 0; jPoly < m_pMesh->GetNumPolygons(iLodLevel); jPoly++)
		{
			const C_Polygon *pPoly = &m_pMesh->GetPolygon(iLodLevel, jPoly);

			for (U32 kPolyVert = 0; kPolyVert < pPoly->GetNumPolygonVertices(); kPolyVert++)
			{
				S32 iPosVtx = pPoly->GetPolygonVertex(kPolyVert).GetPositionIndex();

				AFTK_ASSERT(iPosVtx>=0);

				// add the polygon to this vert's list
				m_ppVertexUsageList[iLodLevel][iPosVtx].PushBack(jPoly);
			}
		}
	}
}

#endif

