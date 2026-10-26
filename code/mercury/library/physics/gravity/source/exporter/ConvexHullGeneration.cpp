#include "stdafx.h"
#include "gravity/exporter/convexhullgeneration.h"
#include "gravity/narrowphase/Primitive.h"

#include <core/utility/endian.h>
#include <System/Serialise/I_FileSystem.h>
#include <system/serialise/BinarySerialise.h>

//#define GRAVITY_WRITE_CONVEX_HULL_POINTS

namespace Gravity
{


	U32 C_Edge::getSource() const 
	{
		BIZ_ASSERT(m_index < 3);
		return (*m_facet)[m_index];
	}

	U32 C_Edge::getTarget() const 
	{
		BIZ_ASSERT(m_index < 3);
		return (*m_facet)[incMod3(m_index)];
	}
	  

	//bool C_Facet::link(U32 edge0, C_Facet* facet, U32 edge1) 
	//{
	//	m_adjFacets[edge0] = facet;
	//	m_adjEdges[edge0] = edge1;
	//	facet->m_adjFacets[edge1] = this;
	//	facet->m_adjEdges[edge1] = edge0;

	//	BIZ_ASSERT((m_indice[edge0] == facet->m_indice[incMod3(edge1)]) && (m_indice[incMod3(edge0)] == facet->m_indice[edge1]));
	//	//insure the index of the end points of the edge is the same
	//	return (m_indice[edge0] == facet->m_indice[incMod3(edge1)] &&
	//	m_indice[incMod3(edge0)] == facet->m_indice[edge1]);
	//}

	void C_Facet::link(U32 edge0, C_Facet* facet, U32 edge1) 
	{
		m_adjFacets[edge0] = facet;
		m_adjEdges[edge0] = edge1;
		facet->m_adjFacets[edge1] = this;
		facet->m_adjEdges[edge1] = edge0;

		//BIZ_ASSERT((m_indice[edge0] == facet->m_indice[incMod3(edge1)]) && (m_indice[incMod3(edge0)] == facet->m_indice[edge1]));
	}

	void C_Facet::silhouette(const U32 _index, const Maths::HVec3 PCREF w, C_EdgeBuffer& edgeBuffer, C_Edge* const __restrict stack) 
	{
		using namespace Maths;

		S32 size = 1;
		stack[0].m_facet = this;
		stack[0].m_index = _index;

		C_Facet* next_facet = this;
		S32    next_index = _index;
		C_Facet* next2_facet = NULL;
		S32    next2_index = -1;

//		const HF32 zero(Constants<HF32>::Zero());
		//const HF32 epsilon(-Constants<HF32>::Epsilon());
		const HF32 epsilon(-0.0001f);

		while(size-- > 0)
		{
			/*const Edge& e = stack[size];
			Facet* const __restrict f = e.m_facet;
			const U32 index(e.m_index);*/

			C_Facet* const __restrict f = next_facet;
			const U32 index( next_index );

			if(f->m_Discarded)
			{
				
				next_facet = next2_facet;
				next_index = next2_index;
				const C_Edge& next2_e = stack[ Max(size-2,0) ];
				next2_facet = next2_e.m_facet;
				next2_index = next2_e.m_index;
				
				continue;
			}
			
			const HF32 signDist((f->m_Normal|w) - f->m_Dist);
			
			if (signDist.LT(epsilon)) //the facet is not visible from w
			{
				//const Edge& e = stack[size];
				//edgeBuffer.Insert(e); // construct the edge and add it into the edge buffer
				//edgeBuffer.Insert(next_facet, next_index);
				edgeBuffer.Insert(f, index);
				if(size!=0)
				{
					next_facet = next2_facet;
					next_index = next2_index;
					const C_Edge& next2_e = stack[ Max(size-2,0) ];
					next2_facet = next2_e.m_facet;
					next2_index = next2_e.m_index;
				}
			}	
			else 
			{
				f->m_Discarded= true; // Facet is visible from w
				const U32 next(incMod3(index));
				const U32 next2(incMod3(next));
				stack[size].m_facet   = f->m_adjFacets[next2];
				stack[size++].m_index = f->m_adjEdges[next2];
				stack[size].m_facet   = f->m_adjFacets[next];
				stack[size++].m_index = f->m_adjEdges[next];
				next_facet = f->m_adjFacets[next];
				next_index = f->m_adjEdges[next];
				next2_facet = f->m_adjFacets[next2];
				next2_index = f->m_adjEdges[next2];
			}
		}
	}

