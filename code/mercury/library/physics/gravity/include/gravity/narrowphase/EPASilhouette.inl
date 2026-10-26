#ifndef EPA_SILHOUETTE_INL
#define EPA_SILHOUETTE_INL

#include "gravity/sharedphysics/Select.h"
//#include "gravity/narrowphase/PrimitiveTest.h"

namespace Collision
{
	__forceinline U32 Edge::getSource() const 
	{
		BIZ_ASSERT(m_index < 3);
		return (*m_facet)[m_index];
	}

	__forceinline U32 Edge::getTarget() const 
	{
		BIZ_ASSERT(m_index < 3);
		return (*m_facet)[incMod3(m_index)];
	}

	__forceinline Facet2* EPASilhouette::addFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2)
	{

		/*using namespace Maths;
		BIZ_ASSERT(i0 != i1 && i0 != i2 && i1 != i2);
		if (freeFacet < MaxFacets)
		{
			Facet2 *facet = new(&facetBuf[freeFacet])Facet2(i0, i1, i2);

			if(facet->isValid(i0, i1, i2, aBuf, bBuf) && lower2.LE(facet->m_dist) && facet->m_dist.LE(upper2))
			{
				heap.insert(*facet);
				++num_facets;
				++freeFacet;
				return facet;
			}
		}
		return 0;*/


		using namespace Maths;

		BIZ_ASSERT(i0 != i1 && i0 != i2 && i1 != i2);
		if (freeFacet < MaxFacets)
		{
			Facet2 *facet = new(&facetBuf[freeFacet])Facet2(i0, i1, i2);

			S32 b1;
			Scalar_Bool b2(Maths::UNDEFINED);
			facet->isValid(i0, i1, i2, aBuf, bBuf,b1, b2);
			if(b2 & lower2.LE(facet->m_dist) & facet->m_dist.LE(upper2))
			{
				heap.insert(facet);
				++num_facets;
			}
			freeFacet+=b1;
			return SharedPhysics::Fast_Select<Facet2>(b1 != 0, facet, (Facet2*)0);
		}
	    
		return 0;
		
	}

	__forceinline Facet2* EPASilhouette::addInitialFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2)
	{
		using namespace Maths;

		BIZ_ASSERT(i0 != i1 && i0 != i2 && i1 != i2);
		//if (freeFacet < MaxFacets)
		{
			Facet2 *facet = new(&facetBuf[freeFacet])Facet2(i0, i1, i2);
			S32 b1;
			Scalar_Bool b2(Maths::UNDEFINED);
			facet->isValid(i0, i1, i2, aBuf, bBuf, b1, b2);
			heap.insert(facet);
			++num_facets;
			freeFacet+=b1;

			return facet;
		}
	    
		//return 0;
	}

	inline void Facet2::isValid(const U32 i0, const U32 i1, const U32 i2, const Maths::HVec3* __restrict a_verts, const Maths::HVec3* __restrict  b_verts, S32& b1, Maths::Scalar_Bool& b2)
	{
		using namespace Maths;

		const HF32 zero(-Constants<Maths::HF32>::Epsilon());
		const HVec3 p0(a_verts[i0]-b_verts[i0]);
		const HVec3 p1(a_verts[i1]-b_verts[i1]);
		const HVec3 p2(a_verts[i2]-b_verts[i2]);
		const HVec3 v1(p1 - p0);
		const HVec3 v2(p2 - p0);

		const HF32 v1dv1 = v1 | v1;
		const HF32 v1dv2 = v1 | v2;
		const HF32 v2dv2 = v2 | v2;
		const HF32 p0dv1 = p0 | v1; 
		const HF32 p0dv2 = p0 | v2;
	    
		const HF32 det( v1dv1 * v2dv2 - v1dv2 * v1dv2 ); // non-negative
		const HF32 recip(det.Reciprocal());
		const HF32 lambda1(p0dv2 * v1dv2 - p0dv1 * v2dv2);
		const HF32 lambda2(p0dv1 * v1dv2 - p0dv2 * v1dv1);
		const HVec3 closest(p0 + (lambda1 * v1 + lambda2 * v2) * recip);

		m_closest=closest;
		m_dir = closest;
		//sDist=closest | closest;
		m_dist = closest|closest;
		
		b1= det >= zero; //NB - this will be a load hit store!!!
		b2=((lambda1.GT(zero)) & (lambda2.GT(zero)) & ((lambda1+lambda2).LE(det)));
	}

	//inline void Facet2::silhouette(const U32 _index, const Maths::HVec3 PCREF w, EdgeBuffer& edgeBuffer, Edge* const __restrict stack) 
	//{
	//	using namespace Maths;

	//	S32 size = 1;
	//	stack[0].m_facet = this;
	//	stack[0].m_index = _index;

	//	Facet2* next_facet = this;
	//	S32    next_index = _index;
	//	Facet2* next2_facet = NULL;
	//	S32    next2_index = -1;

	//	while(size-- > 0)
	//	{
	//		/*const Edge& e = stack[size];
	//		Facet* const __restrict f = e.m_facet;
	//		const U32 index(e.m_index);*/

	//		Facet2* const __restrict f = next_facet;
	//		const U32 index( next_index );

	//		if(f->m_obsolete)
	//		{
	//			
	//			next_facet = next2_facet;
	//			next_index = next2_index;
	//			const Edge& next2_e = stack[ Max(size-2,0) ];
	//			next2_facet = next2_e.m_facet;
	//			next2_index = next2_e.m_index;
	//			
	//			continue;
	//		}
	//		
	//	//	const HF32 dist(f->m_closest|w);
	//		/*const C_Normal n(f->m_closest);
	//		const HF32 dist(n|w);
	//		const HF32 sqDist(dist*dist);
	//		if(sqDist < f->m_dist)*/
	//		if ((f->m_closest | w) < f->m_dist) //the facet is not visible from w
	//		{
	//			//const Edge& e = stack[size];
	//			//edgeBuffer.Insert(e); // construct the edge and add it into the edge buffer
	//			//edgeBuffer.Insert(next_facet, next_index);
	//			edgeBuffer.Insert(f, index);
	//			if(size!=0)
	//			{
	//				next_facet = next2_facet;
	//				next_index = next2_index;
	//				const Edge& next2_e = stack[ Max(size-2,0) ];
	//				next2_facet = next2_e.m_facet;
	//				next2_index = next2_e.m_index;
	//			}
	//		}	
	//		else 
	//		{
	//			f->m_obsolete = true; // Facet is visible from w
	//			const U32 next(incMod3(index));
	//			const U32 next2(incMod3(next));
	//			stack[size].m_facet   = f->m_adjFacets[next2];
	//			stack[size++].m_index = f->m_adjEdges[next2];
	//			stack[size].m_facet   = f->m_adjFacets[next];
	//			stack[size++].m_index = f->m_adjEdges[next];
	//			next_facet = f->m_adjFacets[next];
	//			next_index = f->m_adjEdges[next];
	//			next2_facet = f->m_adjFacets[next2];
	//			next2_index = f->m_adjEdges[next2];
	//		}
	//	}
	//}

	inline void Facet2::silhouette(const U32 _index, const Maths::HVec3 PCREF w, EdgeBuffer& edgeBuffer, Edge* const __restrict stack) 
	{
		using namespace Maths;

		S32 size = 1;

		Facet2* next_facet = this;
		S32    next_index = _index;

		while(size--)
		{
			Facet2* const __restrict f = next_facet;
			const U32 index( next_index );
			if(f->m_obsolete)
			{				
				const Edge& next_e = stack[ Max(size-1,0) ];
				next_facet = next_e.m_facet;;
				next_index = next_e.m_index;
			}
			else
			{
				if ((f->m_closest | w) < f->m_dist) //the facet is not visible from w
				{
					edgeBuffer.Insert(f, index);
					const Edge& next_e = stack[ Max(size-1,0) ];
					next_facet = next_e.m_facet;
					next_index = next_e.m_index;
				}	
				else 
				{
					f->m_obsolete = true; // Facet is visible from w
					const U32 next(incMod3(index));
					const U32 next2(incMod3(next));
					stack[size].m_facet = f->m_adjFacets[next2];
					stack[size].m_index = f->m_adjEdges[next2];
					size += 2;
					next_facet = f->m_adjFacets[next];
					next_index = f->m_adjEdges[next];
				}
			}
		}
	}

	__forceinline void Facet2::silhouette(const Maths::HVec3 PCREF w, EdgeBuffer& edgeBuffer, Edge* const __restrict stack)
	{
		//edgeBuffer.MakeEmpty();
		m_obsolete = true;
		m_adjFacets[0]->silhouette(m_adjEdges[0], w, edgeBuffer, stack);
		m_adjFacets[1]->silhouette(m_adjEdges[1], w, edgeBuffer, stack);
		m_adjFacets[2]->silhouette(m_adjEdges[2], w, edgeBuffer, stack);
		
	}

	__forceinline bool Facet2::link(U32 edge0, Facet2 *facet, U32 edge1) 
	{
		m_adjFacets[edge0] = facet;
		m_adjEdges[edge0] = edge1;
		facet->m_adjFacets[edge1] = this;
		facet->m_adjEdges[edge1] = edge0;

		//insure the index of the end points of the edge is the same
		return ((m_indices[edge0] == facet->m_indices[incMod3(edge1)]) &
		(m_indices[incMod3(edge0)] == facet->m_indices[edge1]));
		
	}

	inline bool Facet2::computeClosest(const U32 i0, const U32 i1, const U32 i2, const Maths::HVec3* __restrict a_verts, const Maths::HVec3* __restrict  b_verts)
	{
		using namespace Maths;
		//const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
		const HF32 zero(Maths::Constants<HF32>::Zero());
		//const HVec3& p0 = verts[m_indices[0]]; 
		const HVec3 p0(a_verts[i0]-b_verts[i0]);
		const HVec3 v1((a_verts[i1]-b_verts[i1]) - p0);
		const HVec3 v2((a_verts[i2]-b_verts[i2]) - p0);

		//const Maths::HVec3 v1 = verts[m_indices[1]] - p0;
		//const Maths::HVec3 v2 = verts[m_indices[2]] - p0;
		const HF32 v1dv1 = v1 | v1;
		const HF32 v1dv2 = v1 | v2;
		const HF32 v2dv2 = v2 | v2;
		const HF32 p0dv1 = p0 | v1; 
		const HF32 p0dv2 = p0 | v2;
	    
		const HF32 det( v1dv1 * v2dv2 - v1dv2 * v1dv2 ); // non-negative
		const HF32 lambda1(p0dv2 * v1dv2 - p0dv1 * v2dv2);
		const HF32 lambda2(p0dv1 * v1dv2 - p0dv2 * v1dv1);
		const HVec3 closest(p0 + (lambda1 * v1 + lambda2 * v2) / det);

		//Store( det, &m_det );
		//Store(lambda1, &m_lambda1);
		//Store(lambda2, &m_lambda2);
		m_closest=closest;

		//m_lambda1 = p0dv2 * v1dv2 - p0dv1 * v2dv2;
		//m_lambda2 = p0dv1 * v1dv2 - p0dv2 * v1dv1; 
		//m_closest = p0 + (lambda1 * v1 + lambda2 * v2) / det;

		

		//m_dist = m_closest | m_closest;
		const Maths::HF32 sDist(closest | closest);
		//Store( sDist, &m_dist);
		
		return det.GT(zero);
	}

	__forceinline bool EPASilhouette::originInTetrahedron(const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF p2, 
									const Maths::HVec3 PCREF p3, const Maths::HVec3 PCREF p4)
	{

		using namespace Maths;
		//const HF32 zero(-Maths::Constants<HF32>::Epsilon());
		/*const HF32 zero(Maths::Constants<HF32>::Zero());
		HVec3 normal1((p2 - p1)^(p3 - p1));
		HVec3 normal2((p3 - p2)^(p4 - p2));
		HVec3 normal3((p4 - p3)^(p1 - p3));
		HVec3 normal4((p1 - p4)^(p2 - p4));

		const Maths::HF32 n1p1(normal1|p1);
		const Maths::HF32 n1p4(normal1|p4);
		const Maths::HF32 n2p2(normal2|p2);
		const Maths::HF32 n2p1(normal2|p1);
		const Maths::HF32 n3p3(normal3|p3);
		const Maths::HF32 n3p2(normal3|p2);
		const Maths::HF32 n4p4(normal4|p4);
		const Maths::HF32 n4p3(normal4|p3);

	    
		return	((n1p1.GT(zero)) ^ (n1p4.GT(zero))) &
				((n2p2.GT(zero)) ^ (n2p1.GT(zero))) &
				((n3p3.GT(zero)) ^ (n3p2.GT(zero))) &
				((n4p4.GT(zero)) ^ (n4p3.GT(zero)));*/


		const HF32 zero(SharedPhysics::Constants::Epsilon());
		HVec3 normal1((p2 - p1)^(p3 - p1));
		HVec3 normal2((p3 - p2)^(p4 - p2));
		HVec3 normal3((p4 - p3)^(p1 - p3));
		HVec3 normal4((p1 - p4)^(p2 - p4));

		const Maths::HF32 n1p1(normal1|p1);
		const Maths::HF32 n1p4(normal1|p4);
		const Maths::HF32 n2p2(normal2|p2);
		const Maths::HF32 n2p1(normal2|p1);
		const Maths::HF32 n3p3(normal3|p3);
		const Maths::HF32 n3p2(normal3|p2);
		const Maths::HF32 n4p4(normal4|p4);
		const Maths::HF32 n4p3(normal4|p3);
	    
		return (n1p1 * n1p4).LE(zero) & 
			(n2p2 * n2p1).LE(zero) & 
			(n3p3 * n3p2).LE(zero) & 
			(n4p4 * n4p3).LE(zero);

	}

	#ifndef PLATFORM_SPU
	template <class ConvexA, class ConvexB>
	bool EPASilhouette::PenetrationDepth(const ConvexA& a, const ConvexB& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8 size,  Maths::HVec3& pa, Maths::HVec3& pb)
	{
		using namespace Maths;

		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		const HF32 two(Constants<HF32>::Two());
		const HF32 _max(Constants<HF32>::Max());
		//const HF32 epsilon(Maths::Min(a.Epsilon(), b.Epsilon());
		const HF32 eps2(SharedPhysics::Constants::Epsilon());
	
		
		num_verts=size;
		num_facets = 0;
		freeFacet = 0;
		
		aBuf[0]=A[0]; aBuf[1]=A[1]; aBuf[2]=A[2]; aBuf[3]=A[3];
		bBuf[0]=B[0]; bBuf[1]=B[1]; bBuf[2]=B[2]; bBuf[3]=B[3];


		
		heap.makeEmpty();

		//if the simplex isn't a tetrahedron, we need to construct one before we can expand it
			switch (num_verts) 
		{
		case 1:
			// Touching contact. Yes, we have a collision,
			// but no penetration.
			return false;
		case 2:	
		{
			// We have a line segment inside the Minkowski sum containing the
			// origin. Blow it up by adding three additional support points.
			const HVec3 q0(aBuf[0]-bBuf[0]);
			const HVec3 q1(aBuf[1]-bBuf[1]);
			C_Normal dir(q1-q0);

			Maths::HVec3 t1(Maths::UNDEFINED), aux1(Maths::UNDEFINED);
			dir.m_V.PerpendicularAxes(t1, aux1);
			
			//const Maths::HF32 sDir(dir|dir);
			//const Maths::C_Normal temp2(sDir, sDir, sDir);
			//const Maths::C_Normal t1(Normalise(temp2 ^ dir));
		    
			//const C_Normal aux1(dir^t1, C_Normal::UNIT_LENGTH_VECTOR);
			aBuf[2] = a.Support(aux1);
			bBuf[2] = b.Support(-aux1);
			const HVec3 q2(aBuf[2]-bBuf[2]);

			const HF32 sinHalfTheta0 = -SharedPhysics::Constants::SinSixty();//sin(120/2)
			const HF32 cosHalfTheta0 = SharedPhysics::Constants::CosSixty();
			const HVec3 imag0 = dir * sinHalfTheta0;
			C_Quaternion qua0(imag0.X(), imag0.Y(), imag0.Z(), cosHalfTheta0);
			const C_Normal aux2(aux1 * qua0);

			aBuf[3] = a.Support(aux2);
			bBuf[3] = b.Support(-aux2);
			const HVec3 q3(aBuf[3]-bBuf[3]);

			const HF32 sinHalfTheta1 = -sinHalfTheta0;//sin(120/2)
			const HF32 cosHalfTheta1 = cosHalfTheta0;
			const HVec3 imag1 = dir * sinHalfTheta1;
			C_Quaternion qua1(imag1.X(), imag1.Y(), imag1.Z(), cosHalfTheta1);
			
			const HVec3 aux3(aux1 * qua1);
			aBuf[4] = a.Support(aux3);
			bBuf[4] = b.Support(-aux3);
			const HVec3 q4(aBuf[4]-bBuf[4]);

		    
			if (originInTetrahedron(q0, q2, q3, q4)) 
			{
				aBuf[1] = aBuf[4];
				bBuf[1] = bBuf[4];
				//DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred(), q0, q2, q3, q4);
			}
			else if (originInTetrahedron(q1, q2, q3, q4)) 
			{
				aBuf[0] = aBuf[4];
				bBuf[0] = bBuf[4];
				//DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred(), q1, q2, q3, q4);
			} 
			else 
			{
				// Origin not in initial polytope
				//DebugRender(DbUtil::DefaultDebugRenderSys()->Persistent(), q0, q2, q3,q4);
				//DebugRender(DbUtil::DefaultDebugRenderSys()->Persistent(), q1, q2, q3,q4);
				return false;
			}
		    
			num_verts = 4;
			break;
		}
		case 3: 
		{
			// We have a triangle inside the Minkowski sum containing
			// the origin. First blow it up.
		    
			num_verts = 4;

			const HVec3 q0(aBuf[0]-bBuf[0]);
			const HVec3 q1(aBuf[1]-bBuf[1]);
			const HVec3 q2(aBuf[2]-bBuf[2]);

			const HVec3 v1(q1 - q0);
			const HVec3 v2(q2 - q0);
			const HVec3 vv(v1^v2);
		    
			aBuf[3] = a.Support(vv);
			bBuf[3] = b.Support(-vv);
			const HVec3 q3(aBuf[3] - bBuf[3]);

			aBuf[4] = a.Support(-vv);
			bBuf[4] = b.Support(vv);
			const HVec3 q4(aBuf[4] - bBuf[4]);

			HVec3 ab(q0 - q3);
			const HVec3 ac(q4 - q3);
			const HVec3 n(ab^ac);
			const HF32 dist(q3|n);
			const HF32 signDist00((q1|n) - dist);
			const HF32 signDist01((q2|n) - dist);
		

			if((signDist00*signDist01)<zero)
			{
				if(originInTetrahedron(q1, q0, q3, q4))
				{
					aBuf[2] = aBuf[4];
					bBuf[2] = bBuf[4];
					break;
				}
				else if(originInTetrahedron(q2, q0, q3, q4))
				{
					aBuf[1] = aBuf[4];
					bBuf[1] = bBuf[4];
					break;
				}
			}
			
			ab = q1 - q3;
			const HVec3 n1(ab^ac);
			//const HF32 scaledEpsilon(Maths::Max(eps2, eps2 * (n1 | n1)));
			const HF32 dist1(q3|n1);
			const HF32 signDist10((q0|n1) - dist1);
			const HF32 signDist11((q2|n1) - dist1);
			const HF32 side = signDist10*signDist11;
			if((signDist10*signDist11) < zero)
			{
				if(originInTetrahedron(q0, q1, q3, q4))
				{
					aBuf[2] = aBuf[4];
					bBuf[2] = bBuf[4];
					break;
				}
				else if(originInTetrahedron(q2, q1, q3, q4))
				{
					aBuf[0] = aBuf[4];
					bBuf[0] = bBuf[4];
					break;
				}
			}
					
				
			if(originInTetrahedron(q0, q2, q3, q4))
			{
				aBuf[1] = aBuf[4];
				bBuf[1] = bBuf[4];
			}
			else if(originInTetrahedron(q1, q2, q3, q4))
			{
				aBuf[0] = aBuf[4];
				bBuf[0] = bBuf[4];
			}
			else
			{
				return false;
			}
		    
			break;
		}
		}


		/*Facet2 *f0 = addInitialFacet(0, 1, 2, zero, _max);
		Facet2 *f1 = addInitialFacet(0, 3, 1, zero, _max);
		Facet2 *f2 = addInitialFacet(0, 2, 3, zero, _max);
		Facet2 *f3 = addInitialFacet(1, 3, 2, zero, _max);*/


		Facet2 *f0 = addFacet(0, 1, 2, zero, _max);
		Facet2 *f1 = addFacet(0, 3, 1, zero, _max);
		Facet2 *f2 = addFacet(0, 2, 3, zero, _max);
		Facet2 *f3 = addFacet(1, 3, 2, zero, _max);
		if(!f0 || !f1 || !f2 || !f3 || heap.isEmpty())
			return false;


		f0->link(0, f1, 2);
		f0->link(1, f3, 2);
		f0->link(2, f2, 0);
		f1->link(0, f2, 2);
		f1->link(1, f3, 0);
		f2->link(1, f3, 1);
	
		
	    
		// at least one facet on the heap.	
	    
		//edgeBuffer.MakeEmpty();
		//int edgeCount=0;

		
	    
		HF32 upper_bound2(_max);
		const HF32 eps(Constants<HF32>::Epsilon());
		//const HF32 ee(one + Maths::Constants<Maths::HF32>::Epsilon2());
		//const HF32 ee(e * e);
	
	
		Facet2* facet= heap.getTop();
		if(facet->getDist() < eps2)//the closest point is the origin itself
		{
			const Facet2* adjFacet0 = facet->m_adjFacets[0];
			const U32 edgeStart = facet->m_adjEdges[0];
			const U32 edgeEnd = incMod3(edgeStart);
			const U32 num = 3 - edgeStart - edgeEnd;
			const U32 index = adjFacet0->m_indices[num];
			const HVec3 dir = -(aBuf[index] - bBuf[index]);
			const HVec3 tempa(a.Support(dir));
			const HVec3 tempb(b.Support(-dir));
			const HVec3 q(tempa - tempb);
			const HF32 dist(q|facet->getClosest());
			facet->setDir(Maths::Select(dist.GT(zero), dir, facet->getDir()));
		}
		
		
		
		do 
		{
			heap.deleteTop(&facet); //get the shortest distance triangle of origin from the list
			
			if (!facet->isObsolete()) 
			{
				BIZ_ASSERT(facet->getDist() > zero);
				
				if (num_verts == MaxSupportPoints)	
					break;

				const HF32 sqDist = facet->getDist();
				const HVec3 closest(facet->getClosest());
				const HVec3 dir(facet->getDir());

				/*const HVec3 tempa(a.Support(closest));
				const HVec3 tempb(b.Support(-closest));*/

				const HVec3 tempa(a.Support(dir));
				const HVec3 tempb(b.Support(-dir));


				aBuf[num_verts]=tempa;
				bBuf[num_verts]=tempb;
				const HVec3 q(tempa - tempb);
				
				
				const U32 index =num_verts++;
				
				//if the support point and closest point close enough, found the contact point, break
				//HF32 dist(qBuf[index]|closest);
				const HF32 dist(q|closest);
			
				/*const HF32 er(ee * sfDist);
				upper_bound2=Min(upper_bound2, (sDist * sDist) / sfDist);
				if(upper_bound2 <= er )
				{
					break;
				}*/

				/*const HF32 t(eps + Sqrt(sqDist));
				const HF32 er(t*t);
				upper_bound2 = Min(upper_bound2, (dist*dist)/sqDist);
				if(upper_bound2 <= er)
				{
					break;
				}*/

				//const HF32 t(eps + Sqrt(sqDist));
				//const HF32 er(t*t);
				upper_bound2 = Min(upper_bound2, (dist*dist)/sqDist);
				if(upper_bound2 <= sqDist)
				{
					break;
				}
				
				
				//upper_bound2=Min(upper_bound2, (dist * dist) / scdist);
				// Compute the silhouette cast by the new vertex
				// Note that the new vertex is on the positive side
				// of the current facet, so the current facet is will
				// not be in the convex hull. Start local search
				// from this facet.
				
				edgeBuffer.MakeEmpty();
				facet->silhouette(q, edgeBuffer, stack);
				
				//assert(!edgeBuffer.IsEmpty());
				if(edgeBuffer.IsEmpty()) 
				{
					return false;
				}
				
				
				Edge* edge=edgeBuffer.Get(0);
			
				Facet2 *firstFacet = 
					addFacet(edge->getTarget(), edge->getSource(),
							 index, sqDist, upper_bound2);
							 

				if (!firstFacet) 
				{
					break;
				}
				
				
				firstFacet->link(0, edge->getFacet(), edge->getIndex());
				Facet2 *lastFacet = firstFacet;
				
				U32 size=edgeBuffer.Size();
				
				U32 i;
				for(i=1; i<size; ++i)
				{
					edge=edgeBuffer.Get(i);
					Facet2* newFacet = 
						addFacet(edge->getTarget(), edge->getSource(),
								 index, sqDist, upper_bound2);

					if (!newFacet) 
					{
						break;
					}
					
					if(!newFacet->link(0, edge->getFacet(), edge->getIndex()))
					{
						break;
					}
					
					if (!newFacet->link(2, lastFacet, 1)) 
					{
						break;
					}

					lastFacet = newFacet;	

				}
				if(i != size)
					break;

				firstFacet->link(2, lastFacet, 1);
				
		
			}

		}
		while ((--num_facets) && heap.getTop()->getDist() <= upper_bound2);
	    
		facet->getClosestPoint(aBuf, bBuf, pa, pb);

		return true;
	}


	template <class ConvexA, class ConvexB>
	bool EPASilhouette::PenetrationDepthMargin(const ConvexA& a, const ConvexB& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8 size,  Maths::HVec3& pa, Maths::HVec3& pb, const Maths::HF32 PCREF marginA, const Maths::HF32 PCREF marginB)

	{
		using namespace Maths;

		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		const HF32 two(Constants<HF32>::Two());
		const HF32 _max(Constants<HF32>::Max());
		const HF32 eps2(SharedPhysics::Constants::Epsilon());
		//const HF32 epsilon(Maths::Constants<HF32>::Epsilon2());
		
		num_verts=size;
		num_facets = 0;
		freeFacet = 0;
		
		aBuf[0]=A[0]; aBuf[1]=A[1]; aBuf[2]=A[2]; aBuf[3]=A[3];
		bBuf[0]=B[0]; bBuf[1]=B[1]; bBuf[2]=B[2]; bBuf[3]=B[3];


		
		heap.makeEmpty();

		//if the simplex isn't a tetrahedron, we need to construct one before we can expand it
		switch (num_verts) 
		{
		case 1:
			// Touching contact. Yes, we have a collision,
			// but no penetration.
			return false;
		case 2:	
		{
			// We have a line segment inside the Minkowski sum containing the
			// origin. Blow it up by adding three additional support points.
			const HVec3 q0(aBuf[0]-bBuf[0]);
			const HVec3 q1(aBuf[1]-bBuf[1]);
			C_Normal dir(q1-q0);

			Maths::HVec3 t1(Maths::UNDEFINED), aux1(Maths::UNDEFINED);
			dir.m_V.PerpendicularAxes(t1, aux1);
			
			//const Maths::HF32 sDir(dir|dir);
			//const Maths::C_Normal temp2(sDir, sDir, sDir);
			//const Maths::C_Normal t1(Normalise(temp2 ^ dir));
		    
			//const C_Normal aux1(dir^t1, C_Normal::UNIT_LENGTH_VECTOR);
			aBuf[2] = a.SupportMargin(aux1, marginA);
			bBuf[2] = b.SupportMargin(-aux1, marginB);
			const HVec3 q2(aBuf[2]-bBuf[2]);

			const HF32 sinHalfTheta0 = -SharedPhysics::Constants::SinSixty();//sin(120/2)
			const HF32 cosHalfTheta0 = SharedPhysics::Constants::CosSixty();
			const HVec3 imag0 = dir * sinHalfTheta0;
			C_Quaternion qua0(imag0.X(), imag0.Y(), imag0.Z(), cosHalfTheta0);
			const C_Normal aux2(aux1 * qua0);

			aBuf[3] = a.SupportMargin(aux2, marginA);
			bBuf[3] = b.SupportMargin(-aux2, marginB);
			const HVec3 q3(aBuf[3]-bBuf[3]);

			const HF32 sinHalfTheta1 = -sinHalfTheta0;//sin(120/2)
			const HF32 cosHalfTheta1 = cosHalfTheta0;
			const HVec3 imag1 = dir * sinHalfTheta1;
			C_Quaternion qua1(imag1.X(), imag1.Y(), imag1.Z(), cosHalfTheta1);
			
			const HVec3 aux3(aux1 * qua1);
			aBuf[4] = a.SupportMargin(aux3, marginA);
			bBuf[4] = b.SupportMargin(-aux3, marginB);
			const HVec3 q4(aBuf[4]-bBuf[4]);

		    
			if (originInTetrahedron(q0, q2, q3, q4)) 
			{
				aBuf[1] = aBuf[4];
				bBuf[1] = bBuf[4];
				//DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred(), q0, q2, q3, q4);
			}
			else if (originInTetrahedron(q1, q2, q3, q4)) 
			{
				aBuf[0] = aBuf[4];
				bBuf[0] = bBuf[4];
				//DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred(), q1, q2, q3, q4);
			} 
			else 
			{
				// Origin not in initial polytope
				//DebugRender(DbUtil::DefaultDebugRenderSys()->Persistent(), q0, q2, q3,q4);
				//DebugRender(DbUtil::DefaultDebugRenderSys()->Persistent(), q1, q2, q3,q4);
				return false;
			}
		    
			num_verts = 4;
			break;
		}
		case 3: 
		{
			num_verts = 4;

			const HVec3 q0(aBuf[0]-bBuf[0]);
			const HVec3 q1(aBuf[1]-bBuf[1]);
			const HVec3 q2(aBuf[2]-bBuf[2]);

			const HVec3 v1(q1 - q0);
			const HVec3 v2(q2 - q0);
			const HVec3 vv(v1^v2);
		    
			aBuf[3] = a.Support(vv);
			bBuf[3] = b.Support(-vv);
			const HVec3 q3(aBuf[3] - bBuf[3]);

			aBuf[4] = a.Support(-vv);
			bBuf[4] = b.Support(vv);
			const HVec3 q4(aBuf[4] - bBuf[4]);

			HVec3 ab(q0 - q3);
			const HVec3 ac(q4 - q3);
			const HVec3 n(ab^ac);
			const HF32 dist(q3|n);
			const HF32 signDist00((q1|n) - dist);
			const HF32 signDist01((q2|n) - dist);
			const HF32 signDist(signDist00*signDist01);


			if((signDist00*signDist01)<zero)
			{
				if(originInTetrahedron(q1, q0, q3, q4))
				{
					aBuf[2] = aBuf[4];
					bBuf[2] = bBuf[4];
					break;
				}
				else if(originInTetrahedron(q2, q0, q3, q4))
				{
					aBuf[1] = aBuf[4];
					bBuf[1] = bBuf[4];
					break;
				}
			}
			
			ab = q1 - q3;
			const HVec3 n1(ab^ac);
			//const HF32 scaledEpsilon(Maths::Max(eps2, eps2 * (n1 | n1)));
			const HF32 dist1(q3|n1);
			const HF32 signDist10((q0|n1) - dist1);
			const HF32 signDist11((q2|n1) - dist1);
			const HF32 side = signDist10*signDist11;
			if((signDist10*signDist11) < zero)
			{
				if(originInTetrahedron(q0, q1, q3, q4))
				{
					aBuf[2] = aBuf[4];
					bBuf[2] = bBuf[4];
					break;
				}
				else if(originInTetrahedron(q2, q1, q3, q4))
				{
					aBuf[0] = aBuf[4];
					bBuf[0] = bBuf[4];
					break;
				}
			}
					
				
			if(originInTetrahedron(q0, q2, q3, q4))
			{
				aBuf[1] = aBuf[4];
				bBuf[1] = bBuf[4];
			}
			else if(originInTetrahedron(q1, q2, q3, q4))
			{
				aBuf[0] = aBuf[4];
				bBuf[0] = bBuf[4];
			}
			else
			{
				return false;
			}
		    
			break;
		}
		}

		Facet2 *f0 = addFacet(0, 1, 2, zero, _max);
		Facet2 *f1 = addFacet(0, 3, 1, zero, _max);
		Facet2 *f2 = addFacet(0, 2, 3, zero, _max);
		Facet2 *f3 = addFacet(1, 3, 2, zero, _max);

		if(!f0 || !f1 || !f2 || !f3 || heap.isEmpty())
			return false;

		f0->link(0, f1, 2);
		f0->link(1, f3, 2);
		f0->link(2, f2, 0);
		f1->link(0, f2, 2);
		f1->link(1, f3, 0);
		f2->link(1, f3, 1);

		
		
	    
		// at least one facet on the heap.	
	    
		edgeBuffer.MakeEmpty();
		//int edgeCount=0;

		//Facet2* facet;
	    
		HF32 upper_bound2(_max);
		const HF32 eps(Constants<HF32>::Epsilon());
		//const HF32 ee(one + Maths::Constants<Maths::HF32>::Epsilon2());
		//const HF32 ee(e * e);
		Facet2* facet= heap.getTop();
		if(facet->getDist() < eps2)//the closest point is the origin itself
		{
			const Facet2* adjFacet0 = facet->m_adjFacets[0];
			const U32 edgeStart = facet->m_adjEdges[0];
			const U32 edgeEnd = incMod3(edgeStart);
			const U32 num = 3 - edgeStart - edgeEnd;
			const U32 index = adjFacet0->m_indices[num];
			const HVec3 dir = -(aBuf[index] - bBuf[index]);
			const HVec3 tempa(a.SupportMargin(dir, marginA));
			const HVec3 tempb(b.SupportMargin(-dir, marginB));
			const HVec3 q(tempa - tempb);
			const HF32 dist(q|facet->getClosest());
			facet->setDir(Select(dist.GT(zero), dir, facet->getDir()));
		}

		do 
		{
			heap.deleteTop(&facet); //get the shortest distance triangle of origin from the list
			
			if (!facet->isObsolete()) 
			{

				const HF32 sqDist = facet->getDist();

				BIZ_ASSERT(sqDist > zero);
				
				if (num_verts == MaxSupportPoints)	
					break;

				const HVec3 closest(facet->getClosest());
				const HVec3 dir(facet->getDir());

				const HVec3 tempa(a.SupportMargin(dir, marginA));
				const HVec3 tempb(b.SupportMargin(-dir, marginB));


				aBuf[num_verts]=tempa;
				bBuf[num_verts]=tempb;
				const HVec3 q(tempa - tempb);
				
				const U32 index =num_verts++;
				
				//if the support point and closest point close enough, found the contact point, break
				const HF32 dist(q|closest);
				
			
				/*const HF32 er(ee * sfDist);
				upper_bound2=Min(upper_bound2, (sDist * sDist) / sfDist);
				if(upper_bound2 <= er )
				{
					break;
				}*/

				const HF32 t(eps + Sqrt(sqDist));
				const HF32 er(t*t);
				upper_bound2 = Min(upper_bound2, (dist*dist)/sqDist);
				if(upper_bound2 <= er)
				{
					break;
				}
				
				
				//upper_bound2=Min(upper_bound2, (dist * dist) / scdist);
				// Compute the silhouette cast by the new vertex
				// Note that the new vertex is on the positive side
				// of the current facet, so the current facet is will
				// not be in the convex hull. Start local search
				// from this facet.
				
				edgeBuffer.MakeEmpty();
				facet->silhouette(q, edgeBuffer, stack);
				
				//BIZ_ASSERT(!edgeBuffer.IsEmpty());
				if (edgeBuffer.IsEmpty()) 
				{
					return false;
				} 
				
				
				Edge* edge=edgeBuffer.Get(0);
			
				Facet2 *firstFacet = 
					addFacet(edge->getTarget(), edge->getSource(),
							 index, sqDist, upper_bound2);
							 

				if (!firstFacet) 
				{
					break;
				}
				
				
				firstFacet->link(0, edge->getFacet(), edge->getIndex());
				Facet2 *lastFacet = firstFacet;
				
				U32 size=edgeBuffer.Size();
				
				U32 i;
				for(i=1; i<size; ++i)
				{
					edge=edgeBuffer.Get(i);
					Facet2* newFacet = 
						addFacet(edge->getTarget(), edge->getSource(),
								 index, sqDist, upper_bound2);

					if (!newFacet) 
					{
						break;
					}
					
					if(!newFacet->link(0, edge->getFacet(), edge->getIndex()))
					{
						break;
					}
					
					if (!newFacet->link(2, lastFacet, 1)) 
					{
						break;
					}

					lastFacet = newFacet;	

				}
				if(i != size)
					break;

				firstFacet->link(2, lastFacet, 1);
				
		
			}

		}
		while ((--num_facets) && heap.getTop()->getDist() <= upper_bound2);
	    
		facet->getClosestPoint(aBuf, bBuf, pa, pb);

		return true;
	}
#endif

}
#endif
