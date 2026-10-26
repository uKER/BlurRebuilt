#ifndef CONETRIANGLE_INL
#define CONETRIANGLE_INL

#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/ConeRay.inl"


namespace Collision
{
	
	//Point already inside the triangle and calculate the closest to the segment
	inline void ClosestPtPointTriangleSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& closest)
	{
		using namespace Maths;

		HVec3 closestA(UNDEFINED);
		HVec3 closestB(UNDEFINED);
		HVec3 closestC(UNDEFINED);

		Collision::ClosestPtPointSegment(p, a, b,  closestA);
		Collision::ClosestPtPointSegment(p, a, c,  closestB);
		Collision::ClosestPtPointSegment(p, b, c,  closestC);

		const HVec3 av( p - closestA );
		const HVec3 bv( p - closestB );
		const HVec3 cv( p - closestC );

		const HF32 sqDist1( av|av );
		const HF32 sqDist2( bv|bv );
		const HF32 sqDist3( cv|cv );

		const Maths::Scalar_Bool con1(sqDist1.LT(sqDist2) && sqDist1.LT(sqDist3));
		const Maths::Scalar_Bool con2(sqDist2.LT(sqDist3));

		closest = Select(con1, closestA, Select(con2, closestB, closestC));

	}


	/*inline void ClosestPtSegmentTriangleSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& closest)
	{
		using namespace Maths;

		HVec3 c1(UNDEFINED);
		HVec3 c2(UNDEFINED);
		HVec3 c3(UNDEFINED);
		HVec3 c4(UNDEFINED);
		HVec3 c5(UNDEFINED);
		HVec3 c6(UNDEFINED);

		Collision::ClosestPtSegmentSegment(p, q, a, b, c1, c2);
		Collision::ClosestPtSegmentSegment(p, q, a, c, c3, c4);
		Collision::ClosestPtSegmentSegment(p, q, b, c, c5, c6);

		const HVec3 av(c1 - c2);
		const HVec3 bv(c3 - c4);
		const HVec3 cv(c5 - c6);

		const HF32 sqDist1( av|av );
		const HF32 sqDist2( bv|bv );
		const HF32 sqDist3( cv|cv );

		const Maths::Scalar_Bool con1(sqDist1.LT(sqDist2) && sqDist1.LT(sqDist3));
		const Maths::Scalar_Bool con2(sqDist2.LT(sqDist3));

		closest = Select(con1, c2, Select(con2, c4, c6));

	} */