	void C_Facet::silhouette(const Maths::HVec3 PCREF w, C_EdgeBuffer& edgeBuffer, C_Edge* const __restrict stack)
	{
		//edgeBuffer.MakeEmpty();
		m_Discarded = true;
		m_adjFacets[0]->silhouette(m_adjEdges[0], w, edgeBuffer, stack);
		m_adjFacets[1]->silhouette(m_adjEdges[1], w, edgeBuffer, stack);
		m_adjFacets[2]->silhouette(m_adjEdges[2], w, edgeBuffer, stack);
		
	}


	/*void C_Point::SetFacetsVisited()
	{
		for(S32 i=0; i<m_Facets.m_numElements; ++i)
		{
			m_Facets[i].m_pFacet->m_Visited = true;
		}
	}

	void C_Point::ClearFacetsVisited()
	{
		for(S32 i=0; i<m_Facets.m_numElements; ++i)
		{
			m_Facets[i].m_pFacet->m_Visited = false;
		}
	}*/


	GenerationState C_ConvexHullMesh::TetrahedronGeneration(/*const SharedPhysics::SortedArray<Maths::HVec3>& points*/)
	{
		BIZ_ASSERT(m_SupportPoints.m_numElements >= 4);
		using namespace Maths;
		HF32 positive(-Constants<HF32>::Max());
		HF32 negative(Constants<HF32>::Max());
		const HF32 zero(Constants<HF32>::Zero());

		U32 indexA=0, indexB=0, indexC=0, indexD=0;
		S32 size = m_SupportPoints.m_numElements;

		for(S32 j =0; j<3; ++j)
		{
			for(S32 i=0; i<size; ++i)
			{
				const HVec3 p = m_SupportPoints[i].m_Point;
				const HF32 axis(p[j]);
				if(positive.LT(axis))
				{
					positive = axis;
					indexA = i;
				}

				if(negative.GT(axis))
				{
					negative = axis;
					indexB = i;
				}
			}

			if(indexA != indexB)
				break;
		}

		if(indexA == indexB)
		{
			indexB = indexA +1;
		}

		HVec3 a(m_SupportPoints[indexA].m_Point);
		HVec3 b(m_SupportPoints[indexB].m_Point);

		//m_SupportPoints.Remove(indexA, 1);
		//m_SupportPoints.Remove(indexB, 1);

		////BIZ_ASSERT(indexA != indexB);

		//size -=2;
		
		const C_Normal ab(b-a);
		HF32 dist(zero);
		for(S32 i=0; i<size; ++i)
		{
			const HVec3 v(m_SupportPoints[i].m_Point - a);
			const HF32 vCos(ab|v);
			const HF32 sVSin((v|v) - (vCos*vCos));
			if(sVSin.GT(dist))
			{
				dist = sVSin;
				indexC = i;
			}
		}

		if(indexA == indexC || indexB == indexC)
		{
			indexC = indexB + 1;
		}

		//BIZ_ASSERT(indexA != indexC && indexB != indexC);

		HVec3 c(m_SupportPoints[indexC].m_Point);
		const HVec3 ac(c - a);
		const C_Normal normal(ac ^ ab);
		const HF32 planeDist(normal|a);

		dist = zero;
		for(S32 i=0; i<size; ++i)
		{
			const HF32 signDist((normal|m_SupportPoints[i].m_Point) - planeDist);
			const HF32 absDist(Abs(signDist));
			if(absDist.GT(dist))
			{
				dist =absDist;
				indexD = i;
			}
		}

		if(indexA == indexD || indexB == indexD || indexC == indexD)
		{
			indexD = indexC + 1;
		}
		//BIZ_ASSERT(indexA != indexD && indexB != indexD && indexC != indexD);
		HVec3 d(m_SupportPoints[indexD].m_Point);

		const HVec3 ad(d-a);
		const HF32 volume = Abs(ad|normal);
		if(volume.LT(Maths::Constants<HF32>::Epsilon()))
		{
			return E_Generation_Fail;
		}
		else
		{

			m_Points.AddToEnd(&indexA);
			m_Points.AddToEnd(&indexB);
			m_Points.AddToEnd(&indexC);
			m_Points.AddToEnd(&indexD);

			m_SupportPoints[indexA].deleted = true;
			m_SupportPoints[indexB].deleted = true;
			m_SupportPoints[indexC].deleted = true;
			m_SupportPoints[indexD].deleted = true;
			return E_Generating;
		}

		//SanityCheck();
		
	}





