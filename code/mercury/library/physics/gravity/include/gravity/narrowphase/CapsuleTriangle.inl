#ifndef CAPSULETRIANGLE_INL
#define CAPSULETRIANGLE_INL

#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/CapsuleRay.inl"
#include "gravity/narrowphase/GJK.inl"

namespace Collision
{

template <int MaxContacts>
bool CollideCapsuleTriangle(const C_Capsule& capsule, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, C_ContactPatch<MaxContacts>& patch)
{
	using namespace Maths;

	if(patch.m_NumContactPoints == MaxContacts)
		return false;
	const HF32 r(capsule.Radius());

	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const C_Normal normal(ab^ac);
	patch.m_Normal = normal;


	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(a, normal, 1.f, 0.25f, RGBA(0, 0, 0));

	const HF32 dA(a | normal);
	const HF32 dS((capsule.Start() | normal) - dA);
	const HF32 dE((capsule.End() | normal) - dA);
	const HF32 absDS(Maths::Abs(dS));
	const HF32 absDE(Maths::Abs(dE));

	const HF32 dist(dS*dE);
	const HF32 zero(Constants<HF32>::Zero());

	if(dist.GT(zero) & absDS.GT(r) & absDE.GT(r))
		return false;

	const HF32 three = Maths::Constants<HF32>::Three();
	const HVec3 v(capsule.End() - capsule.Start());
	const HF32 sqV(v|v);

	
	HVec3 contact(Maths::UNDEFINED);
	bool result= false;

	const HVec3 projS = capsule.Start() - normal*dS;
	const HVec3 projE = capsule.End() - normal*dE;

	if(Collision::IntersectTriangleSegment(a, b, c, capsule.Start(), capsule.End(), contact))
	{
		
		const HVec3 rayStart = contact - normal*sqV;
		HVec3 res1(Maths::UNDEFINED);
		if(CollideCapsuleRay(&capsule, rayStart, contact, res1))
		{
			C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point1.m_ContactPoint = res1;
			const Maths::HF32 penDepth((res1 - contact).Length());
			Maths::Store(penDepth, &point1.m_PenetrationDepth);
			result = true;
		}

		if(dE.LT(zero))
		{
			if(Collision::PointInsideTriangle(projE, a, b, c))
			{
				C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
				point1.m_ContactPoint = capsule.End() - normal*r;
				const Maths::HF32 penDepth(r - dE);
				Maths::Store(penDepth, &point1.m_PenetrationDepth);
				result = true;
			}
		}
		else
		{
			if(Collision::PointInsideTriangle(projS, a, b, c))
			{
				C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
				point1.m_ContactPoint = capsule.Start() - normal*r;
				//point1.m_PenetrationDepth = r - dS;
				const Maths::HF32 penDepth(r - dS);
				Maths::Store(penDepth, &point1.m_PenetrationDepth);
				result = true;
			}

		}
	
	}
	else
	{
		
		HVec3 closestA(Maths::UNDEFINED);
		HVec3 closestB(Maths::UNDEFINED);
		HF32 sqDist(Maths::UNDEFINED);
		ClosestPtSegmentTriangle(capsule.Start(), capsule.End(), a, b, c, closestA, closestB, sqDist);

		if(sqDist.GT(r*r))
			return false;

		const HVec3 rayStart = closestA - normal*sqV;
		HVec3 res;
		if(Collision::CollideCapsuleRay(&capsule, rayStart, closestA, res))
		{
			C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point1.m_ContactPoint = res;
			const Maths::HF32 penDepth((res - closestB).Length());
			//point1.m_PenetrationDepth = (res - closestB).Length();
			Maths::Store(penDepth, &point1.m_PenetrationDepth);
			result = true;
		}
	}

	

	const C_Normal norm(projE - projS);
	const HVec3 contactS =  projS- norm*r*three;
	const HVec3 contactE =  projE + norm*r*three;

	HVec3 res2(Maths::UNDEFINED);
	if(CollideCapsuleRay(&capsule, contactS, contactE, res2))
	{
		C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point1.m_ContactPoint = res2;
		point1.m_PenetrationDepth = 0.f;
		result=true;
	}

	return result;
}

template <int MaxContacts>
bool CollideCapsuleTriangle2(const C_Capsule& capsule, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, C_ContactPatch<MaxContacts>& patch)
{
	using namespace Maths;

	const HF32 r(capsule.m_Radius);

	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const C_Normal normal(ab^ac);
	patch.m_Normal = normal;

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(a, normal, 1.f, 0.25f, RGBA(0, 0, 0));

	const HF32 dA(a | normal);
	const HF32 dS((capsule.Start() | normal) - dA);
	const HF32 dE((capsule.End() | normal) - dA);
	const HF32 absDS(Maths::Abs(dS));
	const HF32 absDE(Maths::Abs(dE));

	const HF32 dist(dS*dE);
	const HF32 zero(Constants<HF32>::Zero());

	if(dist.GT(zero) && absDS.GT(r) && absDE.GT(r))
		return false;

	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	HF32 sqDist(Maths::UNDEFINED);

	ClosestPtSegmentTriangle(capsule.Start(), capsule.End(), a, b, c, closestA, closestB, sqDist);

	if(sqDist.GT(r*r))
		return false;

	
	if(dist.GT(zero))
	{
			
		const HVec3 point1(closestA - capsule.Start());
		const HVec3 point2(closestA - capsule.End());
		const HF32 length1 = point1.Length();
		const HF32 length2 = point2.Length();
			

		if(length1.GT(Maths::Constants<HF32>::Epsilon()) && length2.GT(Maths::Constants<HF32>::Epsilon()))
		{
			const HF32 pDepth(r-sqDist.Sqrt());
			C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point1.m_ContactPoint = closestB;
			//point1.m_PenetrationDepth = pDepth;
			Maths::Store(pDepth, &point1.m_PenetrationDepth);
			return true;
		}
		else
		{

			
			const Scalar_Bool bCon(dS.GT(zero));

			const Maths::Scalar_Bool con((bCon & absDS.LT(absDE)) | (!bCon & absDS.GT(absDE)));
			const HVec3 start = Select(con, capsule.Start(), capsule.End());
			const HVec3 end = Select(con, capsule.End(), capsule.Start());
			const HF32 ddS = Select(con, dS, dE);
			const HF32 ddE = Select(con, dE, dS);
			const HF32 cDist = Select(con, absDS, absDE);


			const HVec3 contactPoint1 = start - normal*cDist;
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactPoint1, 0.25f, RGBA(255, 0, 255));
			if(Collision::PointInsideTriangle(contactPoint1, a, b,c))
			{
				C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
				point1.m_ContactPoint = start - normal*r;
				const HF32 pDepth(r + Maths::Select(bCon, -cDist, cDist));
				Maths::Store(pDepth, &point1.m_PenetrationDepth);
				//point1.m_PenetrationDepth = r + Maths::Select(bCon, -cDist, cDist);
			}


			const HVec3 contact1 = start - normal*ddS;
			const HVec3 contact2 = end - normal*ddE;


			const HF32 three = Maths::Constants<HF32>::Three();
			const C_Normal norm(contact2 - contact1);
			const HVec3 contactS = contact1 - norm*r*three;
			const HVec3 contactE = contact2 + norm*r*three;

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactS, 0.25f, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactE, 0.25f, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(contactS, "S");
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(contactE, "E");
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contactS, contactE, RGBA(255, 0, 0));

			HVec3 contact;
			if(CollideCapsuleRay(&capsule, contactS, contactE, contact))
			{
				//Need to check whether the point is inside the triangle
				if(Collision::PointInsideTriangle(contact, a, b,c))
				{
					C_ContactPoint& point2 = patch.m_ContactPoints[patch.m_NumContactPoints++];
					point2.m_ContactPoint = contact;
					//point2.m_PenetrationDepth = zero;
					point2.m_PenetrationDepth = 0.f;
				}
			}
		}
	}
	else
	{

		const Maths::Scalar_Bool bCon(dS.LT(zero));
		{
			/*const HVec3 projS = capsule.m_Start - normal*r;
			const HVec3 projE = capsule.m_End - normal*r;

			bool sBool = Collision::PointInsideTriangle(projS, a, b, c);
			bool eBool = Collision::PointInsideTriangle(projE, a, b, c);

			if(sBool)
			{
				C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
				point1.m_ContactPoint = projS;
				point1.m_PenetrationDepth = r + abs;
			}*/

			const Maths::HVec3 capsuleVertex(Maths::Select(bCon, capsule.Start(), capsule.End()));
			const Maths::HF32 capsuleDis(Maths::Select(bCon, absDS, absDE));

			const HVec3 contactPoint1(capsuleVertex - normal*capsuleDis);
			if(Collision::PointInsideTriangle(contactPoint1, a, b, c))
			{
				C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
				point1.m_ContactPoint = capsuleVertex - normal*r;
				const HF32 pDepth(r + capsuleDis);
				Maths::Store(pDepth, &point1.m_PenetrationDepth);
				//point1.m_PenetrationDepth = r + capsuleDis;
			}

			const HVec3 contact1 = capsule.Start() - normal*dS;
			const HVec3 contact2 = capsule.End() - normal*dE;

		
			const HF32 three = Maths::Constants<HF32>::Three();
			const C_Normal norm(contact2 - contact1);
			const HVec3 contactS = contact1 - norm*r*three;
			const HVec3 contactE = contact2 + norm*r*three;

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactS, 0.25f, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactE, 0.25f, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(contactS, "S");
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(contactE, "E");
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contactS, contactE, RGBA(255, 0, 0));

			
			HVec3 contact;
			if(CollideCapsuleRay(&capsule, contactS, contactE, contact))
			{
				if(Collision::PointInsideTriangle(contact, a, b, c))
				{
					C_ContactPoint& point2 = patch.m_ContactPoints[patch.m_NumContactPoints++];
					point2.m_ContactPoint = contact;
					//point2.m_PenetrationDepth = zero;
					point2.m_PenetrationDepth = 0.f;
				}
			}
		}

		/*if(dS.LT(zero))
		{
			C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point1.m_ContactPoint = capsule.m_Start - normal*r;
			point1.m_PenetrationDepth = r + absDS;

			const HVec3 contact1 = capsule.m_Start - normal*dS;
			const HVec3 contact2 = capsule.m_End - normal*dE;

		
			const HF32 three = Maths::Constants<HF32>::Three();
			const C_Normal norm(contact2 - contact1);
			const HVec3 contactS = contact1 - norm*r*three;
			const HVec3 contactE = contact2 + norm*r*three;

			
			HVec3 contact;
			if(CollideCapsuleRay(&capsule, contactS, contactE, contact))
			{
				C_ContactPoint& point2 = patch.m_ContactPoints[patch.m_NumContactPoints++];
				point2.m_ContactPoint = contact;
				point2.m_PenetrationDepth = zero;
			}
		}
		else
		{
			C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point1.m_ContactPoint = capsule.m_End - normal*r;
			point1.m_PenetrationDepth = r + absDE;

			const HVec3 contact1 = capsule.m_End - normal*dE;
			const HVec3 contact2 = capsule.m_Start -normal*dS;

			const HF32 three = Maths::Constants<HF32>::Three();
			const C_Normal norm(contact2 - contact1);
			const HVec3 contactS = contact1 - norm*r*three;
			const HVec3 contactE = contact2 + norm*r*three;
			
			HVec3 contact;
			if(CollideCapsuleRay(&capsule, contactS, contactE, contact))
			{
				C_ContactPoint& point2 = patch.m_ContactPoints[patch.m_NumContactPoints++];
				point2.m_ContactPoint = contact;
				point2.m_PenetrationDepth = zero;
			}

		}*/
	}

	return true;
}