template <int MaxContacts>
bool CollideConeTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF norm, Collision::C_Cone& cone, C_ContactPatch<MaxContacts>& patch,
							  const U32 physicsMaterial, const U32 effectMaterial, const U32 triangleStartIndex, const U32 triangleIndex)
{
	using namespace Maths;

	if(patch.m_NumContactPoints == MaxContacts)
		return true;


	const HVec3 support1 = cone.Support(-norm) ;
	const HVec3 support2 = cone.Support(norm);

	const HF32 dist1 = norm |(support1 - a);
	const HF32 dist2 = norm |(support2 - a);

	const HF32 zero = Constants<HF32>::Zero();

	//No intersection
	if((dist1*dist2).GT(zero))
		return false;

	const HVec3 midPoint = (support1 + support2)*Constants<HF32>::Half();
	const HVec3 dir(cone.Centre() - midPoint);
	const HVec3 support3 = cone.Support(dir);

	bool result = false;

	HVec3 contact2(UNDEFINED);
	if(IntersectTriangleSegmentSB(a, b,c, support1, support2, contact2))
	{
		const HF32 sum(dist1 + dist2);
		
		if(sum < zero)
			return false;

		if(patch.m_NumContactPoints == MaxContacts)
			return true;

		
		C_ContactPoint& point2 = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point2.m_HashValue = (triangleStartIndex + triangleIndex) << 4;
		point2.m_PhysicsMaterialIndices = physicsMaterial;
		point2.m_EffectMaterialIndices = effectMaterial;
		point2.m_ContactPoint = contact2;
		point2.m_PenetrationDepth = 0.f;
		result = true;

		bool _result = false;
		const HVec3 projA = support1 + norm *dist1;

		if(Collision::PointInTriangle(projA, a, b, c))
		{
			point2.m_ContactPoint = support1;
			const HF32 pDepth((support1 - projA).Length());
			Store(pDepth, &point2.m_PenetrationDepth);
			//point2.m_PenetrationDepth =(support1 - projA).Length();
			_result = true;
		}
		else
		{

			HVec3 cP2(UNDEFINED);
			//ClosestPtPointTriangleSegment(contact2, a, b, c, cP2);
			ClosestPtSegmentTriangleSegment(support1, support2, a, b, c, cP2);

			const HVec3 end = cP2 - norm*(cone.BaseRadius() + cone.Height())*Constants<HF32>::Three();

			HVec3 contact(UNDEFINED);
			if(Collision::CollideConeRay(&cone, end, cP2, contact))
			{
				point2.m_ContactPoint = contact;
				const HF32 pDepth((cP2 - contact).Length());
				Store(pDepth, &point2.m_PenetrationDepth);
				//point2.m_PenetrationDepth = (cP2 - contact).Length();
				_result = true;
			}
		}

		//Get the second point of contact
		if(_result)
		{
			if(patch.m_NumContactPoints == MaxContacts)
				return true;
			C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point.m_HashValue = (triangleStartIndex + triangleIndex) << 2;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			point.m_ContactPoint = contact2;
			point.m_PenetrationDepth = 0.f;
		}

	}

	
	HVec3 contact3(UNDEFINED);
	if(IntersectTriangleSegmentSB(a, b,c, support1, support3, contact3))
	{
		if(patch.m_NumContactPoints == MaxContacts)
			return true;

		C_ContactPoint& point2 = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point2.m_HashValue = (triangleStartIndex + triangleIndex) << 8;
		point2.m_PhysicsMaterialIndices = physicsMaterial;
		point2.m_EffectMaterialIndices = effectMaterial;
		point2.m_ContactPoint = contact3;
		point2.m_PenetrationDepth = 0.f;
		result =true;

		bool _result =false;
		const HVec3 projA = support1 + norm *dist1;
		if(Collision::PointInTriangle(projA, a, b, c))
		{
			point2.m_ContactPoint = support1;
			const HF32 pDepth((support1 - projA).Length());
			Store(pDepth, &point2.m_PenetrationDepth);
			//point2.m_PenetrationDepth =(support1 - projA).Length();
			_result = true;
		}
		else
		{
			HVec3 cP2(UNDEFINED);
			//ClosestPtPointTriangleSegment(contact3, a, b, c, cP2);
			ClosestPtSegmentTriangleSegment(support1, support3, a, b, c, cP2);

			const HVec3 end = cP2 - norm*(cone.BaseRadius() + cone.Height())*Constants<HF32>::Three();

			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(end, cP2, RGBA(255, 255, 0));
			HVec3 contact(UNDEFINED);
			if(Collision::CollideConeRay(&cone, end, cP2, contact))
			{
				point2.m_ContactPoint = contact;
				const HF32 pDepth((cP2 - contact).Length());
				Store(pDepth, &point2.m_PenetrationDepth);
				//point2.m_PenetrationDepth = (cP2 - contact).Length();
				_result = true;
			}
		}

		if(_result)
		{
			if(patch.m_NumContactPoints == MaxContacts)
				return true;
			C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point.m_HashValue = (triangleStartIndex + triangleIndex) << 4;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			point.m_ContactPoint = contact3;
			point.m_PenetrationDepth = 0.f;
		}
		
	}

	HVec3 contact4(UNDEFINED);
	if(IntersectTriangleSegmentSB(a, b,c, support2, support3, contact4))
	{
		if(patch.m_NumContactPoints == MaxContacts)
			return true;
		
		C_ContactPoint& point2 = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point2.m_HashValue = (triangleStartIndex + triangleIndex) << 12;
		point2.m_PhysicsMaterialIndices = physicsMaterial;
		point2.m_EffectMaterialIndices = effectMaterial;
		point2.m_ContactPoint = contact4;
		point2.m_PenetrationDepth = 0.f;
		result=true;
		
		
		bool _result = false;
		const HF32 dist3 = norm |(support3 - a);
		const HVec3 projC = support3 + norm *dist3;
		if(Collision::PointInTriangle(projC, a, b, c))
		{
			point2.m_ContactPoint = support3;
			const HF32 pDepth((support3 - projC).Length());
			Store(pDepth, &point2.m_PenetrationDepth);
			//point2.m_PenetrationDepth =(support3 - projC).Length();
			_result=true;
		}
		else
		{
			HVec3 cP2(UNDEFINED);
			//ClosestPtPointTriangleSegment(contact4, a, b, c, cP2);
			ClosestPtSegmentTriangleSegment(support2, support3, a, b, c, cP2);

			const HVec3 end = cP2 - norm*(cone.BaseRadius() + cone.Height())*Constants<HF32>::Three();

			HVec3 contact(UNDEFINED);
			if(Collision::CollideConeRay(&cone, end, cP2, contact))
			{
				point2.m_ContactPoint = contact;
				const HF32 pDepth((cP2 - contact).Length());
				Store(pDepth, &point2.m_PenetrationDepth);
				//point2.m_PenetrationDepth = (cP2 - contact).Length();
				_result=true;
			}
		}

		if(_result)
		{
			if(patch.m_NumContactPoints == MaxContacts)
				return true;
			C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point.m_HashValue = (triangleStartIndex + triangleIndex) << 6;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			point.m_ContactPoint = contact4;
			point.m_PenetrationDepth = 0.f;
		}
	}

	return result;
}

