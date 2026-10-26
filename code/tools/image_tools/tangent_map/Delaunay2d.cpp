#include "stdafx.h"
#include "Delaunay2d.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", off)
#endif


namespace Delaunay2d
{
	
C_CircumCircle::C_CircumCircle( const Maths::HVec2& v0, const Maths::HVec2& v1, const Maths::HVec2& v2 )
{
	const Maths::HVec2 v0_to_v1( v1 - v0 );
	const Maths::HVec2 v1_to_v2( v2 - v1 );
	
	const bool v0v1_colinY( Maths::Abs( v0_to_v1.Y() ) < Maths::HF32( 0.0001f ) );
	const bool v1v2_colinY( Maths::Abs( v1_to_v2.Y() ) < Maths::HF32( 0.0001f ) );

	//handle special cases for building the circle.. most common is hopefully last case
	if(v0v1_colinY)
	{
		if(v1v2_colinY)//all three colinear in Y
		{
			Maths::HVec2 p0(v0);
			Maths::HVec2 p1(v1);

			//ignore middle point from the circle edge
			if(v0_to_v1.X() > Maths::HF32(0.0f))
			{
				if(v1_to_v2.X() > Maths::HF32(0.0f))
					p1 = v2;
			}
			else
			{
				if(v2.X() < v0.X())
					p0 = v2;
			}

			m_Centre = (p0 + p1) * Maths::HF32(0.5f);
			m_Radius2 = Maths::DistanceSquared( m_Centre, p0 );
		}
		else //v0 and v1 are colinear in Y
		{
			Maths::HF32		ratio12( -v1_to_v2.X() / v1_to_v2.Y() );
			Maths::HVec2	average01( v0 + v0_to_v1 * Maths::HF32(0.5f) );
			Maths::HVec2	average12( v1 + v1_to_v2 * Maths::HF32(0.5f) );

			m_Centre.X( average01.X() );
			m_Centre.Y( ratio12 * (m_Centre.X() - average12.X()) + average12.Y() );
			m_Radius2 = Maths::DistanceSquared( m_Centre, v0 );
		}
	}
	else
	{
		if(v1v2_colinY) //v1 and v2 are colinear in Y
		{
			Maths::HF32		ratio01( -v0_to_v1.X() / v0_to_v1.Y() );
			Maths::HVec2	average01( v0 + v0_to_v1 * Maths::HF32(0.5f) );
			Maths::HVec2	average12( v1 + v1_to_v2 * Maths::HF32(0.5f) );

			m_Centre.X( average12.X() );
			m_Centre.Y( ratio01 * (m_Centre.X() - average01.X()) + average01.Y() );
			m_Radius2 = Maths::DistanceSquared( m_Centre, v0 );
		}
		else //all are happily not colinear in Y.
		{
			Maths::HF32		ratio01( -v0_to_v1.X() / v0_to_v1.Y() );
			Maths::HF32		ratio12( -v1_to_v2.X() / v1_to_v2.Y() );
			Maths::HVec2	average01( v0 + v0_to_v1 * Maths::HF32(0.5f) );
			Maths::HVec2	average12( v1 + v1_to_v2 * Maths::HF32(0.5f) );

			m_Centre.X( (ratio01 * average01.X() - ratio12 * average12.X() - average01.Y() + average12.Y()) / (ratio01 - ratio12) );
			m_Centre.Y( ratio01 * (m_Centre.X() - average01.X()) + average01.Y() );
			m_Radius2 = Maths::DistanceSquared( m_Centre, v0 );
		}
	}
}


bool C_CircumCircle::IsPointInside( const Maths::HVec2& point )
{
	const Maths::HF32 dist2 = Maths::DistanceSquared( point, m_Centre );

	if(dist2 < m_Radius2)// + Maths::HF32(0.0001f)))
		return true;

	return false;
}

bool C_Triangle::IsPointInside( S16 pointIndex, const Maths::HVec2* vertexList )
{
	for(S32 i = 0 ; i < 3 ; i++)
		if(m_VertexIndex[i] == pointIndex)
			return false;

	const Maths::HVec2 p0( vertexList[m_VertexIndex[0]] );
	const Maths::HVec2 p1( vertexList[m_VertexIndex[1]] );
	const Maths::HVec2 p2( vertexList[m_VertexIndex[2]] );

	if(!ValidCircumCircle())
		BuildCircumCircle( p0, p1, p2 );

	if(m_CircumCircle.IsPointInside( vertexList[pointIndex] ))
		return true;

	return false;
}


void C_Triangulation::ResetAll()
{
	m_nVertex = 0;
	m_Min = m_Max = Maths::HVec2( 0.0f, 0.0f );
	m_PassNum = 0;
	m_NextVertexToAdd = 0;

	ResetTris();
}

void C_Triangulation::ResetTris()
{
	m_nTris = 0;
	m_pFirstUsedTri = 0;

	C_Triangle* prevTri(0);
	C_Triangle* currTri( m_TriList );

	for(S32 i = 0 ; i < E_MaxTris ; i++)
	{
		currTri->Reset();
		currTri->m_pNext = prevTri;
		prevTri = currTri;
		currTri++;
	}

	m_pFirstFreeTri = prevTri;
}


S16	C_Triangulation::AddVertex( const Maths::HVec2& newVertex ) //check for duplicates, return added index or -1 for not.
{
	//first search for this vertex..
	for( S16 i = 0 ; i < m_nVertex ; i++ )
	{
		const Maths::HF32 dist2( Maths::DistanceSquared( newVertex, m_VertexList[i] ) );

		if(dist2 < Maths::HF32( 0.00001f ))
			return i;
	}

	if(m_nVertex < (E_MaxVertex - 3)) //need 3 extra vertex for big super-triangle
	{
		m_Min = Maths::HVec2( Maths::Min( m_Min.X(), newVertex.X() ), Maths::Min( m_Min.Y(), newVertex.Y() ) );
		m_Max = Maths::HVec2( Maths::Max( m_Max.X(), newVertex.X() ), Maths::Max( m_Max.Y(), newVertex.Y() ) );

		m_VertexList[m_nVertex] = newVertex;
		m_nVertex++;
		return m_nVertex - 1;
	}

	return -1;
}

S16 C_Triangulation::GetTriIndex( C_Triangle* tri )
{
	U32 triIndex = ((U32)tri - (U32)m_TriList) / (U32)sizeof(tri);

	if(triIndex < E_MaxTris)
		return (S16)triIndex;

	return -1;
}


bool C_Triangulation::AddTri( S16 vi0, S16 vi1, S16 vi2 )
{
	if( m_pFirstFreeTri && vi0 >= 0 && vi1 >= 0 && vi2 >= 0 && vi0 != vi1 && vi0 != vi2 && vi1 != vi2 )
	{
		m_nTris++;

		C_Triangle* newTri = m_pFirstFreeTri;
		m_pFirstFreeTri = newTri->m_pNext;

		newTri->Reset();
		newTri->m_pNext = m_pFirstUsedTri;
		m_pFirstUsedTri = newTri;

		newTri->m_VertexIndex[0] = vi0;
		newTri->m_VertexIndex[1] = vi1;
		newTri->m_VertexIndex[2] = vi2;
		newTri->m_AddedInPass = m_PassNum;

		return true;
	}
	return false;
}


bool C_Triangulation::FreeTri( C_Triangle* tri, C_Triangle** listToFreeTo )
{
	C_Triangle* prevTri( 0 );
	C_Triangle* currTri( m_pFirstUsedTri );

	if(!listToFreeTo)
		listToFreeTo = &m_pFirstFreeTri;

	while(currTri)
	{
		if(currTri == tri)
		{
			if(prevTri)
				prevTri->m_pNext = currTri->m_pNext;
			else
				m_pFirstUsedTri = currTri->m_pNext;

			currTri->m_pNext = *listToFreeTo;
			*listToFreeTo = currTri;

			m_nTris--;
			return true;
		}

		prevTri = currTri;
		currTri = currTri->m_pNext;
	}

	return false;
}

void C_Triangulation::Triangulate()
{
	TriangulateBegin();

	bool bDone(false);

	while(!bDone)
		bDone = TriangulateStep();

	TriangulateFinish();
}

void C_Triangulation::TriangulateBegin()
{
	assert( m_nVertex >= 3);

	const Maths::HF32 pad( 0.5f );
	const Maths::HF32 two( 2.0f );

	m_Min = Maths::HVec2( m_Min.X() - pad, m_Min.Y() - pad );
	m_Max = Maths::HVec2( m_Max.X() + pad, m_Max.Y() + pad );

	Maths::HVec2 bounds( m_Max - m_Min );

	//add a single super-triangle
	{
		m_PassNum = 0;
		m_NextVertexToAdd = 0;

		ResetTris();

		S16 vi0( AddVertex( m_Min ) );
		S16 vi1( AddVertex( m_Min + Maths::HVec2( bounds.X() * two, 0.0f ) ) );
		S16 vi2( AddVertex( m_Min + Maths::HVec2( 0.0f, bounds.Y() * two ) ) );

		m_nVertex -= 3; //last 3 vertex aren't counted.

		bool result = AddTri( vi0, vi1, vi2 );
		BIZ_ASSERT(result == true);
	}
}

bool C_Triangulation::TriangulateStep() //return true if already done
{
	if(m_NextVertexToAdd >= m_nVertex)
		return true;

	m_PassNum++;

	//add one vertex at a time, check every triangle and split as necessary
	U16			nTrisHit(0);
	C_Triangle* pFirstTriHit(0);

	C_Triangle* currTri( m_pFirstUsedTri );

	while(currTri)
	{
		C_Triangle* nextTri( currTri->m_pNext );

		if(currTri->IsPointInside( m_NextVertexToAdd, m_VertexList ))
		{
			if(FreeTri( currTri, &pFirstTriHit ))
				nTrisHit++;
		}
		currTri = nextTri;
	}

	//we now have a list of all collided triangles (to extract, split and re-add)
	//..pFirstTriHit this is a list of removed triangles from the used list..(but not yet added to the free list)
	// we'll need to reset these and add them manually to the free list when we're done.
	if(pFirstTriHit)
	{	//TODO generate new triangles here!
		C_EdgeList<E_MaxTris*2> edgeList;

		C_Triangle* currTri( pFirstTriHit );

		while(currTri) //for all collided tris, add edges to list.. if duplicate remove.
		{
			edgeList.AddEdge( C_Edge( currTri->m_VertexIndex[0], currTri->m_VertexIndex[1] ) );
			edgeList.AddEdge( C_Edge( currTri->m_VertexIndex[1], currTri->m_VertexIndex[2] ) );
			edgeList.AddEdge( C_Edge( currTri->m_VertexIndex[2], currTri->m_VertexIndex[0] ) );

			currTri = currTri->m_pNext;
		}

		//now discarding duplicates build new tris..
		for( S32 i = 0 ; i < edgeList.m_nEdges ; i++ )
		{
			if(edgeList.m_EdgeCounts[i] == 1)
			{
				AddTri( edgeList.m_Edges[i].m_VertexIndex[0], edgeList.m_Edges[i].m_VertexIndex[1], m_NextVertexToAdd );
			}
		}

	}
	
	//free temp triangles
	if(pFirstTriHit)
	{
		C_Triangle* prevTri( 0 );
		C_Triangle* currTri( pFirstTriHit );

		while(currTri)
		{
			prevTri = currTri;
			currTri = currTri->m_pNext;
		}

		if(prevTri)
			prevTri->m_pNext = m_pFirstFreeTri;

		m_pFirstFreeTri = pFirstTriHit;
		pFirstTriHit = 0;
	}

	//vertex added :D
	m_NextVertexToAdd++;

	return m_NextVertexToAdd >= m_nVertex; 
}

void C_Triangulation::TriangulateFinish()
{
	C_Triangle* currTri( m_pFirstUsedTri );

	while(currTri)
	{
		C_Triangle* nextTri = currTri->m_pNext;

		bool bHasSuperTriVertex(false);

		for(S32 i = 0 ; i < 3 && !bHasSuperTriVertex; i++)
		{
			if(currTri->m_VertexIndex[i] >= m_nVertex)
				bHasSuperTriVertex = true;
		}

		if(bHasSuperTriVertex)
			FreeTri( currTri );

		currTri = nextTri;
	}

	//now clean up the generated triangles by ordering vertex indices for a clockwise winding order.
	currTri = m_pFirstUsedTri;

	while(currTri)
	{
		const Maths::HF32 zero(0.0f);
		const Maths::HF32 half(0.5f);
		const Maths::HF32 third(1.0f/3.0f);

		const Maths::HVec2 triCentre( (m_VertexList[currTri->m_VertexIndex[0]] + m_VertexList[currTri->m_VertexIndex[1]] + m_VertexList[currTri->m_VertexIndex[2]]) * third );

		//flip first edge if needed
		{
			const Maths::HVec2 v0v1( m_VertexList[currTri->m_VertexIndex[1]] - m_VertexList[currTri->m_VertexIndex[0]] );
			const Maths::HVec2 v0v1_centre( m_VertexList[currTri->m_VertexIndex[0]] + v0v1 * half );
			const Maths::HVec2 v0v1_tangent( v0v1.Y(), -v0v1.X() );

			if( ((v0v1_centre - triCentre) | v0v1_tangent) < zero )
			{
				S16 temp = currTri->m_VertexIndex[0];
				currTri->m_VertexIndex[0] = currTri->m_VertexIndex[1];
				currTri->m_VertexIndex[1] = temp;
			}
		}

		{
			//flip second edge if needed
			const Maths::HVec2 v1v2( m_VertexList[currTri->m_VertexIndex[2]] - m_VertexList[currTri->m_VertexIndex[1]] );
			const Maths::HVec2 v1v2_centre( m_VertexList[currTri->m_VertexIndex[1]] + v1v2 * half );
			const Maths::HVec2 v1v2_tangent( v1v2.Y(), -v1v2.X() );

			if( ((v1v2_centre - triCentre) | v1v2_tangent) < zero )
			{
				S16 temp = currTri->m_VertexIndex[1];
				currTri->m_VertexIndex[1] = currTri->m_VertexIndex[2];
				currTri->m_VertexIndex[2] = temp;
			}
		}

		currTri = currTri->m_pNext;
	}
}


void C_Triangulation::DbgRender( DbUtil::I_DebugRenderPtr debugRender )
{
	const Maths::HVec2 centre( 512.0f, 150.0f );
	const Maths::HVec2 halfSize( 100.0f, 100.0f );

	debugRender->Draw2dLine( centre + Maths::HVec2( -halfSize.X(), -halfSize.Y() ), centre + Maths::HVec2( halfSize.X(), -halfSize.Y() ), Maths::RGBA( 32, 32, 32) );
	debugRender->Draw2dLine( centre + Maths::HVec2( halfSize.X(), -halfSize.Y() ), centre + Maths::HVec2( halfSize.X(), halfSize.Y() ), Maths::RGBA( 32, 32, 32) );
	debugRender->Draw2dLine( centre + Maths::HVec2( halfSize.X(), halfSize.Y() ), centre + Maths::HVec2( -halfSize.X(), halfSize.Y() ), Maths::RGBA( 32, 32, 32) );
	debugRender->Draw2dLine( centre + Maths::HVec2( -halfSize.X(), halfSize.Y() ), centre + Maths::HVec2( -halfSize.X(), -halfSize.Y() ), Maths::RGBA( 32, 32, 32) );

	S16 vertexToAdd(-1);

	for(S32 i = 0 ; i < m_nVertex ; i++)
	{
		Maths::HVec2 pos( m_VertexList[i] * halfSize + centre );

		if(vertexToAdd == m_NextVertexToAdd)
			debugRender->Draw2dCircle( pos, 2.0f, Maths::RGBA(0, 255, 255) );
		else
			debugRender->Draw2dCircle( pos, 2.0f, Maths::RGBA(255, 255, 255) );
	}

	C_Triangle* currTri( m_pFirstUsedTri );

	while(currTri)
	{
		assert( currTri->IsValid() );

		const S16 vi0( currTri->m_VertexIndex[0] );
		const S16 vi1( currTri->m_VertexIndex[1] );
		const S16 vi2( currTri->m_VertexIndex[2] );

		Maths::HVec2 p0( m_VertexList[vi0] * halfSize + centre );
		Maths::HVec2 p1( m_VertexList[vi1] * halfSize + centre );
		Maths::HVec2 p2( m_VertexList[vi2] * halfSize + centre );

		Maths::RGBA triCol;

		switch( currTri->m_AddedInPass & 0x3 )
		{
		case 1:
			triCol = Maths::RGBA( 255, 0, 0 );
			break;
		case 2:
			triCol = Maths::RGBA( 0, 255, 0 );
			break;
		case 3:
			triCol = Maths::RGBA( 0, 0, 255 );
			break;
		default:
			triCol = Maths::RGBA( 255, 255, 255 );
			break;
		}


		debugRender->Draw2dArrow( p0, p1-p0, 6.0f, triCol );//Draw2dLine( p0, p1, triCol );
		debugRender->Draw2dArrow( p1, p2-p1, 6.0f, triCol );//Draw2dLine( p1, p2, triCol );
		debugRender->Draw2dArrow( p2, p0-p2, 6.0f, triCol );//Draw2dLine( p2, p0, triCol );

		if(m_NextVertexToAdd < m_nVertex)
		{
			if(!currTri->ValidCircumCircle())
				currTri->BuildCircumCircle( m_VertexList[vi0], m_VertexList[vi1], m_VertexList[vi2] );

			Maths::HVec2 p3( currTri->m_CircumCircle.m_Centre * halfSize + centre );
			F32			 r3( Maths::Sqrt( currTri->m_CircumCircle.m_Radius2 ) * halfSize.X() );
			debugRender->Draw2dCircle( p3, r3, Maths::RGBA( 96, 96, 0 ) );
		}
		currTri = currTri->m_pNext;
	}

}

void C_TriangleBarycentricCoord::Build( const Maths::HVec2& point, const Maths::HVec2& p0, const Maths::HVec2& p1, const Maths::HVec2& p2 )
{
	const Maths::HVec2 v20( p0 - p2 );
	const Maths::HVec2 v21( p1 - p2 );
	const Maths::HVec2 v2p( p2 - point );

	m_Coefs.X( (v21.X() * v2p.Y() - v2p.X() * v21.Y()) / (v20.X() * v21.Y() - v21.X() * v20.Y()) );
	m_Coefs.Y( (v20.X() * v2p.Y() - v2p.X() * v20.Y()) / (v21.X() * v20.Y() - v20.X() * v21.Y()) );

	m_Coefs.Z( Maths::HF32(1.0f) - m_Coefs.X() - m_Coefs.Y() ); //assuming point in triangle?

//#ifndef CFG_SPEED
//	const Maths::HVec2 z_mul_p2( point - p0 * m_Coefs.X() - p1 * m_Coefs.Y() );
//	const Maths::HF32 z( (z_mul_p2.X() * z_mul_p2.X()) > (z_mul_p2.Y() * z_mul_p2.Y()) ? z_mul_p2.X() / p2.X() : z_mul_p2.Y() / p2.Y() );
//
//	assert( Maths::Abs( m_Coefs.Z() - z ) < 0.0001f );
//#endif
}

bool C_TriangleBarycentricCoord::IsInside() const
{
	const Maths::HF32 zero(-0.0001f);
	const Maths::HF32 one(1.0001f);

	return	m_Coefs.X() > zero && m_Coefs.X() < one && 
			m_Coefs.Y() > zero && m_Coefs.Y() < one && 
			m_Coefs.Z() > zero && m_Coefs.Z() < one;
}




}; //namespace Delaunay2d

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", on)
#endif