template <int MaxContacts>
bool CollideCapsuleTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF normal, Collision::C_Capsule& capsule, C_ContactPatch<MaxContacts>& patch,
							  const U32 physicsMaterial, const U32 effectMaterial, const U32 triangleStartIndex, const U32 triangleIndex)
{
	using namespace Maths;

	if(patch.m_NumContactPoints == MaxContacts)
		return false;

	const HF32 r(capsule.Radius());

	const HF32 dA(a | normal);
	const HF32 dS((capsule.Start() | normal) - dA);
	const HF32 dE((capsule.End() | normal) - dA);

	const HF32 dist(dS*dE);
	const HF32 zero(Constants<HF32>::Zero());

	//if(dist.GT(zero) & Abs(dS).GT(r) & Abs(dE).GT(r))
	//	return false;

	const HVec3 support1(capsule.Support(normal));
	const HVec3 support2(capsule.Support(-normal));

	const HF32 dist1((support1|normal) - dA);
	const HF32 dist2((support2|normal) - dA);

	if((dist1*dist2).GT(zero) | (dist1 + dist2).LT(zero))
		return false;

	//bool result= false;

	
	
	HVec3 contact(Maths::UNDEFINED);
	
	const HVec3 projS(capsule.Start() - normal*dS);
	const HVec3 projE(capsule.End() - normal*dE);

	//if(IntersectTriangleSegmentSB(a, b, c, capsule.Start(), capsule.End(), contact))
	//{
	//	C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
	//	point1.m_ContactPoint = contact - normal*r;
	//	point1.m_EffectMaterialIndices = effectMaterial;
	//	point1.m_PhysicsMaterialIndices = physicsMaterial;
	//	point1.m_HashValue =(triangleStartIndex + triangleIndex) << 2;
	//	Maths::Store(r, &point1.m_PenetrationDepth);
	//	//result = true;
	//	if(patch.m_NumContactPoints == MaxContacts)
	//		return true;
	//	

	//	const Scalar_Bool con(dE.LT(zero));
	//	const HVec3 proj(Select(con, projE, projS));
	//	const HVec3 point(Select(con, capsule.End(), capsule.Start()));
	//	const HF32 dist(Select(con, dE, dS));

	//	if(Collision::PointInsideTriangle(proj, a, b, c))
	//	{
	//		C_ContactPoint& point2 = patch.m_ContactPoints[patch.m_NumContactPoints++];
	//		point2.m_ContactPoint = point - normal*r;
	//		point2.m_EffectMaterialIndices = effectMaterial;
	//		point2.m_PhysicsMaterialIndices = physicsMaterial;
	//		point2.m_HashValue =(triangleStartIndex + triangleIndex) << 3;
	//		const Maths::HF32 penDepth(r - dist);
	//		Maths::Store(penDepth, &point2.m_PenetrationDepth);
	//		//result = true;
	//	}
	//	return true;
	//}
	//else
	{
		
		const HVec3 projSupport(support2 + normal*Abs(dist2));
		if(PointInsideTriangle(projSupport, a, b, c))
		{
			C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point1.m_ContactPoint = support2;
			point1.m_EffectMaterialIndices = effectMaterial;
			point1.m_PhysicsMaterialIndices = physicsMaterial;
			point1.m_HashValue =(triangleStartIndex + triangleIndex) << 2;
			Maths::Store(Abs(dist2), &point1.m_PenetrationDepth);
			//result = true;

		}
		else
		{

			HVec3 closestA(Maths::UNDEFINED);
			HVec3 closestB(Maths::UNDEFINED);
			HF32 sqDist(Maths::UNDEFINED);
			ClosestPtSegmentTriangle(capsule.Start(), capsule.End(), a, b, c, closestA, closestB, sqDist);

			if(sqDist.GT(r*r))
				return false;

			const C_Normal dir(closestB - closestA);
			const HVec3 contact= closestA + dir * r;
			const HF32 absDist = Abs((contact|normal) - dA);
			C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point1.m_ContactPoint = contact;
			point1.m_EffectMaterialIndices = effectMaterial;
			point1.m_PhysicsMaterialIndices = physicsMaterial;
			point1.m_HashValue =(triangleStartIndex + triangleIndex) << 2;
			Maths::Store(absDist, &point1.m_PenetrationDepth);
			//result = true;
			
		}

		if(patch.m_NumContactPoints == MaxContacts)
			return true;

		const C_Normal norm(projE - projS);
		const HF32 three = Maths::Constants<HF32>::Three();
		const HVec3 contactS =  projS - norm*r*three;
		const HVec3 contactE =  projE + norm*r*three;

		HVec3 rest1(Maths::UNDEFINED);
		HVec3 rest2(Maths::UNDEFINED);

		if(CollideCapsuleRay(&capsule, contactS, contactE, rest1, rest2))
		{
			if(PointInsideTriangle(rest1, a, b, c))
			{
				C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
				point1.m_ContactPoint = rest1;
				point1.m_EffectMaterialIndices = effectMaterial;
				point1.m_PhysicsMaterialIndices = physicsMaterial;
				point1.m_HashValue =(triangleStartIndex + triangleIndex) << 3;
				point1.m_PenetrationDepth = 0.f;

				if(patch.m_NumContactPoints == MaxContacts)
					return true;
			}

			if(PointInsideTriangle(rest2, a, b, c))
			{
				C_ContactPoint& point2 = patch.m_ContactPoints[patch.m_NumContactPoints++];
				point2.m_ContactPoint = rest2;
				point2.m_EffectMaterialIndices = effectMaterial;
				point2.m_PhysicsMaterialIndices = physicsMaterial;
				point2.m_HashValue =(triangleStartIndex + triangleIndex) << 4;
				point2.m_PenetrationDepth = 0.f;
			}
		}
		
		//if(CollideCapsuleRay(&capsule, contactS, contactE, rest2))
		//{
		//	if(PointInsideTriangle(rest2, a, b, c))
		//	{
		//		C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
		//		point1.m_ContactPoint = rest2;
		//		point1.m_EffectMaterialIndices = effectMaterial;
		//		point1.m_PhysicsMaterialIndices = physicsMaterial;
		//		point1.m_HashValue =(triangleStartIndex + triangleIndex) << 3;
		//		point1.m_PenetrationDepth = 0.f;
		//		//result=true;
		//		if(patch.m_NumContactPoints == MaxContacts)
		//			return true;
		//	}
		//}

		//if(CollideCapsuleRay(&capsule, contactE, contactS, rest2))
		//{
		//	if(PointInsideTriangle(rest2, a, b, c))
		//	{
		//		C_ContactPoint& point1 = patch.m_ContactPoints[patch.m_NumContactPoints++];
		//		point1.m_ContactPoint = rest2;
		//		point1.m_EffectMaterialIndices = effectMaterial;
		//		point1.m_PhysicsMaterialIndices = physicsMaterial;
		//		point1.m_HashValue =(triangleStartIndex + triangleIndex) << 4;
		//		point1.m_PenetrationDepth = 0.f;
		//		//result=true;
		//	/*	if(patch.m_NumContactPoints == MaxContacts)
		//			return true;*/
		//	}
		//}
	}
	//return result;
	return true;
}


