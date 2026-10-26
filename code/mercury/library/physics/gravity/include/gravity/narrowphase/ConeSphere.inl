#ifndef CONESPHERE_INL
#define CONESPHERE_INL

#include "gravity/narrowphase/primitivetest.h"

namespace Collision
{
//Called by the particle system collide with the dynamic objects
bool inline CollideConeSphere(const C_Cone& cone, const C_Sphere& sphere, Maths::HVec3& contact, Maths::HF32& pDepth, Maths::C_Normal& normal)
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 sphereCenter(sphere.Centre());
	const HVec3 coneTip(cone.Tip());
	const HVec3 coneBottom(cone.Bottom());
	const HF32 r(sphere.Radius());
	const HF32 sqR(r*r);

	const HVec3 st(sphereCenter - coneTip);
	const HF32 dTip(st|cone.MidPointAxis());
	if(dTip.GT(zero))
	{
		const HF32 sqSt(st|st);
		if(sqSt.GT(sqR))
			return false;
	}

	HVec3 closestP1(Maths::UNDEFINED);
	Collision::ClosestPtPointLine(sphereCenter, coneTip, coneBottom, closestP1);
	const HVec3 w(closestP1 - sphereCenter);
	/*
		the intersect point x in
		the line formular : x = sphereCenter + tw
	*/
	const HVec3 n(sphereCenter - coneTip);
	const HVec3 m(sphereCenter - closestP1);
	const HF32 sqTheta(cone.SinTheta()*cone.SinTheta());
	const HF32 sqW(w|w);
	const HF32 nw(n|w);
	const HF32 mw(m|w);
	const HF32 nn(n|n);
	const HF32 mm(m|m);
	const HF32 a(sqW*sqTheta - sqW);
	const HF32 b(nw*sqTheta - mw);
	const HF32 c(nn*sqTheta - mm);

	const HF32 discriminants(b*b - a*c);
	if(discriminants.LT(zero))
	{
		return false;
		//might has contact on the bottom
		const HVec3 nor(-cone.MidPointAxis());
		const HF32 signDist(nor|sphereCenter);
		const HF32 d(coneBottom|nor);
		const HF32 dist(signDist - d);
		if(dist.GT(r))
			return false;
		contact = sphereCenter + cone.MidPointAxis()*dist;
		pDepth = r - dist;
		//normal = nor;
		normal = cone.MidPointAxis();
		return true;
	}
	

	const HF32 temp(discriminants.Sqrt());
	const HF32 recipA(a.Reciprocal());
	const HF32 tmin((-b - temp)*recipA);
	const HF32 tmax((-b + temp)*recipA);

	const HF32 t=Min(tmin, tmax);

	const HVec3 intersectP(sphereCenter + t*w);

	const HF32 length(cone.BaseRadius()/cone.SinTheta());

	const HVec3 u(intersectP - coneTip);
	const HF32 uLength(u.Length());
	const HVec3 nor(u/uLength);
	const HVec3 endP(coneTip + length*nor);

	if(uLength.GT(length))
	{
		const HVec3 temp(sphereCenter - endP);
		const HF32 sqTemp(temp|temp);
		if(sqTemp.GT(sqR))
			return false;
		contact = endP;
		pDepth = r - sqTemp.Sqrt();
		//normal = C_Normal(-temp);
		normal = temp;
		return true;
	}

	HVec3 closestP2(Maths::UNDEFINED);
	Collision::ClosestPtPointSegment(sphereCenter, coneTip, endP, closestP2);
	const HVec3 vv(closestP2 - sphereCenter);
	const HF32 sqDist(vv|vv);
	
	if(sqDist.GT(sqR))
		return false;

	//normal= C_Normal(vv);
	normal = C_Normal(-vv);
	contact = closestP2;
	pDepth = r - sqDist.Sqrt();

	return true;
}

template <int MaxContacts>
bool CollideConeSphere(const C_Cone& cone, const C_Sphere& sphere, C_ContactPatch<MaxContacts>& patch, const U32 materialIndices)
{
	return false;
}
};




#endif