template <int MaxContacts>
void SetContactPoint(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c,const Maths::C_Normal PCREF norm,
					 const Maths::HVec3 PCREF support, const Maths::HVec3 PCREF proj, Collision::C_Cone& cone, C_ContactPoint& point)
{
	/*using namespace Maths;
	if(Collision::PointInTriangle(proj, a, b, c))
	{*/
		point.m_ContactPoint = support;
		const Maths::HF32 pDepth((support - proj).Length());
		Maths::Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth =(support - proj).Length();
	/*}
	else
	{
		HVec3 res(UNDEFINED);
		Collision::ClosestPtPointTriangle(support, a, b, c, res);

		const HVec3 end = res - norm*(cone.m_BaseRadius + cone.m_MidPointLength)*Constants<HF32>::Three();

		HVec3 _contact(UNDEFINED);
		if(Collision::CollideConeRay(&cone, end, res, _contact))
		{
			point.m_ContactPoint = _contact;
			point.m_PenetrationDepth = (res - _contact).Length();
		}
	}*/
	
}

												//----------------Cone----------------------
	template <int MaxPatches, int MaxContacts, class Triangle>
	bool CollideConeTriangleMesh( C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
	{
		using namespace Maths;
		//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
		Spu::MemBaseMarker _marker;
		//C_ConeShape* pConeShape = (C_ConeShape*)Spu::FetchAlloc(pShape, sizeof(C_ConeShape));
		C_ConeShape* pConeShape = (C_ConeShape*)pShape;
		C_Cone& convex(pConeShape->m_Cone);
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		C_Triangle triangle;
		triangle.Margin(zero);
		triangle.Epsilon(convex.Epsilon());
		const Maths::HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
		bool bResult(false);
		HVec3 contact(Maths::UNDEFINED);
		HVec3 normal(Maths::UNDEFINED);
		HF32 pen(Maths::UNDEFINED);
		const HF32 margin(convex.Margin());

		MathsStorage::HenDN3 dNorm;
		//const Maths::HVec3 centre(convex.Centre());
		const HVec3 range(high - low);

		for(U32 a = 0; a < numTris; ++a)
		{
			//Spu::MemBaseMarker marker;
			
			const Triangle* tri = &tris[a];
			const HVec3 localA(&verts[tri->m_IndexA ]);
			triangle.m_VertexA = localA*range + low;
			//((U32&)dNorm) = Spu::Get((U32*)normals + tri->m_NormalIndex);
			dNorm = tri->m_SurfaceNormal;
			triangle.m_Normal = (Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm));
			/*const Maths::HF32 dotA(triangle.m_VertexA | triangle.m_Normal);

			const Maths::HF32 dotCentre((convex.Centre() | triangle.m_Normal) - dotA);
			if(dotCentre < zero)
				continue;*/
			
			//const HVec3 supportNow(convex.SupportMargin(triangle.m_Normal, margin));

			//const HF32 dotNow(((supportNow |triangle.m_Normal) + margin) - dotA);
			
			const HVec3 localB(&verts[tri->m_IndexB ]);
			const HVec3 localC(&verts[tri->m_IndexC ]);
			triangle.m_VertexB = localB*range + low;
			triangle.m_VertexC = localC*range + low;


			C_ContactPatch<MaxContacts>* pPatch(NULL);
			S32 incr = 1;
			if(res.m_NumContactPatches < MaxPatches)
			{
				pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
				pPatch->m_Normal = Maths::C_Normal(-triangle.m_Normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
				pPatch->m_NumContactPoints = 0;
			}

			for(U32 b = 0; b < res.m_NumContactPatches; ++b)
			{
				C_ContactPatch<MaxContacts>& patch(res.m_ContactPatches[b]);
				const Maths::HF32 d(patch.m_Normal.m_V | triangle.m_Normal);
				//const Maths::HF32 d(patch.m_Normal.m_V | normal);
				if(d >= closeNormal)
				{
					incr = 0;
					pPatch = &res.m_ContactPatches[b];
					break;
				}
			}
			if(pPatch == NULL)
				continue;

		
			bool bContact = CollideConeTriangle(triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, Maths::C_Normal(triangle.m_Normal), convex, *pPatch,  physicsMaterial, effectMaterial, triangleStartIndex, triangleStartIndex + a);

			res.m_NumContactPatches += (incr && bContact);

			bResult = bResult || bContact;
			
		}
		return bResult;
	}


	template <int MaxPatches, int MaxContacts, class Triangle>
	bool CollideConeTriangleMesh(C_ConeProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, 
		const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
	{
		using namespace Maths;

		C_Cone& convex(proxy.m_Cone);
		//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
		const Maths::HF32 zero(Maths::Constants<HF32>::Zero());
		C_Triangle triangle;
		triangle.Margin(Maths::Constants<HF32>::Zero());//convex.Margin());
		const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
		bool bResult(false);
		HVec3 contact(UNDEFINED);
		HVec3 normal(UNDEFINED);
		HF32 pen(UNDEFINED);
		const HF32 margin(convex.Margin());
		const HF32 disEps(SharedPhysics::Constants::CloseNormal());

		const HMat33 orientation(proxy.m_PosRot.Rotation());
		const HVec3 position(proxy.m_PosRot.Translation());
		const HF32 twenty(SharedPhysics::Constants::Twenty());

		MathsStorage::HenDN3 dNorm;
		const HVec3 range(high - low);

		for(U32 a = 0; a < numTris; ++a)
		{
			//Spu::MemBaseMarker marker;
			//const Triangle* tempTri=(const Triangle*)Spu::FetchAllocUnaligned(tris + a, sizeof(Triangle));
			const Triangle* tempTri=&tris[a];
			//((U32&)dNorm) = Spu::Get( (U32*)normals + tempTri->m_NormalIndex);
			dNorm = tempTri->m_SurfaceNormal;
			const HVec3 localA(&verts[tempTri->m_IndexA ]);
			const HVec3 localB(&verts[tempTri->m_IndexB ]);
			const HVec3 localC(&verts[tempTri->m_IndexC ]);
			triangle.m_VertexA = localA*range + low;
			triangle.m_VertexB = localB*range + low;
			triangle.m_VertexC = localC*range + low;
			triangle.m_Normal = Load<HVec3, MathsStorage::HenDN3>(&dNorm);


			C_ContactPatch<MaxContacts>* pPatch(NULL);
			S32 incr = 1;
			const Maths::HVec3 worldNormal(-triangle.m_Normal * orientation);
			if(res.m_NumContactPatches < MaxPatches)
			{
				pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
				pPatch->m_Normal = Maths::C_Normal(worldNormal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
				pPatch->m_NumContactPoints = 0;
			}

			for(U32 b = 0; b < res.m_NumContactPatches; ++b)
			{
				C_ContactPatch<MaxContacts>& patch(res.m_ContactPatches[b]);
				const Maths::HF32 d(patch.m_Normal.m_V | worldNormal);
				//const Maths::HF32 d(patch.m_Normal.m_V | normal);
				if(d >= closeNormal)
				{
					incr = 0;
					pPatch = &res.m_ContactPatches[b];
					break;
				}
			}
			if(pPatch == NULL)
				continue;

		
			U32 numContacts = pPatch->m_NumContactPoints;
			bool bContact = CollideConeTriangle(triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, Maths::C_Normal(triangle.m_Normal), convex, *pPatch,  physicsMaterial, effectMaterial, triangleStartIndex, triangleStartIndex + a);

			
			for(U32 k = numContacts; k < pPatch->m_NumContactPoints; ++k)
			{
				C_ContactPoint& point(pPatch->m_ContactPoints[k]);
				point.m_ContactPoint = (point.m_ContactPoint * orientation) + position;
			}

			res.m_NumContactPatches += (incr && bContact);
			bResult = bResult || bContact;

		}
		return bResult;
	}


};

#endif