template <int MaxContacts>
bool CollideCapsuleTriangle2(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF norm, Collision::C_Capsule& capsule, C_ContactPatch<MaxContacts>& patch,
							  const U32 physicsMaterial, const U32 effectMaterial, const U32 triangleStartIndex, const U32 triangleIndex)
{
	if(patch.m_NumContactPoints == MaxContacts)
		return true;

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 dA(a | norm);
	const Maths::HF32 dS((capsule.Start() | norm) - dA);
	const Maths::HF32 dE((capsule.End() | norm) - dA);
	const Maths::HF32 absDs(Maths::Abs(dS));
	const Maths::HF32 absDE(Maths::Abs(dE));
	

	if(Maths::Abs(dS * dE).GE(zero) & absDs.GE(capsule.Radius()) & absDE.GE(capsule.Radius()))
		return false;

	Maths::HVec3 closestA(Maths::UNDEFINED), closestB(Maths::UNDEFINED),
		closestStart(Maths::UNDEFINED), closestEnd(Maths::UNDEFINED),
		closestStart2(Maths::UNDEFINED), closestEnd2(Maths::UNDEFINED);
	Maths::HF32 sqDist(Maths::UNDEFINED);

	//Two contacts to find - deepest and point of first intersection!. Firstly, find point of first 
	//intersection.

	ClosestPtSegmentTriangle(capsule.Start(), capsule.End(), a, b, c, closestA, closestB, sqDist);
	const Maths::HVec3 dir(closestA - closestB);
	const Maths::HF32 disA((dir | norm));
	const Maths::HF32 radSq(capsule.Radius() * capsule.Radius());
	//if(sqDist > capsule.Radius()*capsule.Radius()) 
	//	return false;

	if(sqDist > radSq)
		return false;

	//const Maths::HF32 disA(sqDist.Sqrt());
	{
		//Select which side this point is on...
		
		C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point.m_ContactPoint = closestB;
		point.m_HashValue = (triangleStartIndex + triangleIndex) << 2;
		point.m_PhysicsMaterialIndices = physicsMaterial;
		point.m_EffectMaterialIndices = effectMaterial;
		const Maths::HF32 pDepth(capsule.Radius() - disA);
		Maths::Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth = capsule.Radius() - disA;
	}

	if(patch.m_NumContactPoints == MaxContacts)
		return true;

	//Test to see if segment hits triangle...

	{
		const Maths::HVec3 start(Maths::Select(dS.GT(zero), capsule.Start(), capsule.End()));
		const Maths::HVec3 end(Maths::Select(dE.GT(zero), capsule.End(), capsule.Start()));
		const Maths::C_Normal dir((end - start));
		Maths::HVec3 hitPoint(Maths::UNDEFINED);

		if(Collision::IntersectTriangleSegment(a, b, c, start - dir * capsule.Radius(), end + dir * capsule.Radius(), hitPoint))
		{
			//Record point of contact!
			const Maths::HF32 pen = Maths::Abs(Maths::Min(dS, dE) - capsule.Radius());
			C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
			point.m_ContactPoint = end;
			point.m_HashValue = ((triangleStartIndex + triangleIndex) << 2) | 4;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
			//point.m_PenetrationDepth = pen;
			Maths::Store(pen, &point.m_PenetrationDepth);
			
		}
		if(patch.m_NumContactPoints == MaxContacts)
			return true;

	}


	ClosestPtPointTriangle(capsule.Start(), a, b, c, closestStart);
	ClosestPtPointSegment(closestStart, capsule.Start(), capsule.End(), closestStart2);

	const Maths::HVec3 difB(closestStart - closestStart2);
	const Maths::HF32 _disB(difB | difB);
	const Maths::HF32 disB(difB | patch.m_Normal);
	if(_disB < radSq)
	{
		//const Maths::HVec3 dir(closestStart - closestStart2);
		//const Maths::HF32 sign(Maths::Sign(dir | patch.m_Normal));
		C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point.m_ContactPoint = closestStart;
		point.m_HashValue = (triangleStartIndex + triangleIndex) << 2 | 1;
		point.m_PhysicsMaterialIndices = physicsMaterial;
		point.m_EffectMaterialIndices = effectMaterial;
		const Maths::HF32 pDepth( capsule.Radius() - disB );
		Maths::Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth = capsule.Radius() - disB;
	}
	
	if(patch.m_NumContactPoints == MaxContacts)
		return true;

	ClosestPtPointTriangle(capsule.End(), a, b, c, closestEnd);
	ClosestPtPointSegment(closestEnd, capsule.Start(), capsule.End(), closestEnd2);

	const Maths::HVec3 difC(closestEnd - closestEnd2);
	const Maths::HF32 _disC(difC | difC);

	const Maths::HF32 disC(difC | patch.m_Normal);
	if(_disC < radSq)
	{
		C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point.m_ContactPoint = closestEnd;
		point.m_HashValue = (triangleStartIndex + triangleIndex) << 2 | 2;
		point.m_PhysicsMaterialIndices = physicsMaterial;
		point.m_EffectMaterialIndices = effectMaterial;
		const Maths::HF32 pDepth(capsule.Radius() - disC);
		Maths::Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth = capsule.Radius() - disC;
	}
	return true;
}




