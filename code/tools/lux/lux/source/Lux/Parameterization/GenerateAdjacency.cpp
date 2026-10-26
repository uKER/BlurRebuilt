//-----------------------------------------------------------------------------
// File: GenerateAdjacency.cpp
//
// Desc: Generate Adjacency information for a triangulated mesh
//		 assumes that LMPolyAttribute data has already been added
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "Lux/Parameterization/Box.hpp"
#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/GenerateAdjacency.h"
#include "Lux/Parameterization/LMPolyAttributes.h"

// destroy intermediate data
CGenerateAdjacency::~CGenerateAdjacency()
{
	delete [] m_pVertexUsageList;
}

// find a polygon that shares this edge
int CGenerateAdjacency::GetAdjacentPolyIndex(U32 iThisPoly, U32 iVtx1, U32 iVtx2)
{
	int iAdjacentPolyIndex = -1;

	// iterate thru polygons that share the first vertex of the edge
	for (int iList=0; iList<m_pVertexUsageList[iVtx1].Size(); iList++)
	{
        U32 iOtherPoly = m_pVertexUsageList[iVtx1][iList];

		if (iOtherPoly == iThisPoly) continue;

		Aftk::C_Polygon *pOtherPoly = &m_pMesh->GetPolygon(iOtherPoly);

		// does the other polygon contain the 2nd vertex as well?
		for (int k=0; k<3; k++)
		{
			// get index of position of the other polygon
			U32 iPosVtx = m_pMesh->GetPolygonVertex(pOtherPoly->GetPolygonVertexIndex(k)).GetPositionIndex();

			if (iPosVtx == iVtx2)
			{
				// if there's more than one adjacent polygon then the mesh is ropey
				if (iAdjacentPolyIndex != -1)
					m_bValidMesh = false;

				iAdjacentPolyIndex = iOtherPoly;
			}
		}
	}

	return iAdjacentPolyIndex; // return result
}

//
void CGenerateAdjacency::FindAdjacentPolys(U32 i)
{
	// find polygons that share one vertex
	Aftk::C_Polygon *pPoly = &m_pMesh->GetPolygon(i);

	for (int j=0; j<3; j++)
	{
		U32 iVtx1 = m_pMesh->GetPolygonVertex(pPoly->GetPolygonVertexIndex(j)).GetPositionIndex();
		U32 iVtx2 = m_pMesh->GetPolygonVertex(pPoly->GetPolygonVertexIndex((j+1)%3)).GetPositionIndex();

		POLY_DATA(i)->iAdjacency[j] = GetAdjacentPolyIndex(i, iVtx1, iVtx2);
	}
}

// 
bool CGenerateAdjacency::GenerateAdjacency(Aftk::C_SceneMeshNode *pMesh)
{
	m_pMesh			= pMesh;
	m_bValidMesh	= true;

	// the usage list allows us to search for adjacent polygons more quickly
	GenerateUsageList();

	// for each polygon, find another polygon that shares two vertices
	for (int i=0; i<pMesh->GetNumPolygons(); i++)
		FindAdjacentPolys( i );

	return m_bValidMesh;
}

// make a list of face usage by vertex
void CGenerateAdjacency::GenerateUsageList()
{
	m_pVertexUsageList = new intlist[m_pMesh->GetNumPolygonVertices()];
	for (int i=0; i<m_pMesh->GetNumPolygonVertices(); i++)
		m_pVertexUsageList[i].Reserve(10);

	// find polygons that share one vertex
    for (int i=0; i<m_pMesh->GetNumPolygons(); i++)
	{
		Aftk::C_Polygon *pPoly = &m_pMesh->GetPolygon(i);

		for (int j=0; j<3; j++)
		{
			U32 iVtx = pPoly->GetPolygonVertexIndex(j);
			S32 iPosVtx = m_pMesh->GetPolygonVertex(iVtx).GetPositionIndex();

			assert(iPosVtx>=0);

			// add the polygon to this vert's list
			m_pVertexUsageList[iPosVtx].PushBack(i);
		}
	}
}
