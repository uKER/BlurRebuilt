#ifndef DELAUNAY_2D
#define DELAUNAY_2D


#ifdef _MSC_VER
#pragma once
#endif

#include <system/debug/I_DebugRender.h>

/*
	[SteveP] 14/08/2008 this has initially been written for the new animation state machine and node system,
	triangulating a collection of "cycle vertex" in 2D space.

	It's o.k. to use as an offline process (or on initialisation) because it's an iterative brute force method with no speed ups.
	This is not a problem for the animation system as it'll probably never have to triangulate more than 16 points, however 
	things get exponentially slower as the number of points increases.

	Triangle edges are cleaned up in the finish pass and vertex indices are swapped to give clockwise winding order.
	There are some cases where, if the triangle mesh is in a sliver type shape, that the algorithm can fail. Some vertices can be
	left out of the triangulation. 
	
	It's best to check the results and test for any triangle-less vertex.. 
*/

namespace Delaunay2d
{
	class C_CircumCircle
	{
	public:
		C_CircumCircle() {};
		C_CircumCircle( const Maths::HVec2& v0, const Maths::HVec2& v1, const Maths::HVec2& v2 );

		bool IsPointInside( const Maths::HVec2& point );

		Maths::HVec2	m_Centre;
		Maths::HF32		m_Radius2;
	};


	class C_Edge
	{
	public:
					C_Edge() { m_VertexIndex[0] = m_VertexIndex[1] = -1; }
					C_Edge( S16 vi0, S16 vi1 ) { m_VertexIndex[0] = vi0; m_VertexIndex[1] = vi1; }

		bool		IsValid() const { return m_VertexIndex[0] >= 0 && m_VertexIndex[1] >= 0 && m_VertexIndex[0] != m_VertexIndex[1]; }
		bool		IsEqual( const C_Edge& other ) const { return	(m_VertexIndex[0] == other.m_VertexIndex[0] && m_VertexIndex[1] == other.m_VertexIndex[1]) ||
																	(m_VertexIndex[0] == other.m_VertexIndex[1] && m_VertexIndex[1] == other.m_VertexIndex[0]); }

		S16			m_VertexIndex[2];
	};


	template< int E_MaxEdges >
	class C_EdgeList
	{
	public:
		C_EdgeList() : m_nEdges(0) { for(S32 i = 0 ; i < E_MaxEdges ; i++) m_EdgeCounts[i] = 0; }

		void	AddEdge( const C_Edge& edge )
				{ 
					for( S32 i = 0 ; i < m_nEdges ; i++)
					{
						if(m_Edges[i].IsEqual( edge ))
						{
							m_EdgeCounts[i]++;
							return;
						}
					}
					if(m_nEdges < E_MaxEdges)
					{
						m_Edges[m_nEdges] = edge;
						m_EdgeCounts[m_nEdges] = 1;
						m_nEdges++;
					}
				}

		S32		m_nEdges;
		S16		m_EdgeCounts[E_MaxEdges];
		C_Edge	m_Edges[E_MaxEdges];
	};


	class C_Triangle
	{
	public:
				C_Triangle()	{ Reset(); }

		void	Reset() { m_VertexIndex[0] = m_VertexIndex[1] = m_VertexIndex[2] = -1; m_pNext = 0; m_bValidCircumCircle = 0; m_AddedInPass = -1; }

		bool	IsValid() const 
				{ 
					return	m_VertexIndex[0] >= 0 && m_VertexIndex[1] >= 0 && m_VertexIndex[2] >= 0 && 
							m_VertexIndex[0] != m_VertexIndex[1] && m_VertexIndex[0] != m_VertexIndex[2] && m_VertexIndex[1] != m_VertexIndex[2];
				}

		bool	IsPointInside( S16 pointIndex, const Maths::HVec2* vertexList );

		bool	ValidCircumCircle() const { return m_bValidCircumCircle != 0; }
		void	InvalidateCircumCircle() { m_bValidCircumCircle = 0; }
		void	BuildCircumCircle( const Maths::HVec2& v0, const Maths::HVec2& v1, const Maths::HVec2& v2 ) 
											{ m_CircumCircle = C_CircumCircle( v0, v1, v2 ); m_bValidCircumCircle = 1; }

		S16				m_VertexIndex[3];
		S16				m_bValidCircumCircle;
		S16				m_AddedInPass;

		C_CircumCircle	m_CircumCircle;
		C_Triangle*		m_pNext;
	};



	class C_Triangulation
	{
	public:
		enum { E_MaxVertex = 4096, E_MaxTris = E_MaxVertex, E_MaxEdges = (E_MaxTris * 3) };

		C_Triangulation() { ResetAll(); }

		void			ResetAll();		//reset all
		void			ResetTris(); //reset triangle used and free lists only.

		S16				AddVertex( const Maths::HVec2& newVertex ); //check for duplicates, return added index or -1 for not.

		S16				GetTriIndex( C_Triangle* tri );

		bool			AddTri( S16 vi0, S16 vi1, S16 vi2 );
		bool			FreeTri( C_Triangle* tri, C_Triangle** listToFreeTo = 0 ); //by default free to "Free" list.. but can override and start a new list.

		void			Triangulate();//do it all.

		void			TriangulateBegin(); //initialise it.
		bool			TriangulateCanStep() const { return m_NextVertexToAdd < m_nVertex; }
		bool			TriangulateStep(); //return true if already done
		void			TriangulateFinish(); //get rid of super triangle triangles.


		void			DbgRender( DbUtil::I_DebugRenderPtr debugRender );

		S16				m_nVertex;
		S16				m_nTris;

		//bounds of vertex set
		Maths::HVec2	m_Min;
		Maths::HVec2	m_Max;

		C_Triangle*		m_pFirstFreeTri;
		C_Triangle*		m_pFirstUsedTri;


		S16				m_PassNum;
		S16				m_NextVertexToAdd;

		Maths::HVec2	m_VertexList[E_MaxVertex];
		C_Triangle		m_TriList[E_MaxTris];
	};


	class C_TriangleBarycentricCoord
	{
	public:
		C_TriangleBarycentricCoord( C_TriangleBarycentricCoord& other ) : m_Coefs(other.m_Coefs) {}
		C_TriangleBarycentricCoord( const Maths::HF32 coef0, const Maths::HF32 coef1, const Maths::HF32 coef2 ) : m_Coefs(coef0, coef1, coef2) {}
		C_TriangleBarycentricCoord( const Maths::HVec2& point, const Maths::HVec2& p0, const Maths::HVec2& p1, const Maths::HVec2& p2 ) { Build(point, p0, p1, p2); }

		void Build( const Maths::HVec2& point, const Maths::HVec2& p0, const Maths::HVec2& p1, const Maths::HVec2& p2 ); 
		bool IsInside() const;

		Maths::HVec3 m_Coefs;
	};


};



#endif