template <int MaxContacts>
bool CollideCapsuleTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF norm, Collision::C_CapsuleProxy& proxy, C_ContactPatch<MaxContacts>& patch,
							  const U32 physicsMaterial, const U32 effectMaterial, const U32 triangleStartIndex, const U32 triangleIndex)
{
	Collision::C_Capsule& capsule = proxy.m_Capsule;

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 dA(a | norm);
	const Maths::HF32 dS((capsule.Start() | norm) - dA);
	const Maths::HF32 dE((capsule.End() | norm) - dA);
	const Maths::HF32 absDs(Maths::Abs(dS));
	const Maths::HF32 absDE(Maths::Abs(dE));

	
	

	if(Maths::Abs(dS * dE).GE(zero) & absDs.GE(capsule.Radius()) & absDE.GE(capsule.Radius()))
		return false;

	Maths::HVec3 closestA(Maths::UNDEFINED), closestB(Maths::UNDEFINED),
		closestStart(Maths::UNDEFINED), closestEnd(Maths::UNDEFINED),
		closestStart2(Maths::UNDEFINED), closestEnd2(Maths::UNDEFINED);
	Maths::HF32 sqDist(Maths::UNDEFINED);

	//Two contacts to find - deepest and point of first intersection!. Firstly, find point of first 
	//intersection.

	ClosestPtSegmentTriangle(capsule.m_Start, capsule.m_End, a, b, c, closestA, closestB, sqDist);
	if(sqDist > capsule.Radius()*capsule.Radius()) 
		return false;

	const Maths::HF32 disA(sqDist.Sqrt());
	{
		C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point.m_ContactPoint = closestA * proxy.m_PosRot;
		point.m_HashValue = (triangleStartIndex + triangleIndex) << 2;
		point.m_PhysicsMaterialIndices = physicsMaterial;
		point.m_EffectMaterialIndices = effectMaterial;
		const Maths::HF32 pDepth(capsule.Radius() - disA);
		Maths::Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth = capsule.Radius() - disA;
	}

	if(patch.m_NumContactPoints == MaxContacts)
		return true;


	ClosestPtPointTriangle(capsule.Start(), a, b, c, closestStart);
	ClosestPtPointSegment(closestStart, capsule.Start(), capsule.End(), closestStart2);

	const Maths::HF32 disB((closestStart - closestStart2).Length());
	if(disB < capsule.Radius())
	{
		C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point.m_ContactPoint = closestStart * proxy.m_PosRot;
		point.m_HashValue = (triangleStartIndex + triangleIndex) << 2 | 1;
		point.m_PhysicsMaterialIndices = physicsMaterial;
		point.m_EffectMaterialIndices = effectMaterial;
		const Maths::HF32 pDepth(capsule.Radius() - disB);
		Maths::Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth = capsule.Radius() - disB;
	}
	
	if(patch.m_NumContactPoints == MaxContacts)
		return true;

	ClosestPtPointTriangle(capsule.End(), a, b, c, closestEnd);
	ClosestPtPointSegment(closestEnd, capsule.Start(), capsule.End(), closestEnd2);

	const Maths::HF32 disC((closestEnd - closestEnd2).Length());
	if(disC < capsule.Radius())
	{
		C_ContactPoint& point = patch.m_ContactPoints[patch.m_NumContactPoints++];
		point.m_ContactPoint = closestEnd * proxy.m_PosRot;
		point.m_HashValue = (triangleStartIndex + triangleIndex) << 2 | 2;
		point.m_PhysicsMaterialIndices = physicsMaterial;
		point.m_EffectMaterialIndices = effectMaterial;
		const Maths::HF32 pDepth(capsule.Radius() - disC);
		Maths::Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth = capsule.Radius() - disC;
	}
	return true;
}



