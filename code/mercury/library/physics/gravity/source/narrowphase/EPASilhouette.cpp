#include "StdAfx.h"
#include "gravity/narrowphase/EPA.h"
#include "gravity/sharedphysics/Select.h"

namespace Collision
{

	/*void EPASilhouette::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF d)
	{
		using namespace Maths;
	
		const HVec3 origin(Constants<HVec3>::Zero());

		pRender->Draw3dTriangleOutline(a, b, c, RGBA(255, 0,0));
		pRender->Draw3dTriangleOutline(a, b, d, RGBA(255, 0,0));
		pRender->Draw3dTriangleOutline(a, c, d, RGBA(255, 0,0));
		pRender->Draw3dTriangleOutline(b, c, d, RGBA(255, 0,0));
		pRender->Draw3dText(a, "a");
		pRender->Draw3dText(b, "b");
		pRender->Draw3dText(c, "c");
		pRender->Draw3dText(d, "d");
		pRender->Draw3dStar(origin, 0.5f, RGBA(0, 255, 0));
		pRender->Draw3dText(origin, "origin");
	}*/

#ifdef PLATFORM_SPU

	bool EPASilhouette::PenetrationDepth(const C_Convex& a, const C_Convex& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8& size,  Maths::HVec3& pa, Maths::HVec3& pb)
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

			const HF32 sinHalfTheta1 = -sinHalfTheta0;//sin(120/2)
			const HF32 cosHalfTheta1 = cosHalfTheta0;
			const HVec3 imag1 = dir * sinHalfTheta1;
			C_Quaternion qua1(imag1.X(), imag1.Y(), imag1.Z(), cosHalfTheta1);
			
			const HVec3 aux3(aux1 * qua1);
			aBuf[4] = m_EpaManager.Support(&a, aux3);
			bBuf[4] = m_EpaManager.Support(&b, -aux3);
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
		    
			aBuf[3] = m_EpaManager.Support(&a, vv);
			bBuf[3] = m_EpaManager.Support(&b, -vv);
			const HVec3 q3(aBuf[3] - bBuf[3]);

			aBuf[4] = m_EpaManager.Support(&a, -vv);
			bBuf[4] = m_EpaManager.Support(&b, vv);
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
			const HVec3 tempa(m_EpaManager.Support(&a, dir));
			const HVec3 tempb(m_EpaManager.Support(&b, -dir));
			const HVec3 q(tempa - tempb);
			const HF32 dist(q|facet->getClosest());
			facet->setDir(Maths::Select(dist.GT(zero), dir, facet->getDir()));
		}
		
		
		do 
		{
			heap.deleteTop(&facet); //get the shortest distance triangle of origin from the list
			
			if (!facet->isObsolete()) 
			{
				if (num_verts == MaxSupportPoints)	
					break;

				const HF32 sqDist = facet->getDist();
				BIZ_ASSERT(sqDist > zero);
				const HVec3 closest(facet->getClosest());
				const HVec3 dir(facet->getClosest());

				const HVec3 tempa(m_EpaManager.Support(&a, dir));
				const HVec3 tempb(m_EpaManager.Support(&b, -dir));


				aBuf[num_verts]=tempa;
				bBuf[num_verts]=tempb;
				const HVec3 q(tempa - tempb);
				
				
				const U32 index =num_verts++;
				
				//if the support point and closest point close enough, found the contact point, break
				const HF32 dist(q|closest);
				
			
				/*const HF32 er(ee * sfDist);
				upper_bound2=Min(upper_bound2, (dist * dist)/sqDist);
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

	bool EPASilhouette::PenetrationDepthMargin(const C_Convex& a, const C_Convex& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8& size,  Maths::HVec3& pa, Maths::HVec3& pb, const Maths::HF32 PCREF marginA, const Maths::HF32 PCREF marginB)
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

			const HF32 sinHalfTheta1 = -sinHalfTheta0;//sin(120/2)
			const HF32 cosHalfTheta1 = cosHalfTheta0;
			const HVec3 imag1 = dir * sinHalfTheta1;
			C_Quaternion qua1(imag1.X(), imag1.Y(), imag1.Z(), cosHalfTheta1);
			
			const HVec3 aux3(aux1 * qua1);
			aBuf[4] = m_EpaManager.SupportMargin(&a, aux3, marginA);
			bBuf[4] = m_EpaManager.SupportMargin(&b, -aux3, marginB);
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
		    
			aBuf[3] = m_EpaManager.SupportMargin(&a, vv, marginA);
			bBuf[3] = m_EpaManager.SupportMargin(&b, -vv, marginB);
			const HVec3 q3(aBuf[3] - bBuf[3]);

			aBuf[4] = m_EpaManager.SupportMargin(&a, -vv, marginA);
			bBuf[4] = m_EpaManager.SupportMargin(&b, vv, marginB);
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
			const HVec3 tempa(m_EpaManager.SupportMargin(&a, dir, marginA));
			const HVec3 tempb(m_EpaManager.SupportMargin(&b, -dir, marginB));
			const HVec3 q(tempa - tempb);
			const HF32 dist(q|facet->getClosest());
			facet->setDir(Maths::Select(dist.GT(zero), dir, facet->getDir()));
		}

		do 
		{
			heap.deleteTop(&facet); //get the shortest distance triangle of origin from the list
			
			if (!facet->isObsolete()) 
			{
				if (num_verts == MaxSupportPoints)	
					break;

				const HF32 sqDist = facet->getDist();
				BIZ_ASSERT(sqDist > zero);
				const HVec3 closest(facet->getClosest());
				const HVec3 dir(facet->getDir());

				const HVec3 tempa(m_EpaManager.SupportMargin(&a, dir, marginA));
				const HVec3 tempb(m_EpaManager.SupportMargin(&b, -dir, marginB));


				aBuf[num_verts]=tempa;
				bBuf[num_verts]=tempb;
				const HVec3 q(tempa - tempb);
				
				
				U32 index =num_verts++;
				
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
				
				//assert(!edgeBuffer.IsEmpty());
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