	void C_ConvexHullMesh::SanityCheck()
	{
		const Maths::HF32 eps(0.0001f);
		for(S32 i=0; i<m_SupportPoints.m_numElements; ++i)
		{
			if(!m_SupportPoints[i].deleted)
			{
				for(S32 j=0; j<m_Points.m_numElements; ++j)
				{
					const Maths::HF32 dist=((m_SupportPoints[m_Points[j]].m_Point - m_SupportPoints[i].m_Point).Length());
					BIZ_ASSERT(dist > eps);
				}
			}
		}
	}

	void C_ConvexHullMesh::SanityCheckSupport()
	{
		const Maths::HF32 eps(0.0001f);
		for(S32 i=0; i<m_SupportPoints.m_numElements; ++i)
		{
			for(S32 j=i+1; j<m_SupportPoints.m_numElements; ++j)
			{
				const Maths::HF32 dist=((m_SupportPoints[j].m_Point - m_SupportPoints[i].m_Point).Length());
				BIZ_ASSERT(dist > eps);
			}
			
		}
	}

	Collision::C_AABBMinMax C_ConvexHullMesh::ConstructC_AABBMinMax()
	{
		using namespace Maths;
		Collision::C_AABBMinMax aabb;
		aabb.m_min = Maths::Constants<HVec3>::Max();
		aabb.m_max =-aabb.m_min;
		for(S32 i = 0; i<m_Points.m_numElements; ++i)
		{
			const HVec3 point = m_SupportPoints[m_Points[i]].m_Point;
			aabb.m_min = Min(aabb.m_min, point);
			aabb.m_max = Max(aabb.m_max, point);
		}


		return aabb;

	}

	void C_ConvexHullMesh::DebugRender(DbUtil::I_DebugRender* pRender, Maths::RGBA PCREF color, bool wireframe)
	{
		using namespace Maths;

		for(S32 i = 0; i<m_Points.m_numElements; ++i)
		{
			pRender->Draw3dStar(m_SupportPoints[m_Points[i]].m_Point, 0.05f, RGBA(255, 255, 255));
		}

		//After calling remapfacets, we don't need to check whether the facet is discarded anymore
		for(S32 i=0; i<m_Facets.m_numElements; ++i)
		{
			Gravity::C_Facet* facet = m_Facets[i];
			//if(!facet->m_Discarded)
			{
				const HVec3 a(m_SupportPoints[m_Points[facet->m_indice[0]]].m_Point);
				const HVec3 b(m_SupportPoints[m_Points[facet->m_indice[1]]].m_Point);
				const HVec3 c(m_SupportPoints[m_Points[facet->m_indice[2]]].m_Point);
				
				if(!wireframe)
					pRender->Draw3dTriangle(a, c, b, color);
				pRender->Draw3dTriangleOutline(a, b, c, RGBA(color.R()+ 50, color.G() + 50 , color.B()+50));
			}
		}
	}

	U32 C_ConvexHullMesh::NumValidFacets()
	{
		U32 count =0;
		for(S32 i=0; i<m_Facets.m_numElements; ++i)
		{
			Gravity::C_Facet* facet = m_Facets[i];
			if(!facet->m_Discarded)
				count++;
		}
		return count;
	}

	void C_ConvexHullMesh::RemapFacets()
	{
		for(S32 i =0; i< m_Facets.m_numElements; ++i)
		{
			if(m_Facets[i]->m_Discarded)
			{
				m_Facets.QuickRemove(i);
				--i;
			}
		}
		for(S32 i =0; i< m_Facets.m_numElements; ++i)
		{
			m_Facets[i]->m_Index = i;
		}
	}

	void C_ConvexHullMesh::ClearFacetsVisitedFlags()
	{
		for(S32 i =0; i< m_Facets.m_numElements; ++i)
		{
			m_Facets[i]->m_Visited = false;
		}
	}


	GenerationState C_ConvexHullMesh::Build(const SharedPhysics::SortedArray<Maths::HVec3>& point)
	{
		GenerationState state = Initialise(point);

		while(state == E_Generating)
		{
			state = StepBuild();
		}

		return state;
	}