//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideCapsuleTriangleMesh(C_CapsuleProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, 
//								const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
//{
//	using namespace Maths;
//
//	bool collide=false;
//	const HF32 disEps(SharedPhysics::Constants::CloseNormal());
//	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
//	Collision::C_Capsule& capsule(proxy.m_Capsule);
//	const HMat33 rotation(proxy.m_PosRot.Rotation());
//	const HVec3 position(proxy.m_PosRot.Translation());
//	const HVec3 capsuleMiddle((capsule.m_Start + capsule.m_End) * Maths::Constants<Maths::HF32>::Half());
//	Maths::C_Normal resultNormal(Maths::UNDEFINED);
//	Maths::HVec3 contacts[2];
//	Maths::HF32 pens[2];
//	const HVec3 range(high - low);
//	MathsStorage::HenDN3 dNorm;
//	for(U32 i=0; i<numTris; ++i)
//	{
//		Spu::MemBaseMarker marker;
//		//const Triangle* tempTri=(const Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
//		const Triangle* tempTri = &tris[i];
//		//((U32&)dNorm) = Spu::Get( (U32*)normals + tempTri->m_NormalIndex);
//		dNorm = tempTri->m_SurfaceNormal;
//	/*	MathsStorage::Float3* fA = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tempTri->m_IndexA, sizeof(MathsStorage::Float3));
//		MathsStorage::Float3* fB = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tempTri->m_IndexB, sizeof(MathsStorage::Float3));
//		MathsStorage::Float3* fC = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tempTri->m_IndexC, sizeof(MathsStorage::Float3));
//		const Maths::HVec3 a=Load<HVec3, MathsStorage::Float3>(fA);
//		const Maths::HVec3 b=Load<HVec3, MathsStorage::Float3>(fB);
//		const Maths::HVec3 c=Load<HVec3, MathsStorage::Float3>(fC);*/
//
//		const HVec3 localA(&verts[tempTri->m_IndexA ]);
//		const HVec3 localB(&verts[tempTri->m_IndexB ]);
//		const HVec3 localC(&verts[tempTri->m_IndexC ]);
//		const HVec3 a = localA*range + low;
//		const HVec3 b = localB*range + low;
//		const HVec3 c = localC*range + low;
//
//		C_Normal norm = C_Normal(-Load<HVec3, MathsStorage::HenDN3>(&dNorm), C_Normal::UNIT_LENGTH_VECTOR);
//
//		//const HVec3 transNorm(norm * rotation);
//
//		const Maths::HF32 dotA(a | norm);
//		const Maths::HF32 dotCentre((capsule.Centre() | norm) - dotA);
//		if(dotCentre >= zero)
//			continue;
//
//		U32 numContacts = CollideCapsuleTriangle(a, b, c, norm, capsule, resultNormal, contacts, pens);
//
//		if(numContacts > 0)
//		{
//			resultNormal = resultNormal * proxy.m_PosRot.Rotation();
//			collide = true;
//			C_ContactPatch<MaxContacts>* pPatch = NULL;
//			S32 incr = 0 ;
//			
//			if(results.m_NumContactPatches < MaxPatches)
//			{
//				pPatch = &results.m_ContactPatches[results.m_NumContactPatches];
//				pPatch->m_Normal = resultNormal;//norm;
//				pPatch->m_NumContactPoints = 0;
//				incr = 1;
//			}
//			
//			for(U32 j = 0; j < results.m_NumContactPatches; ++j)
//			{
//				Maths::HF32 dis = results.m_ContactPatches[j].m_Normal.m_V | resultNormal;//norm;
//				if(dis >= disEps)
//				{
//					pPatch = &results.m_ContactPatches[j];
//					incr = 0;
//					break;
//				}
//			}
//
//			results.m_NumContactPatches += incr;
//
//			if(pPatch != NULL)
//			{
//				for(U32 a = 0; a < numContacts; ++a)
//				{
//					if(pPatch->m_NumContactPoints == MaxContacts)
//						break;
//					C_ContactPoint& point = pPatch->m_ContactPoints[pPatch->m_NumContactPoints++];
//					point.m_ContactPoint = contacts[a] * proxy.m_PosRot;
//					//point.m_PenetrationDepth = pens[a];
//					Maths::Store(pens[a], &point.m_PenetrationDepth);
//					point.m_PhysicsMaterialIndices = physicsMaterial;
//					point.m_EffectMaterialIndices = effectMaterial;
//					point.m_HashValue = ((triangleStartIndex + i) << 16) | a;
//				}
//			}
//		}
//	}
//
//	return collide;
//
//}

