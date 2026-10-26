#ifndef CONVEXTRIANGLE_INL
#define CONVEXTRIANGLE_INL

#include "gravity/narrowphase/ConvexHull.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/GJK.inl"
//#include "gravity/narrowphase/ConeTriangle.inl"

namespace Collision
{

	
	template <int MaxContacts>
	bool CollideConvexHullTriangle(C_ConvexHull& convex, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Collision::C_ContactPatch<MaxContacts>* pPatch, const U32 physicsMaterial, const U32 effectMaterial)
	{
		using namespace Maths;
		bool collide=false;

		Spu::MemBaseMarker marker;
		C_SpuConvexHull spuConvex(convex, true);
		const HMat33& mat(spuConvex.Orientation());

		const U32 numTriangles = convex.GetNumTriangles();

		for(U32 i=0; i<numTriangles; ++i)
		{
			if(pPatch->m_NumContactPoints >= MaxContacts)
				break;
			Spu::MemBaseMarker innerMarker;
			const C_ConvexTriangle* t = &spuConvex.GetTriangle(i);

			const HVec3 vA(spuConvex.GetWorldVertex(t->m_VertexA));
			const HVec3 vB(spuConvex.GetWorldVertex(t->m_VertexB));
			const HVec3 vC(spuConvex.GetWorldVertex(t->m_VertexC));
	
			const C_Normal normal(Maths::Load<Maths::HVec3>(&t->m_Normal) * mat, C_Normal::UNIT_LENGTH_VECTOR);

			const U32 numContacts = pPatch->m_NumContactPoints;

			collide = Collision::CollideTriangleTriangle(a, b, c, vA, vB, vC, pPatch->m_Normal, normal, *pPatch, 0, physicsMaterial, effectMaterial)|| collide;
		}
		return collide;

	}

