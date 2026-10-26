#include "StdAfx.h"
#include "gravity/narrowphase/EPA.h"
#include "gravity/sharedphysics/Select.h"

namespace Collision
{

	void FacetSimplified::Initialise(const U32& i0, const U32& i1, const U32& i2, const Maths::HVec3* __restrict a_verts, const Maths::HVec3* __restrict  b_verts)
	{
		using namespace Maths;

		const HVec3 p0(a_verts[i0]-b_verts[i0]);
		const HVec3 p1(a_verts[i1]-b_verts[i1]);
		const HVec3 p2(a_verts[i2]-b_verts[i2]);
		
		const HVec3 v1(p2 - p0);
		const HVec3 v2(p1 - p0);
		const C_Normal n(v1^v2);
		const HF32 t(n|p0);
		m_dir = C_Normal(t*n);
		m_dist = t*t;
	}

	bool FacetSimplified::isValid(const U32& i0, const U32& i1, const U32& i2, const Maths::HVec3* __restrict a_verts, const Maths::HVec3* __restrict  b_verts)
	{
		using namespace Maths;
		const HF32 zero(-Constants<Maths::HF32>::Epsilon());
		const HVec3 p0(a_verts[i0]-b_verts[i0]);
		const HVec3 v1((a_verts[i1]-b_verts[i1]) - p0);
		const HVec3 v2((a_verts[i2]-b_verts[i2]) - p0);

		const HF32 v1dv1 ( v1 | v1);
		const HF32 v1dv2 ( v1 | v2);
		const HF32 v2dv2 ( v2 | v2);
		const HF32 p0dv1 ( p0 | v1); 
		const HF32 p0dv2 ( p0 | v2);
	    
		const HF32 det( v1dv1 * v2dv2 - v1dv2 * v1dv2 ); // non-negative
		//BIZ_ASSERT(det >=  Constants<HF32>::Zero());
		if(det.LT(Constants<HF32>::Zero()))
			return false;
		const HF32 lambda1(p0dv2 * v1dv2 - p0dv1 * v2dv2);
		const HF32 lambda2(p0dv1 * v1dv2 - p0dv2 * v1dv1);
		const HVec3 closest(p0 + (lambda1 * v1 + lambda2 * v2) / det);

		m_dir=C_Normal(closest);
		const HF32 dist(m_dir|p0);
		m_dist = dist*dist;
		
		return ((lambda1.GT(zero)) & (lambda2.GT(zero)) & ((lambda1+lambda2).LE(det))) != 0;
	}

	FacetSimplified* EPASimplified::addFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2)
	{
		using namespace Maths;
		BIZ_ASSERT(i0 != i1 && i0 != i2 && i1 != i2);
		if (freeFacet < MaxFacets)
		{
			FacetSimplified *facet = new(&facetBuf[freeFacet])FacetSimplified(i0, i1, i2);

			if(facet->isValid(i0, i1, i2, aBuf, bBuf) && lower2.LE(facet->m_dist) && facet->m_dist.LE(upper2))
			{
				heap.insert(*facet);
				++num_facets;
				++freeFacet;
				return facet;
			}
		}
		return 0;
	}

	FacetSimplified* EPASimplified::addInitialFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2)
	{
		using namespace Maths;
		BIZ_ASSERT(i0 != i1 && i0 != i2 && i1 != i2);
		//if (freeFacet < MaxFacets)
		{
			FacetSimplified *facet = new(&facetBuf[freeFacet])FacetSimplified(i0, i1, i2);
			facet->isValid(i0, i1, i2, aBuf, bBuf);

			//if(facet->isValid(i0, i1, i2, aBuf, bBuf) && lower2.LE(facet->m_dist) && facet->m_dist.LE(upper2))
			{
				heap.insert(*facet);
				++num_facets;
				++freeFacet;
				return facet;
			}
		}
		//return 0;
	}