//template <int MaxContacts>
//bool IntersectTriangleCapsule(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
//							  const Maths::C_Normal PCREF norm, Collision::C_Capsule& capsule, C_ContactPatch<MaxContacts>& patch,
//							  const U32 materialIndex, const U32 triangleStartIndex, const U32 triangleIndex)
//{
//	using namespace Maths;
//	HVec3 closestA(Maths::UNDEFINED);
//	HVec3 closestB(Maths::UNDEFINED);
//	HF32 sqDist(Maths::UNDEFINED);
//	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
//	const HF32 r(capsule.m_Radius);
//
//	Maths::HVec3 contact;
//	bool intersect=IntersectTriangleSegment(a ,b, c, capsule.m_Start,capsule.m_End, contact);
//	if(intersect)
//	{
//		
//		const HF32 zero(Constants<Maths::HF32>::Zero());
//		const HF32 d(a | patch.m_Normal);
//
//
//		const HF32 projA((capsule.m_Start | patch.m_Normal) - d);
//		const HF32 projB((capsule.m_End | patch.m_Normal) - d);
//
//		//const Maths::HVec3 support(Maths::Select(projA.LT(zero), capsule.m_Start, capsule.m_End) - normal * capsule.m_Radius);
//
//		const HF32 penetration(capsule.m_Radius - Maths::Min(Maths::Abs(projA), Maths::Abs(projB)));
//
//		//results.m_NumContactPatches += incr;
//
//		C_ContactPoint& point=patch.m_ContactPoints[patch.m_NumContactPoints++];
//		point.m_ContactPoint = contact;
//		point.m_PenetrationDepth=penetration;
//		point.m_MaterialIndices = materialIndex;
//		point.m_HashValue = triangleStartIndex + triangleIndex;
//		
//		
//	}
//	else
//	{
//
//		ClosestPtSegmentTriangle(capsule.m_Start, capsule.m_End, a, b, c, closestA, closestB, sqDist);
//		if(sqDist > r*r) 
//			return false;
//
//		//results.m_NumContactPatches += incr;
//	
//		//collide=true;
//		const Maths::HF32 length(sqDist.Sqrt());
//		//const C_Normal normal((closestA- closestB)/length, Maths::C_Normal::UNIT_LENGTH_VECTOR);
//		
//		const HF32 pDepth(r-length);
//		//const HVec3 contactPoint(closestB);
//
//		C_ContactPoint& point=patch.m_ContactPoints[patch.m_NumContactPoints++];
//		point.m_ContactPoint = closestB;
//		point.m_PenetrationDepth=pDepth;
//		assert(pDepth > zero);
//		point.m_MaterialIndices = materialIndex;
//		point.m_HashValue = triangleStartIndex + triangleIndex;
//	}
//	return true;
//}


