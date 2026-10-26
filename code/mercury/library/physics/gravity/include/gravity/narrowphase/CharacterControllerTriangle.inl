#ifndef CHARACTERCONTROLLERTRIANGLE_INL
#define CHARACTERCONTROLLERTRIANGLE_INL

namespace Collision
{

NOALIAS FORCE_INLINE Maths::Scalar_Bool IntersectTriangleSegmentCC(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::C_Normal PCREF norm, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, Maths::HVec3& res, Maths::HF32& distance, Maths::C_Normal& resultNormal) 
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const Scalar_Bool bFalse(false);

	const HF32 radSq(radius * radius);

	HVec3 result(UNDEFINED);
	if(IntersectTriangleSegmentSB(a, b, c, p, q + direction * radius, result))
	{
		res = result;
		distance = (direction | (result - p)) - radius;
		resultNormal = -direction;
		return !bFalse;
	}

	HVec3 closestLine(UNDEFINED), closestTri(UNDEFINED);
	HF32 sqDist(UNDEFINED);
	Collision::ClosestPtSegmentTriangle(p, q, a, b, c, closestLine, closestTri, sqDist);

	if(sqDist < radSq)
	{
		//OK - definitely hit, but need to solve such that the distance from the line and point of contact
		//is exactly radius!
		HVec3 closestLine2(UNDEFINED);
		Collision::ClosestPtPointSegment(closestTri, p, q + direction * SharedPhysics::Constants::Twenty(), closestLine2);
		const HVec3 resRes(closestLine2 - closestTri);
		const HF32 dd(resRes | resRes);
		const HF32 scaleSq(radSq - dd);
		const HF32 scale(scaleSq.Sqrt());


		const HVec3 rayEnd = closestLine2 - direction * scale;
		res = closestTri;
		distance = (rayEnd - p).Length();
		resultNormal = Maths::C_Normal(rayEnd - res);
		return !bFalse;
	}
	return bFalse;
}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCapsuleTriangle_CC(C_Capsule& convex, const MathsStorage::Fixed3_S16<14>* const __restrict verts, 
	const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, 
	Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial, 
	const Maths::HF32 PCREF maxCosAngle, const Maths::HVec3& low, const Maths::HVec3& high)
{
	using namespace Maths;
	//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
	const HF32 zero(Constants<HF32>::Zero());
	//const Maths::HF32 third(Maths::Constants<Maths::HF32>::Third());
	const HF32 twenty(SharedPhysics::Constants::Twenty());
	C_Triangle triangle;
	triangle.Margin(Constants<HF32>::Zero());//convex.Margin());
	const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
	bool bResult(false);
	HVec3 contact(UNDEFINED);
	C_Normal normal(UNDEFINED);
	HF32 pen(UNDEFINED);
	const HF32 margin(convex.Margin());

	MathsStorage::HenDN3 dNorm;
	//const Maths::HVec3 centre(convex.Centre());
	const HVec3 range(high - low);
	for(U32 a = 0; a < numTris; ++a)
	{
		Spu::MemBaseMarker marker;
		//const Triangle* tri = (const Triangle*)Spu::FetchAllocUnaligned(tris + a, sizeof(Triangle));
		const Triangle* tri = &tris[a];
		//((U32&)dNorm) = Spu::Get((U32*)(normals + tri->m_NormalIndex));
		dNorm = tri->m_SurfaceNormal;
		triangle.m_Normal = Load<HVec3, MathsStorage::HenDN3>(&dNorm);

		/*if(triangle.m_Normal.Y() > maxCosAngle)
		{
			continue;
		}*/

		

		triangle.m_Normal = -triangle.m_Normal;
	/*	const HF32 dotA(triangle.m_VertexA | triangle.m_Normal);
		
		const HVec3 supportBefore(convex.SupportMargin(-triangle.m_Normal, margin));
		const HVec3 supportNow(convex.SupportMargin(triangle.m_Normal, margin));

		const HF32 dotBefore((supportBefore |triangle.m_Normal) - (dotA + margin));
		const HF32 dotDt(bodyDt | triangle.m_Normal);
		if((dotBefore - dotDt) > zero)
		{
			continue;	
		}
		const HF32 dotNow(((supportNow |triangle.m_Normal) + margin) - dotA);*/

	/*	MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexA, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexB, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexC, sizeof(MathsStorage::Float3));

		triangle.m_VertexA = Load<HVec3>(f0);
		triangle.m_VertexB = Load<HVec3>(f1);
		triangle.m_VertexC = Load<HVec3>(f2);*/

		const HVec3 localA(&verts[tri->m_IndexA ]);
		const HVec3 localB(&verts[tri->m_IndexB ]);
		const HVec3 localC(&verts[tri->m_IndexC ]);
		
		triangle.m_VertexA = localA*range + low;
		triangle.m_VertexB = localB*range + low;
		triangle.m_VertexC = localC*range + low;

		if(Collision::CollideCapsuleTriangle(convex, triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, contact, normal, pen))
		{
			bResult = true;
			//Store this contact into the collision result...
			C_ContactPatch<MaxContacts>* pPatch(NULL);
			S32 incr = 1;

			if(res.m_NumContactPatches < MaxPatches)
			{
				pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
				pPatch->m_Normal = normal;//Maths::C_Normal(triangle.m_Normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
				pPatch->m_NumContactPoints = 0;
			}

			for(U32 b = 0; b < res.m_NumContactPatches; ++b)
			{
				C_ContactPatch<MaxContacts>& patch(res.m_ContactPatches[b]);
				const Maths::HF32 d(patch.m_Normal.m_V | normal);
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
				C_ContactPoint& point(pPatch->m_ContactPoints[pPatch->m_NumContactPoints++]);
				point.m_ContactPoint = contact;
				point.m_PenetrationDepth = pen;//Maths::Max((normal | -triangle.m_Normal) * pen, zero);
				Maths::Store(pen, &point.m_PenetrationDepth);
				point.m_HashValue = triangleStartIndex + a;
				point.m_PhysicsMaterialIndices = physicsMaterial;
				point.m_EffectMaterialIndices = effectMaterial;
			}
		}
		
	}
	return bResult;
}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideControllerTriangleMesh(C_Shape* shape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, 
C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
{
	//Collision::C_CharacterControllerShape* pShape = (Collision::C_CharacterControllerShape*)Spu::FetchAlloc(shape, sizeof(C_CharacterControllerShape));
	Collision::C_CharacterControllerShape* pShape = (Collision::C_CharacterControllerShape*)shape;
	bool b1(CollideCapsuleTriangle_CC<MaxPatches, MaxContacts, Triangle>
		(pShape->m_Controller.m_Capsule, verts, normals, tris, numTris, triangleStartIndex, res, physicsMaterial, effectMaterial,
		pShape->m_Controller.m_CosMaxAngle, low, high));
	return b1;

}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCapsuleTriangle_CC(C_CharacterControllerProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, 
	const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, 
	Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial, 
	const Maths::HF32 PCREF maxCosAngle, const Maths::HVec3& low, const Maths::HVec3& high)
{

	using namespace Maths;
	HMat33 rot(proxy.m_PosRot.Rotation());
	HVec3 pos(proxy.m_PosRot.Translation());

	C_Capsule& convex(proxy.m_Capsule);

	//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
	const HF32 zero(Constants<HF32>::Zero());
	//const Maths::HF32 third(Maths::Constants<Maths::HF32>::Third());
	//const Maths::HF32 twenty(SharedPhysics::Constants::Twenty());
	C_Triangle triangle;
	triangle.Margin(Constants<HF32>::Zero());//convex.Margin());
	const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
	bool bResult(false);
	HVec3 contact(UNDEFINED);
	C_Normal normal(UNDEFINED);
	HF32 pen(UNDEFINED);
	const HF32 margin(convex.Margin());
	MathsStorage::HenDN3 dNorm;

	const HVec3 range(high - low);

	//const Maths::HVec3 centre(convex.Centre());

	for(U32 a = 0; a < numTris; ++a)
	{
		Spu::MemBaseMarker marker;

		const Triangle* tri = &tris[a];
		const HVec3 localA(&verts[tri->m_IndexA ]);
		triangle.m_VertexA = localA*range + low;

		//const Triangle* tri((Triangle*)Spu::FetchAllocUnaligned(&tris[a], sizeof(Triangle)));
		//triangle.m_VertexA = Load<HVec3>((MathsStorage::Float3*)Spu::FetchAllocUnaligned(&verts[tri->m_IndexA], sizeof(MathsStorage::Float3)));
		//((U32&)dNorm) = Spu::Get((U32*)&normals[tri->m_NormalIndex]);
		dNorm = tri->m_SurfaceNormal;
		triangle.m_Normal =  Load<HVec3>(&dNorm);
		const HVec3 norm(-triangle.m_Normal * rot);

		/*if(norm.Y() > maxCosAngle)
		{
			continue;
		}*/
		/*const Maths::HF32 dp(norm | -proxy.m_Ray.m_Direction);
		if(dp > maxCosAngle)
			continue;*/

		triangle.m_Normal = -triangle.m_Normal;
		/*const HF32 dotA(triangle.m_VertexA | triangle.m_Normal);
		
		const HVec3 supportBefore(convex.SupportMargin(-triangle.m_Normal, margin));
		const HVec3 supportNow(convex.SupportMargin(triangle.m_Normal, margin));

		const HF32 dotBefore((supportBefore |triangle.m_Normal) - (dotA + margin));
		const HF32 dotDt(bodyDt | triangle.m_Normal);
		if((dotBefore - dotDt) > zero)
		{
			continue;	
		}
		const HF32 dotNow(((supportNow |triangle.m_Normal) + margin) - dotA);*/
		/*triangle.m_VertexB = Load<HVec3>((MathsStorage::Float3*)Spu::FetchAllocUnaligned(&verts[tri->m_IndexB], sizeof(MathsStorage::Float3)));
		triangle.m_VertexC = Load<HVec3>((MathsStorage::Float3*)Spu::FetchAllocUnaligned(&verts[tri->m_IndexC], sizeof(MathsStorage::Float3)));*/
		const HVec3 localB(&verts[tri->m_IndexB ]);
		const HVec3 localC(&verts[tri->m_IndexC ]);
		triangle.m_VertexB = localB*range + low;
		triangle.m_VertexC = localC*range + low;

		//triangle.m_Extruded = ((triangle.m_VertexA + triangle.m_VertexB + triangle.m_VertexC) * third) + 
		//	triangle.m_Normal * Maths::Constants<Maths::HF32>::Twenty();

		//triangle.m_Extrusion = Maths::Max(zero, dotNow * twenty);
		
		//triangle.WarmStart(triangle.m_VertexA);
		//triangle.Margin(convex.Margin());

		//if(GJKCPairMargin<C_Capsule, C_Triangle>(convex, triangle , contact, normal, pen, *res.m_pEpa))
		if(Collision::CollideCapsuleTriangle(convex, triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, contact, normal, pen))
		{
			bResult = true;
			//Store this contact into the collision result...
			C_ContactPatch<MaxContacts>* pPatch(NULL);
			S32 incr = 1;
			if(res.m_NumContactPatches < MaxPatches)
			{
				pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
				pPatch->m_Normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
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
				C_ContactPoint& point(pPatch->m_ContactPoints[pPatch->m_NumContactPoints++]);
				point.m_ContactPoint = contact*rot + pos;
				//const Maths::HF32 norDir(normal | triangle.m_Normal);
				//point.m_PenetrationDepth = Maths::Abs(norDir * pen);
				//point.m_PenetrationDepth = Maths::Max(dotNow, zero);
				//point.m_PenetrationDepth = Maths::Max((normal | -triangle.m_Normal) * pen, zero);
				const Maths::HF32 pDepth(Max((normal | -triangle.m_Normal) * pen, zero));
				Maths::Store(pDepth, &point.m_PenetrationDepth);
				//point.m_PenetrationDepth = Max((normal | -triangle.m_Normal) * pen, zero);
				point.m_HashValue = triangleStartIndex + a;
				point.m_PhysicsMaterialIndices = physicsMaterial;
				point.m_EffectMaterialIndices = effectMaterial;
			}
		}
		
	}
	return bResult;

}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideControllerTriangleMesh(C_CharacterControllerProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, 
C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial, 
const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
{
	//Collision::C_CharacterControllerShape* pShape = (Collision::C_CharacterControllerShape*)shape;
	bool b1(CollideCapsuleTriangle_CC<MaxPatches, MaxContacts, Triangle>
		(proxy, verts, normals, tris, numTris, triangleStartIndex, res, physicsMaterial, effectMaterial,
		proxy.m_CosMaxAngle, low, high));
	return b1;
}


};

#endif