	GenerationState C_ConvexHullMesh::Initialise(const SharedPhysics::SortedArray<Maths::HVec3>& point)
	{
		/*for(S32 a = 0; a < m_Facets.m_numElements; ++a)
		{
			delete m_Facets[a];
		}*/
		const Maths::HF32 epsilon(0.0001f);
		m_Iterator.Reset();
		m_Facets.m_numElements = 0;
		m_SupportPoints.m_numElements = 0;
		m_Points.m_numElements =0;
		//m_Edges.m_numElements = 0;
		using namespace Maths;
		const S32 size = point.m_numElements;

		for(S32 i=0; i<size; ++i)
		{
			bool found = false;
			for(S32 j = 0; j < m_SupportPoints.m_numElements; ++j)
			{
				const Maths::HVec3 dif(point[i] - m_SupportPoints[j].m_Point);
				//const Maths::HF32 dis(dif | dif);
				if(dif.Length() < epsilon)
				{
					found = true;
				}
			}
			if(!found)
			{
				C_Point p(point[i]);
				p.m_IndexInOriginalModel = i;
				m_SupportPoints.AddToEnd(&p);
			}
		}

#ifdef GRAVITY_WRITE_CONVEX_HULL_POINTS
		{
			static U32 count =0;
			Btl::String filename;
			filename = Btl::String::Format("z:\\ConvexHullPoints%i.dat", count++);
			Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(filename.Chars(), Sys::E_FileAccess_Write );

			if(pFile)
			{
				pFile->SetEndianSwapFlag( true );
				{
					Sys::BinarySerialiser<Ut::DoEndianSwap> ar(*pFile);

					S32 sourceVertexCount = m_SupportPoints.m_numElements;

					ar << sourceVertexCount;

					for(S32 i=0; i<sourceVertexCount; i++)
					{
						F32 x = m_SupportPoints[i].m_Point.X();
						F32 y = m_SupportPoints[i].m_Point.Y();
						F32 z = m_SupportPoints[i].m_Point.Z();
						ar << x;
						ar << y;
						ar << z;
					}
				}

				SAFE_DELETE_PTR(pFile);
			}		
		}
#endif


		SanityCheckSupport();

		if(TetrahedronGeneration() == E_Generation_Fail)
			return E_Generation_Fail;



		//assume the tetrahedron is valid, add facet and edges
		/*C_Facet* f0 = new C_Facet(0, 1, 2);
		C_Facet* f1 = new C_Facet(0, 3, 1);
		C_Facet* f2 = new C_Facet(0, 2, 3);
		C_Facet* f3 = new C_Facet(1, 3, 2);*/

		C_Facet* f0 = m_Iterator.Create();
		new (f0) C_Facet(0, 1, 2);
		C_Facet* f1 = m_Iterator.Create();
		new (f1) C_Facet(0, 3, 1);
		C_Facet* f2 = m_Iterator.Create();
		new (f2) C_Facet(0, 2, 3);
		C_Facet* f3 = m_Iterator.Create();
		new (f3)C_Facet(1, 3, 2);


		const HF32 zero(Constants<HF32>::Zero());
		const HVec3 a(m_SupportPoints[m_Points[0]].m_Point);
		const HVec3 b(m_SupportPoints[m_Points[1]].m_Point);
		const HVec3 c(m_SupportPoints[m_Points[2]].m_Point);
		const HVec3 d(m_SupportPoints[m_Points[3]].m_Point);

		const HVec3 ab(b-a);
		const HVec3 ac(c-a);
		
		const C_Normal norm(ac^ab);
		HF32 signDist(norm|(d - a));

		if(signDist > zero)
		{
			//flip the vertex index
			const U32 index = m_Points[1];
			m_Points[1] = m_Points[2];
			m_Points[2] = index; 
		}
		else
		{
			signDist = - signDist;
		}

		f0->Initialise(m_Points, m_SupportPoints);
		f1->Initialise(m_Points, m_SupportPoints);
		f2->Initialise(m_Points, m_SupportPoints);
		f3->Initialise(m_Points, m_SupportPoints);
		
		f0->link(0, f1, 2);
		f0->link(1, f3, 2);
		f0->link(2, f2, 0);
		f1->link(0, f2, 2);
		f1->link(1, f3, 0);
		f2->link(1, f3, 1);  

		m_Facets.AddToEnd(&f0);
		m_Facets.AddToEnd(&f1);
		m_Facets.AddToEnd(&f2);
		m_Facets.AddToEnd(&f3);

		return E_Generating;

		//SanityCheck();

	}