	template <int MaxPatches, int MaxContacts, class Triangle>
	bool CollideConvexHullTriangleMesh( C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
	{
		using namespace Maths;
		Spu::MemBaseMarker _marker;
		//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
		//C_ConvexHullShape* pHullShape = (C_ConvexHullShape*)Spu::FetchAlloc(pShape, sizeof(C_ConvexHullShape));
		C_ConvexHullShape* pHullShape = (C_ConvexHullShape*)pShape;
		C_ConvexHull& convex(pHullShape->m_Hull);
		C_LocalSpaceSpuConvexHull spuConvex(convex, true);
		const HMat33 mat(spuConvex.Orientation());
		const HVec3 centre(spuConvex.Centre());

		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		C_Triangle triangle;
		triangle.Margin(zero);
		triangle.Epsilon(spuConvex.Epsilon());
		const Maths::HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
		const Maths::HF32 cosThirty(SharedPhysics::Constants::CosThirty());
		bool bResult(false);
		HVec3 contact(Maths::UNDEFINED);
		HVec3 contact2(UNDEFINED);
		HVec3 norm(UNDEFINED);
		HF32 pen(UNDEFINED), pen1(Maths::UNDEFINED), pen2(Maths::UNDEFINED);
		const Maths::HF32 margin(convex.Margin());

		Maths::C_Normal resultNormal(Maths::UNDEFINED);
		const HF32 epsilon(Maths::Constants<HF32>::Epsilon());

		MathsStorage::HenDN3 dNorm;
		//const Maths::HVec3 centre(convex.Centre());
		const HVec3 range(high - low);

		for(U32 a = 0; a < numTris; ++a)
		{
			Spu::MemBaseMarker marker;

			const Triangle* tri = &tris[a];
			const HVec3 localA(&verts[tri->m_IndexA ]);
			const HVec3 localB(&verts[tri->m_IndexB ]);
			const HVec3 localC(&verts[tri->m_IndexC ]);
			triangle.m_VertexA = mat * (localA*range + low - spuConvex.m_Centre);
			triangle.m_VertexB = mat * (localB*range + low - spuConvex.m_Centre);
			triangle.m_VertexC = mat * (localC*range + low - spuConvex.m_Centre);

			dNorm = tri->m_SurfaceNormal;
			triangle.m_Normal = mat * (Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm));

			//Early exit - we're in local space of hull so centre is 0,0,0
			if((triangle.m_Normal | (triangle.m_VertexA)).GT(zero))
				continue;

			const Maths::C_Normal triNormal(triangle.m_Normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
			triangle.m_Extrusion = zero;
			triangle.WarmStart(triangle.m_VertexA);
			if(GJKCPair<C_LocalSpaceSpuConvexHull, C_Triangle>(spuConvex, triangle))
			{
				norm = -triangle.m_Normal;
				const Maths::HVec3 worldNormal(norm * mat);

				bResult = true;
				//Store this contact into the collision result...
				C_ContactPatch<MaxContacts>* pPatch(NULL);
				S32 incr = 1;
				if(res.m_NumContactPatches < MaxPatches)
				{
					pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
					pPatch->m_Normal = Maths::C_Normal(worldNormal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
					pPatch->m_NumContactPoints = 0;
				}

				for(U32 b = 0; b < res.m_NumContactPatches; ++b)
				{
					C_ContactPatch<MaxContacts>& patch(res.m_ContactPatches[b]);
					//const Maths::HF32 d(patch.m_Normal | negNormal);
					const Maths::HF32 d(patch.m_Normal | worldNormal);
					if(d >= closeNormal)
					{
						incr = 0;
						pPatch = &res.m_ContactPatches[b];
						break;
					}
				}
				if(pPatch == NULL)
					continue;
				//Stuff contact in there...
				if(pPatch->m_NumContactPoints < MaxContacts)
				{
					const U32 numContacts = pPatch->m_NumContactPoints;
					U32 numTriangles = convex.GetNumTriangles();
					for(U32 i=0; i<numTriangles; ++i)
					{
						if(pPatch->m_NumContactPoints >= MaxContacts)
							break;
						
						/*const C_ConvexTriangle* t = &spuConvex.m_pTriangles[i];
						const HVec3 vA(spuConvex.GetWorldVertex(t->m_VertexA));
						const HVec3 vB(spuConvex.GetWorldVertex(t->m_VertexB));
						const HVec3 vC(spuConvex.GetWorldVertex(t->m_VertexC));*/

						const C_ConvexTriangle* t = &spuConvex.GetTriangle(i);
						const HVec3 vA(spuConvex.GetLocalVertex(t->m_VertexA));
						const HVec3 vB(spuConvex.GetLocalVertex(t->m_VertexB));
						const HVec3 vC(spuConvex.GetLocalVertex(t->m_VertexC));


						/*const Maths::HVec3 vA(spuConvex.m_Position + spuConvex.GetVertex(t->m_VertexA) * spuConvex.m_Orientation);
						const Maths::HVec3 vB(spuConvex.m_Position + spuConvex.GetVertex(t->m_VertexB) * spuConvex.m_Orientation);
						const Maths::HVec3 vC(spuConvex.m_Position + spuConvex.GetVertex(t->m_VertexC) * spuConvex.m_Orientation);*/

						const C_Normal normal(Maths::Load<Maths::HVec3>(&t->m_Normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);


						if(Collision::TomasCollideTriangleTriangle(vA, vB, vC, triangle.m_VertexA, triangle.m_VertexB, 
							triangle.m_VertexC, normal, triNormal, contact, contact2, pen1, pen2))
						{
							const Maths::HF32 largestPen(Maths::Max(pen1, pen2));
							if(pPatch != NULL && pPatch->m_NumContactPoints < MaxContacts)
							{
								C_ContactPoint& point = pPatch->m_ContactPoints[pPatch->m_NumContactPoints++];
								point.m_ContactPoint = contact * mat + centre;
								Maths::Store(largestPen/*pen1*/, &point.m_PenetrationDepth);
								//point.m_PenetrationDepth = pen1;//pen1;//Maths::Min(pen1, pen);
								point.m_EffectMaterialIndices = effectMaterial;
								point.m_PhysicsMaterialIndices = physicsMaterial;
								point.m_HashValue = (triangleStartIndex + a) << 16 | i;
								if(pPatch->m_NumContactPoints < MaxContacts)
								{
									C_ContactPoint& point2 = pPatch->m_ContactPoints[pPatch->m_NumContactPoints++];
									point2.m_ContactPoint = contact2 * mat + centre;
									Maths::Store(largestPen/*pen2*/, &point2.m_PenetrationDepth);
									//point2.m_PenetrationDepth = pen2;//pen2;//Maths::Min(pen2, pen);
									point2.m_EffectMaterialIndices = effectMaterial;
									point2.m_PhysicsMaterialIndices = physicsMaterial;
									point2.m_HashValue = ((triangleStartIndex + a) << 16 | i)|1;
								}
							}
						}
					}
				}
				if(incr && pPatch->m_NumContactPoints > 0)
					++res.m_NumContactPatches;
				
			}
			
		}
		return bResult;
	}



	template <int MaxPatches, int MaxContacts, class Triangle>
	bool CollideConvexHullTriangleMesh(C_ConvexHullProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
	{

		using namespace Maths;
		//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
		Spu::MemBaseMarker _outerMarker;
		C_LocalSpaceSpuConvexHull& spuConvex(proxy.m_Hull);
		const HMat33 mat(spuConvex.Orientation());
		const Maths::HVec3 centre(spuConvex.Centre());

		//C_SpuConvexHull spuConvex(proxy.m_Hull, true);
		const HMat33 orientation(proxy.m_PosRot.Rotation());
		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		
		C_Triangle triangle;
		triangle.Margin(zero);
		triangle.Epsilon(spuConvex.Epsilon());
		const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
		bool bResult(false);
		HVec3 contact(Maths::UNDEFINED), contact2(Maths::UNDEFINED);
		HVec3 norm(Maths::UNDEFINED);
		HF32 pen(Maths::UNDEFINED), pen1(Maths::UNDEFINED), pen2(Maths::UNDEFINED);
		const HF32 margin(spuConvex.Margin());
		
		MathsStorage::HenDN3 dNorm;
		//const Maths::HVec3 centre(convex.Centre());
		const HVec3 range(high - low);
		for(U32 a = 0; a < numTris; ++a)
		{
			//Spu::MemBaseMarker marker;
			
			const Triangle* tri = &tris[a];
			
			const HVec3 localA(&verts[tri->m_IndexA ]);
			triangle.m_VertexA = mat * (localA*range + low - centre);
			//((U32&)dNorm) = Spu::Get((U32*)normals + tri->m_NormalIndex);
			dNorm = tri->m_SurfaceNormal;
			const Maths::HVec3 modelNorm = (Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm));
			triangle.m_Normal = mat * modelNorm;
			const Maths::C_Normal triNormal(triangle.m_Normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
			
			const Maths::HF32 dotA(triangle.m_VertexA | triangle.m_Normal);
			//const Maths::HF32 dotCentre((spuConvex.Centre() | triangle.m_Normal) - dotA);
			/*if((dotCentre) > zero)
			{
				continue;	
			}*/

			if(dotA > zero)
			{
				continue;	
			}

			const HVec3 localB(&verts[tri->m_IndexB ]);
			const HVec3 localC(&verts[tri->m_IndexC ]);
			triangle.m_VertexB = mat * (localB*range + low - centre);
			triangle.m_VertexC = mat * (localC*range + low - centre);
			triangle.m_Extrusion = zero;
			
			triangle.WarmStart(triangle.m_VertexA);
	
			if(GJKCPair<C_LocalSpaceSpuConvexHull, C_Triangle>(spuConvex, triangle))
			{
				/*if((norm | triangle.m_Normal) < zero)
					continue;*/
				norm = (-modelNorm) * orientation;
				bResult = true;
				//Store this contact into the collision result...
				C_ContactPatch<MaxContacts>* pPatch(NULL);
				S32 incr = 1;
				if(res.m_NumContactPatches < MaxPatches)
				{
					pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
					pPatch->m_Normal = Maths::C_Normal(norm, Maths::C_Normal::UNIT_LENGTH_VECTOR);
					pPatch->m_NumContactPoints = 0;
				}

				for(U32 b = 0; b < res.m_NumContactPatches; ++b)
				{
					C_ContactPatch<MaxContacts>& patch(res.m_ContactPatches[b]);
					const Maths::HF32 d(patch.m_Normal.m_V | norm);
					if(d >= closeNormal)
					{
						incr = 0;
						pPatch = &res.m_ContactPatches[b];
						break;
					}
				}
				if(pPatch == NULL)
					continue;
				res.m_NumContactPatches += incr;
				//Stuff contact in there...
				if(pPatch->m_NumContactPoints < MaxContacts)
				{
					/*for(U32 i=0; i<convex.m_NumTriangles; ++i)*/
					const U32 numTriangles = spuConvex.GetNumTriangles();
					for(U32 i=0; i<numTriangles; ++i)
					{
						if(pPatch->m_NumContactPoints >= MaxContacts)
							continue;
			
						const C_ConvexTriangle* t = &spuConvex.GetTriangle(i);
						const HVec3 vA(spuConvex.GetLocalVertex(t->m_VertexA));
						const HVec3 vB(spuConvex.GetLocalVertex(t->m_VertexB));
						const HVec3 vC(spuConvex.GetLocalVertex(t->m_VertexC));

						/*const Maths::HVec3 vA(spuConvex.m_Position + spuConvex.GetVertex(t->m_VertexA) * spuConvex.m_Orientation);
						const Maths::HVec3 vB(spuConvex.m_Position + spuConvex.GetVertex(t->m_VertexB) * spuConvex.m_Orientation);
						const Maths::HVec3 vC(spuConvex.m_Position + spuConvex.GetVertex(t->m_VertexC) * spuConvex.m_Orientation);*/

						const Maths::C_Normal normal(Maths::Load<Maths::HVec3>(&t->m_Normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);

						if(Collision::TomasCollideTriangleTriangle(vA, vB, vC, triangle.m_VertexA, triangle.m_VertexB, 
							triangle.m_VertexC, normal, triNormal, contact, contact2, pen1, pen2))
						{
							const Maths::HF32 maxPen(Maths::Max(pen1, pen2));
							if(pPatch != NULL && pPatch->m_NumContactPoints < MaxContacts)
							{
								C_ContactPoint& point = pPatch->m_ContactPoints[pPatch->m_NumContactPoints++];
								point.m_ContactPoint = (contact * mat + centre) * proxy.m_PosRot;
								//point.m_PenetrationDepth = pen1;
								Maths::Store(maxPen/*pen1*/, &point.m_PenetrationDepth);
								point.m_EffectMaterialIndices = effectMaterial;
								point.m_PhysicsMaterialIndices = physicsMaterial;
								point.m_HashValue = (triangleStartIndex + a) << 16 | i;
								if(pPatch->m_NumContactPoints < MaxContacts)
								{
									C_ContactPoint& point2 = pPatch->m_ContactPoints[pPatch->m_NumContactPoints++];
									point2.m_ContactPoint = (contact2 * mat + centre) * proxy.m_PosRot;
									//point2.m_PenetrationDepth = pen2;
									Maths::Store(maxPen/*pen2*/, &point2.m_PenetrationDepth);
									point2.m_EffectMaterialIndices = effectMaterial;
									point2.m_PhysicsMaterialIndices = physicsMaterial;
									point2.m_HashValue = ((triangleStartIndex + a) << 16 | i)|1;
								}
							}
						}
					}
				}
			}
			
		}
		return bResult;
	}

	template <class Triangle>
	bool ContinuousConvexHullTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex,
		const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, const Maths::HVec3& low, const Maths::HVec3& high)
	{
		using namespace Maths;
		Spu::MemBaseMarker marker;

		C_ContinuousShape* pContinuousShape = (C_ContinuousShape*)Spu::FetchAlloc(pShape, sizeof(C_ContinuousShape));
		C_ConvexHullShape* pHullShape = (C_ConvexHullShape*)Spu::FetchAlloc(pContinuousShape->m_pContinuousPrimitive, sizeof(C_ConvexHullShape));
		C_ConvexHull& convex(pHullShape->m_Hull);
		C_SpuConvexHull spuConvex(convex);

		const HF32 zero(Constants<HF32>::Zero());
		C_Triangle triangle;
		triangle.Margin(zero);
		triangle.Epsilon(convex.Epsilon());

		bool bResult(false);
		const HF32 margin(convex.Margin());

		MathsStorage::HenDN3 dNorm;

		HVec3 normal(UNDEFINED);
		const HVec3 range(high - low);

		for(U32 a = 0; a < numTris; ++a)
		{
			//Spu::MemBaseMarker marker;
			const Triangle* tri = &tris[a];
			
			dNorm = tri->m_SurfaceNormal;
			HVec3 normal = -Load<HVec3, MathsStorage::HenDN3>(&dNorm);

			const HVec3 localA(&verts[tri->m_IndexA ]);
			const HVec3 localB(&verts[tri->m_IndexB ]);
			const HVec3 localC(&verts[tri->m_IndexC ]);

			triangle.m_VertexA = localA*range + low;
			triangle.m_VertexB = localB*range + low;
			triangle.m_VertexC = localC*range + low;
			//lambda = zero;
			//if(GJKCPairMargin<C_ConvexHull, C_Triangle>(convex, triangle , contact, normal, pen, *res.m_pEpa))
			if(Collision::GJKRayCast<C_SpuConvexHull, C_Triangle>(spuConvex, triangle , r, maxLambda, lambda, nor, closestA, closestB))
			{
				nor = normal;
				maxLambda = lambda;
				bResult = true;
			}
			
		}
		return bResult;
	}

									//------------------Capsule----------------------------------
	//template <int MaxPatches, int MaxContacts, class Triangle>
	//bool CollideConvexCapsuleTriangleMesh(C_CapsuleProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
	//	const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
	//	const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
	//{
	//	using namespace Maths;
	//	C_Capsule& convex(proxy.m_Capsule);
	//	//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
	//	const HF32 zero(Constants<HF32>::Zero());
	//	//const Maths::HF32 third(Maths::Constants<Maths::HF32>::Third());
	//	const HF32 twenty(SharedPhysics::Constants::Twenty());
	//	C_Triangle triangle;
	//	triangle.Margin(zero);//convex.Margin());
	//	const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
	//	bool bResult(false);
	//	HVec3 contact(UNDEFINED);
	//	HVec3 normal(UNDEFINED);
	//	HF32 pen(Maths::UNDEFINED);
	//	const HF32 margin(convex.Margin());
	//	MathsStorage::HenDN3 dNorm;

	//	const HMat33 orientation(proxy.m_PosRot.Rotation());
	//	//const Maths::HVec3 centre(convex.Centre());
	//	const HVec3 range(high - low);
	//	for(U32 a = 0; a < numTris; ++a)
	//	{
	//		//Spu::MemBaseMarker marker;
	//		//const Triangle* tri((Triangle*)Spu::FetchAllocUnaligned(&tris[a], sizeof(Triangle)));
	//		const Triangle* tri =&tris[a];
	//		const HVec3 localA(&verts[tri->m_IndexA ]);
	//		const HVec3 localB(&verts[tri->m_IndexB ]);
	//		const HVec3 localC(&verts[tri->m_IndexC ]);
	//		triangle.m_VertexA = localA*range + low;
	//		triangle.m_VertexB = localB*range + low;
	//		triangle.m_VertexC = localC*range + low;
	//		dNorm = tri->m_SurfaceNormal;
	//		triangle.m_Normal = (-Maths::Load<Maths::HVec3>(&dNorm));

	//		const HF32 dotA(triangle.m_VertexA | triangle.m_Normal);
	//		
	//		const HVec3 supportNow(convex.SupportMargin(triangle.m_Normal, margin));
	//		const HF32 dotNow(((supportNow |triangle.m_Normal) + margin) - dotA);

	//		//triangle.m_Extruded = ((triangle.m_VertexA + triangle.m_VertexB + triangle.m_VertexC) * third) + 
	//		//	triangle.m_Normal * Maths::Constants<Maths::HF32>::Twenty();

	//		triangle.m_Extrusion = zero;
	//		
	//		triangle.WarmStart(triangle.m_VertexA);
	//		//triangle.Margin(convex.Margin());

	//		if(GJKCPairMargin<C_Capsule, C_Triangle>(convex, triangle , contact, normal, pen, *res.m_pEpa))
	//		{
	//			bResult = true;
	//			//Store this contact into the collision result...
	//			C_ContactPatch<MaxContacts>* pPatch(NULL);
	//			S32 incr = 1;

	//			const Maths::C_Normal norm(triangle.m_Normal * orientation, Maths::C_Normal::UNIT_LENGTH_VECTOR);

	//			if(res.m_NumContactPatches < MaxPatches)
	//			{
	//				pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
	//				pPatch->m_Normal = norm;//Maths::C_Normal(triangle.m_Normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	//				pPatch->m_NumContactPoints = 0;
	//			}

	//			for(U32 b = 0; b < res.m_NumContactPatches; ++b)
	//			{
	//				C_ContactPatch<MaxContacts>& patch(res.m_ContactPatches[b]);
	//				const Maths::HF32 d(patch.m_Normal.m_V | norm);
	//				if(d >= closeNormal)
	//				{
	//					incr = 0;
	//					pPatch = &res.m_ContactPatches[b];
	//					break;
	//				}
	//			}
	//			if(pPatch == NULL)
	//				continue;
	//			res.m_NumContactPatches += incr;
	//			//Stuff contact in there...
	//			if(pPatch->m_NumContactPoints < MaxContacts)
	//			{
	//				C_ContactPoint& point(pPatch->m_ContactPoints[pPatch->m_NumContactPoints++]);
	//				point.m_ContactPoint = contact * proxy.m_PosRot;
	//				const Maths::HF32 norDir(normal | triangle.m_Normal);
	//				//point.m_PenetrationDepth = Maths::Abs(norDir * pen);
	//				//point.m_PenetrationDepth = Maths::Max(dotNow, zero);
	//				//point.m_PenetrationDepth = Maths::Max((normal | -triangle.m_Normal) * pen, zero);
	//				const Maths::HF32 pDepth = Maths::Max(Maths::Max((normal | -triangle.m_Normal) * pen, dotNow), zero);
	//				Maths::Store(pDepth, &point.m_PenetrationDepth);
	//				//point.m_PenetrationDepth = Maths::Max(Maths::Max((normal | -triangle.m_Normal) * pen, dotNow), zero);
	//				point.m_HashValue = triangleStartIndex + a;
	//				point.m_PhysicsMaterialIndices = physicsMaterial;
	//				point.m_EffectMaterialIndices = effectMaterial;
	//			}
	//		}
	//		
	//	}
	//	return bResult;
	//}

									//----------------------Cylinder------------------------------
	//template <int MaxPatches, int MaxContacts, class Triangle>
	//bool CollideConvexCylinderTriangleMesh(C_Shape* shape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
	//	const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
	//	const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
	//{
	//	using namespace Maths;
	//	C_CylinderShape* pShape((C_CylinderShape*)Spu::FetchAlloc(shape, sizeof(C_CylinderShape)));
	//	C_Cylinder& convex(pShape->m_Cylinder);
	//	//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
	//	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	//	//const Maths::HF32 third(Maths::Constants<Maths::HF32>::Third());
	//	const HF32 twenty(SharedPhysics::Constants::Twenty());
	//	C_Triangle triangle;
	//	triangle.Margin(zero);//convex.Margin());
	//	const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
	//	bool bResult(false);
	//	HVec3 contact(Maths::UNDEFINED);
	//	HVec3 normal(Maths::UNDEFINED);
	//	HF32 pen(Maths::UNDEFINED);
	//	const HF32 margin(convex.Margin());
	//	const HF32 disEps(SharedPhysics::Constants::CloseNormal());

	//	MathsStorage::HenDN3 dNorm;
	//	//const Maths::HVec3 centre(convex.Centre());
	//	const HVec3 range(high - low);
	//	for(U32 a = 0; a < numTris; ++a)
	//	{
	//		//Spu::MemBaseMarker marker;
	//		//const Triangle* tri((Triangle*)Spu::FetchAllocUnaligned(tris+a, sizeof(Triangle)));
	//		const Triangle* tri = &tris[a];

	//		const HVec3 localA(&verts[tri->m_IndexA ]);
	//		const HVec3 localB(&verts[tri->m_IndexB ]);
	//		const HVec3 localC(&verts[tri->m_IndexC ]);
	//		triangle.m_VertexA = localA*range + low;
	//		triangle.m_VertexB = localB*range + low;
	//		triangle.m_VertexC = localC*range + low;
	//		
	//		dNorm = tri->m_SurfaceNormal;
	//		triangle.m_Normal = (-Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm));
	//		const Maths::HF32 dotA(triangle.m_VertexA | triangle.m_Normal);

	//		const Maths::HF32 dotCentre((convex.Centre() | triangle.m_Normal) - dotA);
	//		if(dotCentre >= zero)
	//			continue;

	//		//triangle.m_Extrusion = Maths::Max(zero, dotNow * twenty);

	//		C_ContactPatch<MaxContacts>* pPatch(NULL);
	//		bool incr = false;
	//		if(res.m_NumContactPatches < MaxPatches)
	//		{
	//			pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
	//			pPatch->m_NumContactPoints = 0;
	//			pPatch->m_Normal = Maths::C_Normal(triangle.m_Normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	//			incr = true;
	//		}
	//		
	//		for(U32 j = 0; j < res.m_NumContactPatches; ++j)
	//		{
	//			Maths::HF32 dis(res.m_ContactPatches[j].m_Normal.m_V| triangle.m_Normal);
	//			if(dis >= disEps)
	//			{
	//				pPatch = &res.m_ContactPatches[j];
	//				incr = false;
	//				break;
	//			}
	//		}

	//		if(!pPatch)
	//			continue;

	//		
	//		
	//		const U32 numContacts = pPatch->m_NumContactPoints;

	//		 
	//		
	//		bool bContact = CollideCylinderTriangle(convex, triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, *pPatch, triangleStartIndex + a, physicsMaterial, effectMaterial);
	//		bResult = bResult || bContact;
	//		res.m_NumContactPatches += (incr && bContact);			
	//	}
	//	return bResult;

	//}

	//template <int MaxPatches, int MaxContacts, class Triangle>
	//bool CollideConvexCylinderTriangleMesh(C_CylinderProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
	//	const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
	//	const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
	//{
	//	using namespace Maths;

	//	const C_Cylinder& convex(proxy.m_Cylinder);
	//	//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
	//	const Maths::HF32 zero(Maths::Constants<HF32>::Zero());
	//	C_Triangle triangle;
	//	triangle.Margin(zero);//convex.Margin());
	//	const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
	//	bool bResult(false);
	//	Maths::HVec3 contact(UNDEFINED);
	//	Maths::HVec3 normal(UNDEFINED);
	//	Maths::HF32 pen(UNDEFINED);
	//	const HF32 margin(convex.Margin());
	//	const HF32 disEps(SharedPhysics::Constants::CloseNormal());

	//	//const Maths::HVec3 centre(convex.Centre());
	//	const HMat33 orientation(proxy.m_PosRot.Rotation());
	//	const HVec3 position(proxy.m_PosRot.Translation());
	//	const HF32 twenty(SharedPhysics::Constants::Twenty());

	//	MathsStorage::HenDN3 dNorm;
	//	const HVec3 range(high - low);
	//	for(U32 a = 0; a < numTris; ++a)
	//	{
	//		Spu::MemBaseMarker marker;
	//		//const Triangle* tempTri=(const Triangle*)Spu::FetchAllocUnaligned(tris + a, sizeof(Triangle));
	//		const Triangle* tempTri = &tris[a];
	//		//((U32&)dNorm) = Spu::Get( (U32*)normals + tempTri->m_NormalIndex);
	//		dNorm = tempTri->m_SurfaceNormal;

	//		const HVec3 localA(&verts[tempTri->m_IndexA ]);
	//		const HVec3 localB(&verts[tempTri->m_IndexB ]);
	//		const HVec3 localC(&verts[tempTri->m_IndexC ]);
	//		const HVec3 vertexA = localA*range + low;
	//		const HVec3 vertexB = localB*range + low;
	//		const HVec3 vertexC = localC*range + low;
	//		HVec3 normal = -Load<HVec3, MathsStorage::HenDN3>(&dNorm);

	//		const HF32 dotA(vertexA | normal);
	//		

	//		const Maths::HF32 dotCentre((convex.Centre() | normal) - dotA);
	//		if(dotCentre >= zero)
	//			continue;


	//		C_ContactPatch<MaxContacts>* pPatch(NULL);
	//		bool incr = false;
	//		if(res.m_NumContactPatches < MaxPatches)
	//		{
	//			pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
	//			pPatch->m_NumContactPoints = 0;
	//			pPatch->m_Normal = C_Normal(normal * orientation, C_Normal::UNIT_LENGTH_VECTOR);
	//			incr = true;
	//		}
	//		
	//		for(U32 j = 0; j < res.m_NumContactPatches; ++j)
	//		{
	//			HF32 dis(res.m_ContactPatches[j].m_Normal.m_V| normal);
	//			if(dis >= disEps)
	//			{
	//				pPatch = &res.m_ContactPatches[j];
	//				incr = false;
	//				break;
	//			}
	//		}

	//		if(!pPatch)
	//			continue;

	//		
	//		
	//		const U32 numContacts = pPatch->m_NumContactPoints;
	//		bool bContact = CollideCylinderTriangle(convex, vertexA, vertexB, vertexC, *pPatch, triangleStartIndex + a, physicsMaterial, effectMaterial);
	//		for(U32 c = numContacts; c < pPatch->m_NumContactPoints; ++c)
	//		{
	//			pPatch->m_ContactPoints[c].m_ContactPoint = pPatch->m_ContactPoints[c].m_ContactPoint * orientation + position;
	//		}
	//		bResult = bResult || bContact;
	//		res.m_NumContactPatches += (incr && bContact);		
	//	}
	//	return bResult;

	//}

};


#endif
