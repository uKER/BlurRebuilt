#include <stdafx.h>
#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/PrimitiveTest.h"
//#include "gravity/narrowphase/PrimitiveTest.inl"
//#include "CapsuleCapsule.h"
//#include "CapsuleCapsule.inl"

namespace Collision
{

#ifndef PLATFORM_SPU
bool CollideCapsuleCapsule(const C_Capsule& a, const C_Capsule& b, Maths::HVec3& contact, Maths::C_Normal& normal, Maths::HF32& pDepth)
{
	using namespace Maths;

	const HVec3 a_start(a.Start());
	const HVec3 a_end(a.End());
	const HF32 a_r(a.Radius());

	const HVec3 b_start(b.Start());
	const HVec3 b_end(b.End());
	const HF32 b_r(b.Radius());

	const HVec3 pq(a_end-a_start);
	const HVec3 ab(b_end-b_start);
	const HVec3 n(ab^pq);
	const HF32 sn((n|n).Reciprocal());
	const HVec3 pa(b_start-a_start);
	const HF32 d(pa|n);
	
	const HF32 sdis((d*d)*sn);
	
	const HF32 tr(a_r+b_r);
	

	if(sdis.GT(tr*tr))
		return false;

	pDepth=tr-sdis.Sqrt();

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	
	const HF32 d1(d*sn);
	const HVec3 temp(pa-n*d1);
	const HVec3 temp1(ab^temp);
	const HF32 _r((temp1|n)*sn);
	const HF32 r(Clamp(_r, zero, one));
	const HVec3 closestA(a_start+pq*r);

	const HF32 v0((pq ^ pa)|n);
	const HF32 _v(v0 *sn);
	const HF32 v(Clamp(_v, zero, one));
	const HVec3 closestB(b_start+v*ab);
	
	normal=C_Normal(Normalise(closestB-closestA));
	contact=closestA+(normal * a_r);

	return true;
}

//bool CollideCapsuleSphere(const C_Capsule& cap, const C_Sphere& sphere, Maths::HVec3& contact, Maths::C_Normal& normal, Maths::HF32& pDepth)
//{
//	using namespace Maths;
//	const HVec3 c_start(cap.m_Start);
//	const HVec3 c_end(cap.m_End);
//	const HVec3 center(sphere.Centre());
//	const HF32 radius(sphere.Radius());
//	const HF32 r(radius+cap.m_Radius);
//	HVec3 closest;
//	ClosestPtPointSegment(center, c_start, c_end, closest);
//	const HVec3 temp(closest-center);
//	const HF32 sqDist(temp|temp);
//	if(sqDist > r*r)
//		return false;		//non collide
//
//	pDepth=r-sqDist.Sqrt();
//
//	normal=C_Normal(temp);
//	contact = center+(normal*radius);
//	
//	return true;
//}
#endif
bool CollideCapsuleTriangle(const C_Capsule& capsule, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& contact, Maths::C_Normal& normal, Maths::HF32& pDepth)
{
	using namespace Maths;

	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	HF32 sqDist(Maths::UNDEFINED);
	const HF32 r(capsule.Radius());
	const HF32 r2(r * r);

	ClosestPtSegmentTriangle(capsule.Start(), capsule.End(), a, b, c, closestA, closestB, sqDist);
	if(sqDist > r2)
		return false;

	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	
	pDepth=r-sqDist.Sqrt();
	normal=C_Normal((closestB - closestA)/pDepth);
	//normal=C_Normal(closestA-closestB);
	contact =closestB;

	return true;
}


};