	void C_ConvexHullMesh::Clear()
	{
		for(S32 i=0; i<m_Facets.m_numElements; ++i)
		{
			m_Facets[i]->m_FurthestPointDist = -Maths::Constants<Maths::HF32>::Max();
			m_Facets[i]->m_OutsideSet.m_numElements = 0;
		}

		for(S32 i=0; i<m_SupportPoints.m_numElements; ++i)
		{
			if(!m_SupportPoints[i].deleted && !m_SupportPoints[i].allocatedOutsidePoint)
				m_SupportPoints[i].deleted = true;

			m_SupportPoints[i].allocatedOutsidePoint = false;
		}
		
		//for(S32 i=0; i<m_SupportPoints.m_numElements; ++i)
		//{
		//	//if(!m_SupportPoints[i].deleted)
		//	m_SupportPoints[i].allocatedOutsidePoint = false;
		//}

		
		for(S32 i=0; i<m_Points.m_numElements; ++i)
		{
			bool discard = true;
			//U32 index = m_Points[i];
			for(S32 j=0; j<m_Facets.m_numElements; ++j)
			{
				if(!m_Facets[j]->m_Discarded)
				{
					if(i  == m_Facets[j]->m_indice[0] || i == m_Facets[j]->m_indice[1] || i == m_Facets[j]->m_indice[2])
					{
						discard = false;
						break;
					}
				}
			}
			if(discard)
			{
				m_SupportPoints[m_Points[i]].deleted = false;
			}
		}

		//SanityCheck();
		
	}

	GenerationState C_ConvexHullMesh::StepBuild()
	{
	/*	if(m_SupportPoints.m_numElements == 0)
		{
			return true;
		}*/

		/*SanityCheck();
		SanityCheckSupport();*/

		using namespace Maths;
		
		Maths::HF32 signDist;
		for(S32 j=0; j < m_Facets.m_numElements; ++j)
		{
			
			if(!m_Facets[j]->m_Discarded)
			{
				
				for(S32 i =0; i <m_SupportPoints.m_numElements; ++i)
				{
					if((!m_SupportPoints[i].deleted) && (!m_SupportPoints[i].allocatedOutsidePoint) && m_Facets[j]->IsAbove(m_SupportPoints[i].m_Point, signDist))
					{
						m_Facets[j]->AddOutsideSet(&m_SupportPoints[i], i,  signDist);
						m_SupportPoints[i].allocatedOutsidePoint=true;
					}
				}
			}
		}

		C_Facet* curFacet;
		U32 count=0;
		do
		{
			if(count == m_Facets.m_numElements)
				return E_Generation_Succeed;
			if(!m_Facets[count]->m_Discarded && m_Facets[count]->NumOutsideSet() > 0)
			{
				curFacet = m_Facets[count];
				break;
			}
			count++;
		}while(1);

		//SanityCheck();


		HVec3 p = curFacet->m_FurthestPoint.m_OutsidePoint.m_Point;
	

		//If the furthestPoint already in m_Points, don't add to the m_points 
		S32 numVerts = -1;
		for(S32 i=0; i<m_Points.m_numElements; ++i)
		{
			if(m_Points[i] == curFacet->m_FurthestPoint.m_Index)
			{
				numVerts = i;
				break;
			}
		}
		if(numVerts < 0)
		{
			numVerts = m_Points.m_numElements;
			m_Points.AddToEnd(&curFacet->m_FurthestPoint.m_Index);
		}
		m_SupportPoints[curFacet->m_FurthestPoint.m_Index].deleted = true;
		
		//SanityCheck();
	
		edgeBuffer.MakeEmpty();
		curFacet->silhouette(p, edgeBuffer, stack);

		//BIZ_ASSERT(edgeBuffer.Size() != 0); 
		if(edgeBuffer.Size() == 0) // the model isn't convex at all
			return E_Generation_Fail;	

		C_Edge* edge=edgeBuffer.Get(0);

		C_Facet* firstFacet = m_Iterator.Create();
		new (firstFacet) C_Facet(edge->getTarget(), edge->getSource(), numVerts);
		firstFacet->Initialise(m_Points, m_SupportPoints);
		m_Facets.AddToEnd(&firstFacet);

		firstFacet->link(0, edge->getFacet(), edge->getIndex());
				
	
		U32 size=edgeBuffer.Size();
		C_Facet *lastFacet = firstFacet;
	
		for(U32 i=1; i<size; ++i)
		{
			edge=edgeBuffer.Get(i);
			C_Facet* newFacet = m_Iterator.Create();
			new (newFacet) C_Facet(edge->getTarget(), edge->getSource(), numVerts);
			newFacet->Initialise(m_Points, m_SupportPoints);
			m_Facets.AddToEnd(&newFacet);
			newFacet->link(0, edge->getFacet(), edge->getIndex());
			newFacet->link(2, lastFacet, 1);
			lastFacet = newFacet;
		}

		firstFacet->link(2, lastFacet, 1);
		Clear();
	
		return  E_Generating;
	}

}
