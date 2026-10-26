#ifndef CAPSULECAPSULE_INL
#define CAPSULECAPSULE_INL

//using namespace Maths;

namespace Collision
{

template <int MaxContacts>
bool CollideCapsuleCapsule(const C_Capsule& a, const C_Capsule& b, C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial)
{
	using namespace Maths;
	const HVec3 d1=a.End()-a.Start();
	const HVec3 d2=b.End()-b.Start();
	const HVec3 r=a.Start()-b.Start();
	const HF32 ae(d1|d1);
	const HF32 e(d2|d2);
	const HF32 f(d2|r);

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	HF32 s(Maths::ZERO_INIT);
	HF32 t(Maths::ZERO_INIT);
	//const HF32 eps(EPSILON);
	

	const HF32 c=d1|r;
	const HF32 be= d1|d2;
	const HF32 denom((ae*e) - (be*be));

	//if the segment is not parallel, compute closest point on L1 and L2 and clamp to segment S1, otherwise, s=0.f;
	const HF32 w(Clamp((be*f - c*e)/denom, zero, one));
	s = Select(denom.NE(zero), w, s);
	const HF32 _t((be*s + f)/e);
	t=Clamp(_t, zero, one);

	const HF32 s1(Clamp(-c/ae, zero, one));
	const HF32 s2(Clamp((be-c)/ae, zero, one));

	s = Select(_t.LT(zero), s1, Select(_t.GT(one), s2, s));
	//assert(s.GE(zero) && s.LE(one));
	//assert(t.GE(zero) && t.LE(one));
	const HVec3 closestA(a.Start() + d1*s);
	const HVec3 closestB(b.Start() + d2*t);

	const Maths::HVec3 ab(closestA - closestB);
	const Maths::HF32 dsq(ab | ab);
	const Maths::HF32 sumRad(a.Radius() + b.Radius());
	if(dsq > (sumRad * sumRad))
		return false;

	//OK - contact, so work out the point of contact and normal!

	const Maths::HF32 dis(dsq.Sqrt());
	const Maths::C_Normal norm(ab/dis, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	C_ContactPoint& point(patch.m_ContactPoints[0]);
	patch.m_Normal = -norm;
	patch.m_NumContactPoints = 1;
	point.m_ContactPoint = closestB + norm * b.Radius();
	const Maths::HF32 penDepth(sumRad - dis);
	Maths::Store(penDepth, &point.m_PenetrationDepth);
	//point.m_PenetrationDepth = sumRad - dis;
	point.m_HashValue = 1;
	point.m_PhysicsMaterialIndices = physicsMaterial;
	point.m_EffectMaterialIndices = effectMaterial;
	return true;
}


template <int MaxPatches, int MaxContacts>
bool CollideCapsuleCapsule2(const C_Capsule& a, const C_Capsule& b, C_CollisionResult<MaxPatches, MaxContacts>& results)
{
	using namespace Maths;

	const HVec3 a_start(a.m_Start);
	const HVec3 a_end(a.m_End);
	const HF32 a_r(a.m_Radius);

	const HVec3 b_start(b.m_Start);
	const HVec3 b_end(b.m_End);
	const HF32 b_r(b.m_Radius);

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

	const HF32 pDepth(tr-sdis.Sqrt());

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
	
	const C_Normal normal(closestB-closestA);
	const HVec3 contactPoint(closestA+(normal * a_r));


	results.m_NumContactPatches = 1;
	C_ContactPatch<MaxContacts>& patch=results.m_ContactPatches[0];
	patch.m_Normal= normal;
	patch.m_NumContactPoints=1;

	C_ContactPoint& point=patch.m_ContactPoints[0];
	point.m_ContactPoint = contactPoint;
	Maths::Store(pDepth, &point.m_PenetrationDepth);//point.m_PenetrationDepth=pDepth;
	return true;
}


};

#endif