bool EPASimplified::originInTetrahedron(const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF p2, 
								const Maths::HVec3 PCREF p3, const Maths::HVec3 PCREF p4)
{

	using namespace Maths;
	//const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
	const HF32 zero(Maths::Constants<HF32>::Zero());
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
    /*return	(((n1p1.GT(zero)) ^ (n1p4.GT(zero))) | ((n1p1.GT(epsilon)) ^ (n1p4.GT(epsilon))))&
			(((n2p2.GT(zero)) ^ (n2p1.GT(zero))) | ((n2p2.GT(epsilon)) ^ (n2p1.GT(epsilon)))) &
			(((n3p3.GT(zero)) ^ (n3p2.GT(zero))) | ((n3p3.GT(epsilon)) ^ (n3p2.GT(epsilon))))&
			(((n4p4.GT(zero)) ^ (n4p3.GT(zero))) | ((n4p4.GT(epsilon)) ^ (n4p3.GT(epsilon))));*/
}


#ifdef PLATFORM_SPU

bool EPASimplified::PenetrationDepth(const C_Convex &a, const C_Convex &b, const Maths::HVec3 *__restrict Q, const Maths::HVec3 *__restrict A, const Maths::HVec3 *__restrict B, const U8& size, Maths::HVec3 &pa, Maths::HVec3 &pb)
{

	using namespace Maths;

	Spu::MemBaseMarker marker;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 two(Constants<HF32>::Two());
	const HF32 _max(Constants<HF32>::Max());
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
		// origin. Blow it up by adding three additional support points
		const HVec3 q0(aBuf[0]-bBuf[0]);
		const HVec3 q1(aBuf[1]-bBuf[1]);
		C_Normal dir(q1-q0);
		
		const Maths::HF32 sDir(dir|dir);
		const Maths::C_Normal temp2(sDir, sDir, sDir);
		const Maths::C_Normal t1(Normalise(temp2 ^ dir));
	    
		const C_Normal aux1(dir^t1, C_Normal::UNIT_LENGTH_VECTOR);
		aBuf[2] = m_EpaManager.Support(&a, aux1);
		bBuf[2] = m_EpaManager.Support(&b, -aux1);
		const HVec3 q2(aBuf[2]-bBuf[2]);

		const HF32 sinHalfTheta0 = -SharedPhysics::Constants::SinSixty();//sin(120/2)
		const HF32 cosHalfTheta0 = SharedPhysics::Constants::CosSixty();
		const HVec3 imag0 = dir * sinHalfTheta0;
		C_Quaternion qua0(imag0.X(), imag0.Y(), imag0.Z(), cosHalfTheta0);
		const C_Normal aux2(aux1 * qua0);

		aBuf[3] = m_EpaManager.Support(&a, aux2);
		bBuf[3] = m_EpaManager.Support(&b, -aux2);
		const HVec3 q3(aBuf[3]-bBuf[3]);

		const HF32 sinHalfTheta1 = -sinHalfTheta0;
		const HF32 cosHalfTheta1 = cosHalfTheta0;
		const HVec3 imag1 = dir * sinHalfTheta1;
		C_Quaternion qua1(imag1.X(), imag1.Y(), imag1.Z(), cosHalfTheta1);
		
		const C_Normal aux3(aux1 * qua1);
		aBuf[4] = m_EpaManager.Support(&a, aux3);
		bBuf[4] = m_EpaManager.Support(&b, -aux3);
		const HVec3 q4(aBuf[4]-bBuf[4]);

		if (originInTetrahedron(q0, q2, q3, q4)) 
		{
			aBuf[1] = aBuf[4];
			bBuf[1] = bBuf[4];
		}
		else if (originInTetrahedron(q1, q2, q3, q4)) 
		{
			aBuf[0] = aBuf[4];
			bBuf[0] = bBuf[4];
		} 
		else 
		{
			// Origin not in initial polytope

			return false;
		}
	    
		num_verts = 4;
		break;

	}
	case 3: 
		{
			// We have a triangle inside the Minkowski sum containing
			// the origin. First blow it up.
		    
			const HVec3 q0(aBuf[0]-bBuf[0]);
			const HVec3 q1(aBuf[1]-bBuf[1]);
			const HVec3 q2(aBuf[2]-bBuf[2]);

			const HVec3 v1(q1 - q0);
			const HVec3 v2(q2 - q0);
			const HVec3 vv(v1^v2);
		   
			aBuf[3] = m_EpaManager.Support(&a, vv);
			bBuf[3] = m_EpaManager.Support(&b, -vv);
			const HVec3 q3(aBuf[3] - bBuf[3]);

			aBuf[4] = m_EpaManager.Support(&a, -vv);
			bBuf[4] = m_EpaManager.Support(&b, vv);
			const HVec3 q4(aBuf[4] - bBuf[4]);
		  
		    
		   
			if (originInTetrahedron(q0, q1, q2, q4)) 
			{
				aBuf[3] = aBuf[4];
				bBuf[3] = bBuf[4];
			}
			else if (!originInTetrahedron(q0, q1, q2, q3))
			{ 
				// Origin not in initial polytope
				return false;
			}
		    
			num_verts = 4;
		    
			break;
		}
	}

	
    FacetSimplified *f0 = addFacet(0, 1, 2, zero, _max);
    FacetSimplified *f1 = addFacet(0, 3, 1, zero, _max);
    FacetSimplified *f2 = addFacet(0, 2, 3, zero, _max);
    FacetSimplified *f3 = addFacet(1, 3, 2, zero, _max);
	
	
	/*if(!f0 || f0->getDist() < epsilon ||!f1 || f1->getDist() < epsilon||!f2 || f2->getDist() < epsilon ||!f3 || f3->getDist() < epsilon)
	{
		return false;
	}*/

	if(heap.isEmpty())
		return false;

	FacetSimplified f;
	FacetSimplified* facet = &f;

	
   // HF32 upper_bound2(_max);
	HF32 upper_bound2(_max);
	/*const HF32 ee(one + epsilon);
	const HF32 e(ee * ee);*/
	const HF32 eps(Constants<HF32>::Epsilon());

	//const HF32 eps2(0.000001f);
	//FacetSimplified* facet = heap.getTop();
	//if(facet->getDist() < eps2)//the closest point is the origin itself
	//{
	//	const FacetSimplified* adjFacet0 = facet->m_adjFacets[0];
	//	const U32 edgeStart = facet->m_adjEdges[0];
	//	const U32 edgeEnd = incMod3(edgeStart);
	//	const U32 num = 3 - edgeStart - edgeEnd;
	//	const U32 index = adjFacet0->m_indices[num];
	//	const HVec3 closest = -(aBuf[index] - bBuf[index]);
	//	facet->setSupportDir(closest);

	//}
	

    do 
	{
		Spu::MemBaseMarker innerMarker;
		heap.deleteTop(f); //get the shortest distance triangle of origin from the list

		const HF32 sFacetDist(facet->getDist());

		BIZ_ASSERT(sFacetDist > zero);
		
		if (num_verts == MaxSupportPoints)	
			break;

		const HVec3 dir(facet->getSupportDir());

		const HVec3 tempa(m_EpaManager.Support(&a, dir));
		const HVec3 tempb(m_EpaManager.Support(&b, -dir));

		aBuf[num_verts]=tempa;
		bBuf[num_verts]=tempb;
		const HVec3 q(tempa - tempb);
		
		U32 index =num_verts++;

		//if the support point and closest point close enough, found the contact point, break
		//HF32 dist(qBuf[index]|closest);
		const HF32 u(q|dir);
		const HF32 su(u*u);
		upper_bound2 = Min(upper_bound2, su);
		
		const HF32 err(eps + Sqrt(sFacetDist));
		const HF32 err2(err*err);
		if(upper_bound2 <= err2)
		{
			break;
		}

		addFacet((*facet)[0], (*facet)[1], index, sFacetDist, upper_bound2);
		addFacet((*facet)[1], (*facet)[2], index, sFacetDist, upper_bound2);
		addFacet((*facet)[2], index, (*facet)[0], sFacetDist, upper_bound2);


		/*const HF32 u(q|dir);
		upper_bound = Min(upper_bound, u);
		
		const HF32 dif = upper_bound - facetDist;
		const HF32 dif2 = dif*dif;
		if(dif2 <= eps)
		{
			break;
		}

		addFacet((*facet)[0], (*facet)[1], index, facetDist, upper_bound);
		addFacet((*facet)[1], (*facet)[2], index, facetDist, upper_bound);
		addFacet((*facet)[2], index, (*facet)[0], facetDist, upper_bound);*/

    }
	while ((--num_facets) && U32(heap.getTop().getDist() <= upper_bound2));


	facet->getClosestPoint(aBuf, bBuf, pa, pb);
	
	
    return true;
}