template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCapsuleTriangleMesh(C_CapsuleProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
	const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 physicsMaterial, const U32 effectMaterial,
	const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
{



	using namespace Maths;

	Spu::MemBaseMarker outerMarker;
	bool collide=false;
	const HF32 disEps(SharedPhysics::Constants::CloseNormal());
	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	//C_CapsuleShape* capsuleShape = (C_CapsuleShape*)Spu::FetchAlloc(shape, sizeof(C_CapsuleShape));
	C_Capsule& capsule=proxy.m_Capsule;
	const Maths::HVec3 capsuleMiddle((capsule.Start() + capsule.End()) * Maths::Constants<Maths::HF32>::Half());

	const Maths::HMat33 orientation(proxy.m_PosRot.Rotation());

	MathsStorage::HenDN3 dNorm;

	Maths::HVec3 contacts[2];
	Maths::HF32 pens[2];
	Maths::C_Normal resultNormal(Maths::UNDEFINED);

	const HVec3 range(high - low);

	for(U32 i=0; i<numTris; ++i)
	{
		Spu::MemBaseMarker marker;
		//Triangle* tri = (Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
		const Triangle* tri = &tris[i];
		//((U32&)dNorm) = Spu::Get((U32*)(normals + tri->m_NormalIndex));
		dNorm = tri->m_SurfaceNormal;
		const C_Normal norm(Load<HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);

		const HVec3 localA(&verts[tri->m_IndexA ]);
		const HVec3 localB(&verts[tri->m_IndexB ]);
		const HVec3 localC(&verts[tri->m_IndexC ]);
		const HVec3 a(localA*range + low);
		const HVec3 b(localB*range + low);
		const HVec3 c(localC*range + low);

		resultNormal = -norm;
		const Maths::HVec3 transformedNormal(resultNormal * orientation);

		{
			//collide = true;
			C_ContactPatch<MaxContacts>* pPatch = NULL;
			S32 incr = 0 ;
			
			if(results.m_NumContactPatches < MaxPatches)
			{
				pPatch = &results.m_ContactPatches[results.m_NumContactPatches];
				pPatch->m_Normal = resultNormal * orientation;//norm;
				pPatch->m_NumContactPoints = 0;
				incr = 1;
			}
			
			for(U32 j = 0; j < results.m_NumContactPatches; ++j)
			{
				Maths::HF32 dis = results.m_ContactPatches[j].m_Normal.m_V | transformedNormal;//norm;
				if(dis >= disEps)
				{
					pPatch = &results.m_ContactPatches[j];
					incr = 0;
					break;
				}
			}

			if(pPatch != NULL)
			{
				Maths::C_Normal normal = norm;

				U32 contactCount = pPatch->m_NumContactPoints;
				bool bNewCollide = CollideCapsuleTriangle<MaxContacts>(a, b, c, normal, capsule, *pPatch,
				physicsMaterial, effectMaterial, triangleStartIndex, i);

				for(U32 a = contactCount; a < pPatch->m_NumContactPoints; ++a)
				{
					pPatch->m_ContactPoints[a].m_ContactPoint = pPatch->m_ContactPoints[a].m_ContactPoint * proxy.m_PosRot;
				}


				collide = bNewCollide || collide;
				results.m_NumContactPatches += (bNewCollide && incr);
			}

		}
	}

	return collide;


	//using namespace Maths;
	//C_Capsule& convex(proxy.m_Capsule);
	////Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
	//const HF32 zero(Constants<HF32>::Zero());
	////const Maths::HF32 third(Maths::Constants<Maths::HF32>::Third());
	//const HF32 twenty(SharedPhysics::Constants::Twenty());
	//C_Triangle triangle;
	//triangle.Margin(zero);//convex.Margin());
	//const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
	//bool bResult(false);
	//HVec3 contact(UNDEFINED);
	//HVec3 normal(UNDEFINED);
	//HF32 pen(Maths::UNDEFINED);
	//const HF32 margin(convex.Margin());
	//MathsStorage::HenDN3 dNorm;

	//const HMat33 orientation(proxy.m_PosRot.Rotation());
	////const Maths::HVec3 centre(convex.Centre());
	//const HVec3 range(high - low);
	//for(U32 a = 0; a < numTris; ++a)
	//{
	//	//Spu::MemBaseMarker marker;
	//	//const Triangle* tri((Triangle*)Spu::FetchAllocUnaligned(&tris[a], sizeof(Triangle)));
	//	const Triangle* tri =&tris[a];
	//	const HVec3 localA(&verts[tri->m_IndexA ]);
	//	const HVec3 localB(&verts[tri->m_IndexB ]);
	//	const HVec3 localC(&verts[tri->m_IndexC ]);
	//	triangle.m_VertexA = localA*range + low;
	//	triangle.m_VertexB = localB*range + low;
	//	triangle.m_VertexC = localC*range + low;
	//	dNorm = tri->m_SurfaceNormal;
	//	triangle.m_Normal = (-Maths::Load<Maths::HVec3>(&dNorm));

	//	const HF32 dotA(triangle.m_VertexA | triangle.m_Normal);
	//	
	//	const HVec3 supportNow(convex.SupportMargin(triangle.m_Normal, margin));
	//	const HF32 dotNow(((supportNow |triangle.m_Normal) + margin) - dotA);

	//	//triangle.m_Extruded = ((triangle.m_VertexA + triangle.m_VertexB + triangle.m_VertexC) * third) + 
	//	//	triangle.m_Normal * Maths::Constants<Maths::HF32>::Twenty();

	//	triangle.m_Extrusion = zero;
	//	
	//	triangle.WarmStart(triangle.m_VertexA);
	//	//triangle.WarmStart(localA);

	//	if(GJKCPairMargin<C_Capsule, C_Triangle>(convex, triangle , contact, normal, pen, *res.m_pEpa))
	//	{
	//		bResult = true;
	//		//Store this contact into the collision result...
	//		C_ContactPatch<MaxContacts>* pPatch(NULL);
	//		S32 incr = 1;

	//		const Maths::C_Normal norm(triangle.m_Normal * orientation, Maths::C_Normal::UNIT_LENGTH_VECTOR);

	//		if(res.m_NumContactPatches < MaxPatches)
	//		{
	//			pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
	//			pPatch->m_Normal = norm;//Maths::C_Normal(triangle.m_Normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	//			pPatch->m_NumContactPoints = 0;
	//		}

	//		for(U32 b = 0; b < res.m_NumContactPatches; ++b)
	//		{
	//			C_ContactPatch<MaxContacts>& patch(res.m_ContactPatches[b]);
	//			const Maths::HF32 d(patch.m_Normal.m_V | norm);
	//			if(d >= closeNormal)
	//			{
	//				incr = 0;
	//				pPatch = &res.m_ContactPatches[b];
	//				break;
	//			}
	//		}
	//		if(pPatch == NULL)
	//			continue;
	//		res.m_NumContactPatches += incr;
	//		//Stuff contact in there...
	//		if(pPatch->m_NumContactPoints < MaxContacts)
	//		{
	//			C_ContactPoint& point(pPatch->m_ContactPoints[pPatch->m_NumContactPoints++]);
	//			point.m_ContactPoint = contact * proxy.m_PosRot;
	//			const Maths::HF32 norDir(normal | triangle.m_Normal);
	//			//point.m_PenetrationDepth = Maths::Abs(norDir * pen);
	//			//point.m_PenetrationDepth = Maths::Max(dotNow, zero);
	//			//point.m_PenetrationDepth = Maths::Max((normal | -triangle.m_Normal) * pen, zero);
	//			const Maths::HF32 pDepth = Maths::Max(Maths::Max((normal | -triangle.m_Normal) * pen, dotNow), zero);
	//			Maths::Store(pDepth, &point.m_PenetrationDepth);
	//			//point.m_PenetrationDepth = Maths::Max(Maths::Max((normal | -triangle.m_Normal) * pen, dotNow), zero);
	//			point.m_HashValue = triangleStartIndex + a;
	//			point.m_PhysicsMaterialIndices = physicsMaterial;
	//			point.m_EffectMaterialIndices = effectMaterial;
	//		}
	//	}
	//	
	//}
	//return bResult;
}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCapsuleTriangleMesh(C_Shape* shape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, 
								const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
{
	using namespace Maths;

	Spu::MemBaseMarker outerMarker;
	bool collide=false;
	const HF32 disEps(SharedPhysics::Constants::CloseNormal());
	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	//C_CapsuleShape* capsuleShape = (C_CapsuleShape*)Spu::FetchAlloc(shape, sizeof(C_CapsuleShape));
	C_CapsuleShape* capsuleShape = (C_CapsuleShape*)shape;
	C_Capsule& capsule=capsuleShape->m_Capsule;
	const Maths::HVec3 capsuleMiddle((capsule.Start() + capsule.End()) * Maths::Constants<Maths::HF32>::Half());
	MathsStorage::HenDN3 dNorm;

	Maths::HVec3 contacts[2];
	Maths::HF32 pens[2];
	Maths::C_Normal resultNormal(Maths::UNDEFINED);

	const HVec3 range(high - low);

	for(U32 i=0; i<numTris; ++i)
	{
		Spu::MemBaseMarker marker;
		//Triangle* tri = (Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
		const Triangle* tri = &tris[i];
		//((U32&)dNorm) = Spu::Get((U32*)(normals + tri->m_NormalIndex));
		dNorm = tri->m_SurfaceNormal;
		const C_Normal norm(Load<HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
		//assert((norm | norm) > zero);
	/*	MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexA, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexB, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexC, sizeof(MathsStorage::Float3));
		const Maths::HVec3 a=Load<HVec3, MathsStorage::Float3>(f0);
		const Maths::HVec3 b=Load<HVec3, MathsStorage::Float3>(f1);
		const Maths::HVec3 c=Load<HVec3, MathsStorage::Float3>(f2);*/

		const HVec3 localA(&verts[tri->m_IndexA ]);
		const HVec3 localB(&verts[tri->m_IndexB ]);
		const HVec3 localC(&verts[tri->m_IndexC ]);
		const HVec3 a(localA*range + low);
		const HVec3 b(localB*range + low);
		const HVec3 c(localC*range + low);

		/*const HF32 dotA(a | norm);
		const HF32 dotMiddle((capsule.Centre() | norm) - dotA);
		if(dotMiddle < zero)
			continue;*/

		resultNormal = -norm;

		//Get contact normal...

		//U32 numContacts = CollideCapsuleTriangle(a, b, c, norm, capsule, resultNormal, contacts, pens);

		//if(numContacts > 0)
		{
			//collide = true;
			C_ContactPatch<MaxContacts>* pPatch = NULL;
			bool incr = false ;
			
			if(results.m_NumContactPatches < MaxPatches)
			{
				pPatch = &results.m_ContactPatches[results.m_NumContactPatches];
				pPatch->m_Normal = resultNormal;//norm;
				pPatch->m_NumContactPoints = 0;
				incr = true;
			}
			
			for(U32 j = 0; j < results.m_NumContactPatches; ++j)
			{
				Maths::HF32 dis = results.m_ContactPatches[j].m_Normal.m_V | resultNormal;//norm;
				if(dis >= disEps)
				{
					pPatch = &results.m_ContactPatches[j];
					incr = false;
					break;
				}
			}

			//results.m_NumContactPatches += incr;

			//if(pPatch != NULL)
			//{
			//	for(U32 a = 0; a < numContacts; ++a)
			//	{
			//		if(pPatch->m_NumContactPoints == MaxContacts)
			//			break;
			//		C_ContactPoint& point = pPatch->m_ContactPoints[pPatch->m_NumContactPoints++];
			//		point.m_ContactPoint = contacts[a];
			//		//point.m_PenetrationDepth = pens[a];
			//		Store(pens[a], &point.m_PenetrationDepth);
			//		point.m_PhysicsMaterialIndices = physicsMaterial;
			//		point.m_EffectMaterialIndices = effectMaterial;
			//		point.m_HashValue = ((triangleStartIndex + i) << 16) | a;
			//	}
			//}

			if(pPatch != NULL)
			{
				Maths::C_Normal normal = norm;

				bool bNewCollide = CollideCapsuleTriangle<MaxContacts>(a, b, c, normal, capsule, *pPatch,
				physicsMaterial, effectMaterial, triangleStartIndex, i);
				collide = bNewCollide || collide;
				results.m_NumContactPatches += (bNewCollide & incr);
			}

		}
	}

	return collide;

}


template <class Triangle>
bool ContinuousCapsuleTriangleMesh(C_Shape* shape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, 
								   const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB,  const Maths::HVec3& low, const Maths::HVec3& high)
{
	using namespace Maths;

	Spu::MemBaseMarker outerMarker;
	bool collide=false;

	C_ContinuousShape* continuousShape = (C_ContinuousShape*)Spu::FetchAlloc(shape, sizeof(C_ContinuousShape));
	C_CapsuleShape* capsuleShape = (C_CapsuleShape*)Spu::FetchAlloc(continuousShape->m_pContinuousPrimitive, sizeof(C_CapsuleShape));
	C_Capsule& capsule=capsuleShape->m_Capsule;

	MathsStorage::HenDN3 dNorm;
	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 range(high - low);

	for(U32 i=0; i<numTris; ++i)
	{
		Spu::MemBaseMarker marker;
		//Triangle* tri = (Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
		const Triangle* tri=&tris[i];
		//((U32&)dNorm) = Spu::Get((U32*)(normals + tri->m_NormalIndex));
		dNorm = tri->m_SurfaceNormal;
		const C_Normal norm(-Load<HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
		BIZ_ASSERT((norm | norm) > zero);
		/*MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexA, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexB, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexC, sizeof(MathsStorage::Float3));
		
		Collision::C_Triangle triangle;
		triangle.m_VertexA=Load<HVec3, MathsStorage::Float3>(f0);
		triangle.m_VertexB=Load<HVec3, MathsStorage::Float3>(f1);
		triangle.m_VertexC=Load<HVec3, MathsStorage::Float3>(f2);*/

		const HVec3 localA(&verts[tri->m_IndexA ]);
		const HVec3 localB(&verts[tri->m_IndexB ]);
		const HVec3 localC(&verts[tri->m_IndexC ]);

		Collision::C_Triangle triangle;
		triangle.m_VertexA = localA*range + low;
		triangle.m_VertexB = localB*range + low;
		triangle.m_VertexC = localC*range + low;

		const C_Normal normal(-norm);
		//lambda = zero;

		if( Collision::GJKRayCast<Collision::C_Capsule, Collision::C_Triangle>(capsule, triangle, r, maxLambda, lambda, nor, closestA, closestB))
		{
			nor = norm;
			maxLambda = lambda;
			collide = true;
		}
	}

	return collide;
}


};

#endif

