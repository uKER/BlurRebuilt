#ifndef EPA_INL
#define EPA_INL

//#include "gravity/narrowphase/EPA.h"
#include "gravity/sharedphysics/Select.h"

namespace Collision
{
//-------------------------------------Simplifiled EPA version------------------------------------------//
#ifndef PLATFORM_SPU
template <class ConvexA, class ConvexB>
bool EPASimplified::PenetrationDepth(const ConvexA &a, const ConvexB &b, const Maths::HVec3 *__restrict Q, const Maths::HVec3 *__restrict A, const Maths::HVec3 *__restrict B, const U8& size, Maths::HVec3 &pa, Maths::HVec3 &pb)
{

	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 two(Constants<HF32>::Two());
	const HF32 _max(Constants<HF32>::Max());
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
		//C_Normal dir (qBuf[1] - qBuf[0]);
	   // HVec3 abs(Abs(dir));
		const HVec3 q0(aBuf[0]-bBuf[0]);
		const HVec3 q1(aBuf[1]-bBuf[1]);
		C_Normal dir(q1-q0);
		
		const Maths::HF32 sDir(dir|dir);
		const Maths::C_Normal temp2(sDir, sDir, sDir);
		const Maths::C_Normal t1(Normalise(temp2 ^ dir));
	    
		const C_Normal aux1(dir^t1, C_Normal::UNIT_LENGTH_VECTOR);
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

		const HF32 sinHalfTheta1 = -sinHalfTheta0;
		const HF32 cosHalfTheta1 = cosHalfTheta0;
		const HVec3 imag1 = dir * sinHalfTheta1;
		C_Quaternion qua1(imag1.X(), imag1.Y(), imag1.Z(), cosHalfTheta1);
		
		const C_Normal aux3(aux1 * qua1);
		aBuf[4] = a.Support(aux3);
		bBuf[4] = b.Support(-aux3);
		const HVec3 q4(aBuf[4]-bBuf[4]);

		//const HVec3 zero(Constants<Maths::HVec3>::Zero());
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(zero, aux1, 1.f, 0.25f, RGBA(255, 0, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(zero, aux2, 1.f, 0.25f, RGBA(0, 255, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(zero, aux3, 1.f, 0.25f, RGBA(0, 0, 255));

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(q0, q2, q3, RGBA(0, 255, 255));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(q0, q2, q4, RGBA(0, 255, 255));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(q0, q3, q4, RGBA(0, 255, 255));

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(q2, q3, q4, RGBA(0, 255, 0));

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(q1, q2, q3, RGBA(255, 255, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(q1, q2, q4, RGBA(255, 255, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(q1, q3, q4, RGBA(255, 255, 0));
		//

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q0, "q0");
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q1, "q1");
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q2, "q2");
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q3, "q3");
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q4, "q4");
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(zero, 0.5f, RGBA(255, 0, 0));

	    
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

	FacetSimplified* f0 = addFacet(0, 1, 2, zero, _max);
	FacetSimplified* f1 = addFacet(0, 3, 1, zero, _max);
	FacetSimplified* f2 = addFacet(0, 2, 3, zero, _max);
	FacetSimplified* f3 = addFacet(1, 3, 2, zero, _max);
	if(!f0 || !f1 || !f2 || !f3 || heap.isEmpty())
		return false;

	
    HF32 upper_bound2(_max);
	/*const HF32 ee(one + epsilon);
	const HF32 e(ee * ee);*/
	const HF32 eps(Constants<HF32>::Epsilon());
	

	FacetSimplified* facet= &heap.getTop();
	if(facet->getDist() < eps2)//the closest point is the origin itself
	{
		const U32 sum = facet->m_indices[0] + facet->m_indices[1] + facet->m_indices[2];
		const U32 index = 6 - sum;
		const HVec3 dir = -(aBuf[index] - bBuf[index]);
		const HVec3 tempa(a.Support(dir));
		const HVec3 tempb(b.Support(-dir));
		const HVec3 q(tempa - tempb);
		const HF32 dist(q|facet->getClosest());
		facet->setSupportDir(Maths::Select(dist.GT(zero), dir, facet->getSupportDir()));
	}


    do 
	{
		heap.deleteTop((*facet)); //get the shortest distance triangle of origin from the list

		const HF32 sqFacetDist(facet->getDist());

		//assert(sFacetDist > zero);
		
		if (num_verts == MaxSupportPoints)	
			break;

		const HVec3 closest = facet->getClosest();
		const HVec3 dir(facet->getSupportDir());

		const HVec3 tempa(a.Support(dir));
		const HVec3 tempb(b.Support(-dir));

		aBuf[num_verts]=tempa;
		bBuf[num_verts]=tempb;
		const HVec3 q(tempa - tempb);
		
		U32 index =num_verts++;



		//if the support point and closest point close enough, found the contact point, break
		//HF32 dist(qBuf[index]|closest);
		const HF32 u(q|closest);
		const HF32 sqU(u*u);
		//assert(u.GT(zero));

		upper_bound2 = Min(upper_bound2, sqU);
		
		//const HF32 err(eps + Sqrt(sqFacetDist));
		//const HF32 err2(err*err);
		const HF32 err2(sqFacetDist + eps);
		if(upper_bound2 <= err2)
		{
			break;
		}

		addFacet((*facet)[0], (*facet)[1], index, sqFacetDist, upper_bound2);
		addFacet((*facet)[1], (*facet)[2], index, sqFacetDist, upper_bound2);
		addFacet((*facet)[2], index, (*facet)[0], sqFacetDist, upper_bound2);
    }
	while ((--num_facets) && U32(heap.getTop().getDist() <= upper_bound2));

	facet->getClosestPoint(aBuf, bBuf, pa, pb);
	
	
    return true;
}


template <class ConvexA, class ConvexB>
bool EPASimplified::PenetrationDepthMargin(const ConvexA& a, const ConvexB& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8 PCREF size,  Maths::HVec3& pa, Maths::HVec3& pb, const Maths::HF32 PCREF marginA, const Maths::HF32 PCREF marginB)
{

	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 two(Constants<HF32>::Two());
	const HF32 _max(Constants<HF32>::Max());
	//const HF32 epsilon(Maths::Constants<HF32>::Epsilon2());
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
		
		const Maths::HF32 sDir(dir|dir);
		const Maths::C_Normal temp2(sDir, sDir, sDir);
		const Maths::C_Normal t1(Normalise(temp2 ^ dir));
	    
		const C_Normal aux1(dir^t1, C_Normal::UNIT_LENGTH_VECTOR);
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

		const HF32 sinHalfTheta1 = -sinHalfTheta0;
		const HF32 cosHalfTheta1 = cosHalfTheta0;
		const HVec3 imag1 = dir * sinHalfTheta1;
		C_Quaternion qua1(imag1.X(), imag1.Y(), imag1.Z(), cosHalfTheta1);
		
		const C_Normal aux3(aux1 * qua1);
		aBuf[4] = a.SupportMargin(aux3, marginA);
		bBuf[4] = b.SupportMargin(-aux3, marginB);
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

	
    FacetSimplified* f0 = addFacet(0, 1, 2, zero, _max);
	FacetSimplified* f1 = addFacet(0, 3, 1, zero, _max);
	FacetSimplified* f2 = addFacet(0, 2, 3, zero, _max);
	FacetSimplified* f3 = addFacet(1, 3, 2, zero, _max);
	if(!f0 || !f1 || !f2 || !f3 || heap.isEmpty())
		return false;

	
    HF32 upper_bound2(_max);
	/*const HF32 ee(one + epsilon);
	const HF32 e(ee * ee);*/
	
	const HF32 eps(Constants<HF32>::Epsilon());
	FacetSimplified* facet= &heap.getTop();
	if(facet->getDist() < eps2)//the closest point is the origin itself
	{
		const U32 sum = facet->m_indices[0] + facet->m_indices[1] + facet->m_indices[2];
		const U32 index = 6 - sum;
		const HVec3 dir = -(aBuf[index] - bBuf[index]);
		const HVec3 tempa(a.Support(dir));
		const HVec3 tempb(b.Support(-dir));
		const HVec3 q(tempa - tempb);
		const HF32 dist(q|facet->getClosest());
		facet->setSupportDir(Maths::Select(dist.GT(zero), dir, facet->getSupportDir()));
	}


    do 
	{
		heap.deleteTop(*facet); //get the shortest distance triangle of origin from the list

		const HF32 sFacetDist(facet->getDist());

		//assert(sFacetDist > zero);
		
		if (num_verts == MaxSupportPoints)	
			break;


		/*const HVec3 closest = facet->m_closest;
		const HF32 distClosest = closest.Length();
		const HF32 sDistClosest = distClosest*distClosest;
		const HVec3 nor=C_Normal(closest);

		const HVec3 temp1(a.Support(nor));
		const HVec3 temp2(b.Support(-nor));*/

		const HVec3 dir(facet->getSupportDir());
		const HVec3 closest(facet->getClosest());

		const HVec3 tempa(a.SupportMargin(dir, marginA));
		const HVec3 tempb(b.SupportMargin(-dir, marginB));

		aBuf[num_verts]=tempa;
		bBuf[num_verts]=tempb;
		const HVec3 q(tempa - tempb);
		
		const U32 index =num_verts++;

		//if the support point and closest point close enough, found the contact point, break
		//HF32 dist(qBuf[index]|closest);
		HF32 u(q|closest);
		const HF32 su(u*u);
		//assert(u.GT(zero));

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

    }
	while ((--num_facets) && U32(heap.getTop().getDist() <= upper_bound2));


	facet->getClosestPoint(aBuf, bBuf, pa, pb);
	
	
    return true;
}


#endif

}

#endif

