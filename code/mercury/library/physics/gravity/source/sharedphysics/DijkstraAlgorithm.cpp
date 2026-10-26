#include "stdafx.h"
#include "gravity/sharedphysics/DijkstraAlgorithm.h"
#include "gravity/sharedphysics/Select.h"

namespace SharedPhysics
{

	void C_DijkstraShortestPath::GetShorestPath(C_GraphVertex* pVertexA, C_GraphVertex* pVertexB)
	{
		F32 max = Maths::Constants<F32>::Max(); 
		for(S32 i=0; i< m_Vertices.m_numElements; ++i)
		{
			m_Vertices[i].m_Dist = max;
			m_Vertices[i].m_pShortestPathEdge = NULL;
		}

		pVertexA->m_Dist = 0.f;

		C_GraphVertex* pCurVertex = pVertexA;

		while(pCurVertex)
		{

			for(S32 i=0; i< pCurVertex->m_NumEdges; ++i)
			{
				C_GraphEdge* pEdge = pCurVertex->m_pGraphEdges[i];
				C_GraphVertex* pVertex = SharedPhysics::Fast_Select(pEdge->m_pVertexA == pCurVertex, pEdge->m_pVertexB, pEdge->m_pVertexA);
				if(!pVertex->m_Visited)
				{
					if(pVertex == pVertexB)
					{
						pCurVertex = NULL;
						pVertex->m_pShortestPathEdge = pEdge;
						break;
					}
					F32 dist = pCurVertex->m_Dist + 1.f;
					if(dist < pVertex->m_Dist)
					{
						pVertex->m_Dist = dist;
						pVertex->m_pShortestPathEdge = pEdge;
					}
				}
			}

			pCurVertex->m_Visited = true;
			F32 minDist = max;
			pCurVertex = NULL;

			for(S32 i=0; i< m_Vertices.m_numElements; ++i)
			{
				if(!m_Vertices[i].m_Visited && minDist < m_Vertices[i].m_Dist)
				{
					minDist = m_Vertices[i].m_Dist;
					pCurVertex = &m_Vertices[i];
				}
			}
		}
	}
}