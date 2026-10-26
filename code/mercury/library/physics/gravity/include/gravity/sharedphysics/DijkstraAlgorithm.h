#ifndef	DIJKSTRA_ALGORITHM_H
#define DIJKSTRA_ALGORITHM_H

#include "gravity/sharedphysics/sortedarray.h"


namespace SharedPhysics
{
	class C_GraphVertex;
	

	class C_GraphEdge
	{
	public:
		C_GraphEdge(): m_pVertexA(NULL), m_pVertexB(NULL), m_pNext(NULL)
		{
		}

		C_GraphVertex* m_pVertexA, *m_pVertexB;
		C_GraphEdge* m_pNext;//the next edge contected in the shortest path finding
		
	};

	class C_GraphVertex
	{
	public:
		C_GraphVertex(): m_Visited(false), m_NumEdges(0), m_Dist(0.f)
		{
		}

		Maths::HVec3 m_Position;
		C_GraphEdge** m_pGraphEdges;
		C_GraphEdge* m_pShortestPathEdge;
		bool m_Visited;
		U32 m_NumEdges;
		F32 m_Dist;
	};


	class C_DijkstraShortestPath
	{
	public:
		C_DijkstraShortestPath()
		{
		}

		~C_DijkstraShortestPath()
		{
		}

		void GetShorestPath(C_GraphVertex* vertexA, C_GraphVertex* vertexB);

		SharedPhysics::SortedArray<C_GraphVertex> m_Vertices;
		SharedPhysics::SortedArray<C_GraphEdge> m_Edges;
	};
}

#endif