bool EPASimplified::PenetrationDepthMargin(const C_Convex &a, const C_Convex &b, const Maths::HVec3 *__restrict Q, const Maths::HVec3 *__restrict A, const Maths::HVec3 *__restrict B, const U8& size, Maths::HVec3 &pa, Maths::HVec3 &pb, const Maths::HF32 PCREF marginA, const Maths::HF32 PCREF marginB)
{

	using namespace Maths;

	Spu::MemBaseMarker marker;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 two(Constants<HF32>::Two());
	const HF32 _max(Constants<HF32>::Max());
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
		// origin. Blow it up by adding three additional support points
		const HVec3 q0(aBuf[0]-bBuf[0]);
		const HVec3 q1(aBuf[1]-bBuf[1]);
		C_Normal dir(q1-q0);
		
		const Maths::HF32 sDir(dir|dir);
		const Maths::C_Normal temp2(sDir, sDir, sDir);
		const Maths::C_Normal t1(Normalise(temp2 ^ dir));
	    
		const C_Normal aux1(dir^t1, C_Normal::UNIT_LENGTH_VECTOR);
		aBuf[2] = m_EpaManager.SupportMargin(&a, aux1, marginA);
		bBuf[2] = m_EpaManager.SupportMargin(&b, -aux1, marginB);
		const HVec3 q2(aBuf[2]-bBuf[2]);

		const HF32 sinHalfTheta0 = -SharedPhysics::Constants::SinSixty();//sin(120/2)
		const HF32 cosHalfTheta0 = SharedPhysics::Constants::CosSixty();
		const HVec3 imag0 = dir * sinHalfTheta0;
		C_Quaternion qua0(imag0.X(), imag0.Y(), imag0.Z(), cosHalfTheta0);
		const C_Normal aux2(aux1 * qua0);

		aBuf[3] = m_EpaManager.SupportMargin(&a, aux2, marginA);
		bBuf[3] = m_EpaManager.SupportMargin(&b, -aux2, marginB);
		const HVec3 q3(aBuf[3]-bBuf[3]);

		const HF32 sinHalfTheta1 = -sinHalfTheta0;
		const HF32 cosHalfTheta1 = cosHalfTheta0;
		const HVec3 imag1 = dir * sinHalfTheta1;
		C_Quaternion qua1(imag1.X(), imag1.Y(), imag1.Z(), cosHalfTheta1);
		
		const C_Normal aux3(aux1 * qua1);
		aBuf[4] = m_EpaManager.SupportMargin(&a, aux3, marginA);
		bBuf[4] = m_EpaManager.SupportMargin(&b, -aux3, marginB);
		const HVec3 q4(aBuf[4]-bBuf[4]);

		if (originInTetrahedron(q0, q2, q3, q4)) 
		{
			aBuf[1] = aBuf[4];
			bBuf[1] = bBuf[4];
		}
		else if (originInTetrahedron(q1, q2, q3, q4)) 
		{
			aBuf[0] = aBuf[4];
			bBuf[0] = bBuf[4];
		} 
		else 
		{
			// Origin not in initial polytope

			return false;
		}
	    
		num_verts = 4;
		break;

	}
	case 3: 
		{
			// We have a triangle inside the Minkowski sum containing
			// the origin. First blow it up.
		    
			const HVec3 q0(aBuf[0]-bBuf[0]);
			const HVec3 q1(aBuf[1]-bBuf[1]);
			const HVec3 q2(aBuf[2]-bBuf[2]);

			const HVec3 v1(q1 - q0);
			const HVec3 v2(q2 - q0);
			const HVec3 vv(v1^v2);
		   
			aBuf[3] = m_EpaManager.SupportMargin(&a, vv, marginA);
			bBuf[3] = m_EpaManager.SupportMargin(&b, -vv, marginB);
			const HVec3 q3(aBuf[3] - bBuf[3]);

			aBuf[4] = m_EpaManager.SupportMargin(&a, -vv, marginA);
			bBuf[4] = m_EpaManager.SupportMargin(&b, vv, marginB);
			const HVec3 q4(aBuf[4] - bBuf[4]);
		  
		    
		   
			if (originInTetrahedron(q0, q1, q2, q4)) 
			{
				aBuf[3] = aBuf[4];
				bBuf[3] = bBuf[4];
			}
			else if (!originInTetrahedron(q0, q1, q2, q3))
			{ 
				// Origin not in initial polytope
				return false;
			}
		    
			num_verts = 4;
		    
			break;
		}
	}

	
    FacetSimplified *f0 = addFacet(0, 1, 2, zero, _max);
    FacetSimplified *f1 = addFacet(0, 3, 1, zero, _max);
    FacetSimplified *f2 = addFacet(0, 2, 3, zero, _max);
    FacetSimplified *f3 = addFacet(1, 3, 2, zero, _max);
	
	
	/*if(!f0 || f0->getDist() < epsilon ||!f1 || f1->getDist() < epsilon||!f2 || f2->getDist() < epsilon ||!f3 || f3->getDist() < epsilon)
	{
		return false;
	}*/

	if(heap.isEmpty())
		return false;

	FacetSimplified f;
	FacetSimplified* facet = &f;

	
   // HF32 upper_bound2(_max);
	HF32 upper_bound2(_max);
	/*const HF32 ee(one + epsilon);
	const HF32 e(ee * ee);*/
	const HF32 eps(Constants<HF32>::Epsilon());

	//const HF32 eps2(0.000001f);
	//Facet2* facet = heap.getTop();
	//if(facet->getDist() < eps2)//the closest point is the origin itself
	//{
	//	const Facet2* adjFacet0 = facet->m_adjFacets[0];
	//	const U32 edgeStart = facet->m_adjEdges[0];
	//	const U32 edgeEnd = incMod3(edgeStart);
	//	const U32 num = 3 - edgeStart - edgeEnd;
	//	const U32 index = adjFacet0->m_indices[num];
	//	const HVec3 closest = -(aBuf[index] - bBuf[index]);
	//	facet->setSupportDir(closest);
	//}


    do 
	{
		Spu::MemBaseMarker innerMarker;
		heap.deleteTop(f); //get the shortest distance triangle of origin from the list

		const HF32 sFacetDist(facet->getDist());

		BIZ_ASSERT(sFacetDist > zero);
		
		if (num_verts == MaxSupportPoints)	
			break;

		const HVec3 dir(facet->getSupportDir());

		const HVec3 tempa(m_EpaManager.SupportMargin(&a, dir, marginA));
		const HVec3 tempb(m_EpaManager.SupportMargin(&b, -dir, marginB));

		aBuf[num_verts]=tempa;
		bBuf[num_verts]=tempb;
		const HVec3 q(tempa - tempb);
		
		U32 index =num_verts++;

		//if the support point and closest point close enough, found the contact point, break
		//HF32 dist(qBuf[index]|closest);
		const HF32 u(q|dir);
		const HF32 su(u*u);
		upper_bound2 = Min(upper_bound2, su);
		
		const HF32 err(eps + Sqrt(sFacetDist));
		const HF32 err2(err*err);
		if(upper_bound2 <= err2)
		{
			break;
		}

		addFacet((*facet)[0], (*facet)[1], index, sFacetDist, upper_bound2);
		addFacet((*facet)[1], (*facet)[2], index, sFacetDist, upper_bound2);
		addFacet((*facet)[2], index, (*facet)[0], sFacetDist, upper_bound2);


		/*const HF32 u(q|dir);
		upper_bound = Min(upper_bound, u);
		
		const HF32 dif = upper_bound - facetDist;
		const HF32 dif2 = dif*dif;
		if(dif2 <= eps)
		{
			break;
		}

		addFacet((*facet)[0], (*facet)[1], index, facetDist, upper_bound);
		addFacet((*facet)[1], (*facet)[2], index, facetDist, upper_bound);
		addFacet((*facet)[2], index, (*facet)[0], facetDist, upper_bound);*/

    }
	while ((--num_facets) && U32(heap.getTop().getDist() <= upper_bound2));


	facet->getClosestPoint(aBuf, bBuf, pa, pb);
	
	
    return true;
}
#endif

}





