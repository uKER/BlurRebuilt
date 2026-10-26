#ifndef _NEW_PRIMITIVE_INL
#define _NEW_PRIMITIVE_INL


#include "gravity/narrowphase/PrimitiveTest.h"


//using namespace Maths;

namespace Collision
{

NOALIAS __forceinline bool CollideSphereSphere(const C_Sphere& sphereA, const C_Sphere& sphereB)
{
	const Maths::HVec3 d(sphereB.Centre() - sphereA.Centre());
	const Maths::HF32 rad(sphereA.Radius() + sphereB.Radius());
	const Maths::HF32 d2(d | d);
	return d2 < (rad * rad);
}

NOALIAS inline void ClosestPtPointSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& closest)
{
	using namespace Maths;
	const HVec3 ab(b-a);
	//const HF32 recipAb = Reciprocal(ab|ab);
	const HF32 recipAb = (ab|ab).Reciprocal();
	const HF32 _t=((p-a)|ab)*recipAb;
	//const HF32 _t=((p-a)| ab)/(ab|ab);//Dot(p-a, ab)/Dot(ab, ab);
	const HF32 t=Clamp(_t, Constants<HF32>::Zero(), Constants<HF32>::One());
	closest=a+(ab*t);
}

NOALIAS inline void ClosestPtPointSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& closest, Maths::HF32& t)
{
	using namespace Maths;
	const HVec3 ab(b-a);
	const HF32 recipAb = Reciprocal(ab|ab);
	const HF32 _t=((p-a)|ab)*recipAb;
	//const HF32 _t=((p-a)| ab)/(ab|ab);//Dot(p-a, ab)/Dot(ab, ab);
	t=Clamp(_t, Constants<HF32>::Zero(),Constants<HF32>::One());
	closest=a+(ab*t);
}

NOALIAS inline void ClosestPtPointLine(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& closest)
{
	using namespace Maths;
	const HVec3 ab(b-a);
	const HF32 recipAb = Reciprocal(ab|ab);
	const HF32 t=((p-a)|ab)*recipAb;
	closest=a+(ab*t);
}


NOALIAS __forceinline void ClosestPtSegmentSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& c1, Maths::HVec3& c2)
{
	using namespace Maths;
	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	const HVec3 pq(q-p);
	const HVec3 ab(b-a);
	const HVec3 n(ab^pq);
	const HF32 sn((n|n).Reciprocal());
	const HVec3 pa(a-p);
	const HF32 d((pa|n)*sn);
	const HVec3 temp(pa-n*d);
	const HVec3 temp1(ab^temp);
	const HF32 _r((temp1|n)*sn);
	const HF32 r(Clamp(_r, zero, one));
	c1=p+pq*r;

	//const HVec3 ap(p-a);
	const HF32 v0((pq ^ pa)|n);
	const HF32 _v(v0 *sn);
	const HF32 v(Clamp(_v, zero, one));
	c2=a+v*ab;
}

NOALIAS inline void DistanceSegmentSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HF32& sqDist)
{
	using namespace Maths;

	const HVec3 pq(q-p);
	const HVec3 ab(b-a);
	const HVec3 n(ab^pq);
	const HVec3 pa(a-p);
	const HF32 v(pa|n);
	const HF32 sn((n|n).Reciprocal());
	const HF32 d(v*sn);
	const HF32 tempSqDist((v*v)*sn);

	const HVec3 temp(pa-n*d);
	const HVec3 temp1(ab^temp);
	const HF32 _r((temp1|n)*sn);
	Scalar_Bool con(_r > Constants<HF32>::Zero() && _r <Constants<HF32>::One());
	sqDist = Select(con, tempSqDist, Constants<HF32>::Max());
}
//
//NOALIAS inline bool IntersectSegmentSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b)
//{
//	using namespace Maths;
//	const HF32 zero(Constants<HF32>::Zero());
//	/*const HVec3 pq(q-p);
//	const HVec3 ab(b-a);
//	const HVec3 n(ab^pq);
//	const HVec3 pa(a-p);
//	const HF32 d(pa|n);
//	const HF32 sdis((d*d)/(n|n));*/
//	HF32 sdis(UNDEFINED);
//	DistanceSegmentSegment(p, q, a, b, sdis);
//	return sdis.LT(zero);
//}
//
//NOALIAS inline bool IntersectSegmentSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& res)
//{
//	using namespace Maths;
//	const HF32 zero(0.0001f);
//	HF32 sdis(UNDEFINED);
//	DistanceSegmentSegment(p, q, a, b, sdis);
//	if(sdis.LT(zero))
//	{
//		const HVec3 v1(b-a);
//		const HVec3 v2(q-p);
//		const HVec3 u(p-a);
//		const HF32 uX(u.X());
//		const HF32 uY(u.Y());
//		const HF32 v1X(v1.X());
//		const HF32 v1Y(v1.Y());
//		const HF32 v2X(v2.X());
//		const HF32 v2Y(v2.Y());
//		const HF32 demon = v1Y*v2X - v1X*v2Y;
//		const HF32 nmon = v1X*uY - v1Y*uX;
//		const HF32 t2 = nmon/demon;
//		res = p + t2*v2;
//		return true;
//
//	}
//	return false;
//}

NOALIAS inline bool IntersectSegmentPlane(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const C_Plane& p, Maths::HVec3& res)
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HVec3 ab(b-a);
	const HF32 temp1(p.m_n|a);
	const HF32 temp2(p.m_n|ab);
	const HF32 t((p.m_d - temp1)/temp2);

	if(t.GE(zero)& t.LE(one))
	{
		res=a + (ab*t);
		return true;
	}
	return false;
	
}

NOALIAS inline U32 IntersectRayPlane(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF dir, const C_Plane& p, Maths::HVec3& res)
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 temp1(p.m_n|a);
	const HF32 temp2(p.m_n|dir);
	const HF32 t((p.m_d - temp1)/temp2);

	if(t.GE(zero))
	{
		res=a + (dir*t);
		return true;
	}
	return false;
	
}

NOALIAS inline int IntersectSegmentPlane1(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const C_Plane& p, Maths::HVec3& res)
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HVec3 ab(b-a);
	const HF32 temp1(p.m_n|a);
	const HF32 temp2(p.m_n|ab);
	const HF32 t((p.m_d - temp1)/temp2);

	if(t.GE(zero)& t.LE(one))
	{
		res=a + (ab*t);
		return 0;
	}
	return 1;
	
}

NOALIAS inline int IntersectSegmentPlaneRadius(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, 
											   const C_Plane& p, const Maths::HF32 PCREF radius, 
											   Maths::HVec3& res, Maths::HVec3& planePoint)
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	//const Maths::HVec3 ab(b-a);
	const HF32 _temp1((p.m_n|a) - p.m_d);
	const HF32 temp1(_temp1 - radius);
	const HF32 _temp2((p.m_n|b) - p.m_d);
	const HF32 temp2(_temp2 - radius);

	if(temp1.LE(zero) && temp1.GE(-radius))
	{
		res = a;
		planePoint = a + p.m_n *temp1;
		return 1;
	}

	if((temp1 * temp2) >= zero)
		return 0;

	//const Maths::HF32 t((p.m_d - temp1)/temp2);
	const HF32 t(temp1/(temp1 - temp2));//temp2 - temp1));
	res=a + ((b - a)*t);
	const HF32 resDis((res | p.m_n) - p.m_d);
	planePoint = res - p.m_n * resDis;
	return 1;	
}

NOALIAS inline bool IntersectCapsuleSegment(const C_Capsule& capsule, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b)
{
	using namespace Maths;
	const HVec3 start(capsule.Start());
	const HVec3 end(capsule.End());
	const HF32 r(capsule.Radius());
	HF32 sdis(UNDEFINED);
	const HF32 sr(r*r);

	DistanceSegmentSegment(start, end, a, b, sdis);
	
	return sdis.LT(sr);
}



NOALIAS inline bool IntersectCapsuleCapsule(const C_Capsule& a, const C_Capsule& b)
{
	using namespace Maths;
	const HVec3 a_start(a.Start());
	const HVec3 a_end(a.End());
	//const HF32 a_r(a.r);

	const HVec3 b_start(b.Start());
	const HVec3 b_end(b.End());
	const HF32 tr(a.Radius()+b.Radius());

	HF32 sdis(UNDEFINED);
	DistanceSegmentSegment(a_start, a_end, b_start, b_end, sdis);
	
	return sdis.LT(tr*tr);
}

NOALIAS inline bool IntersectCapsuleSphere(const C_Capsule& capsule, const C_Sphere& sphere)
{
	using namespace Maths;
	HVec3 closest(Maths::UNDEFINED);
	const HF32 r(capsule.Radius()+sphere.Radius());
	
	ClosestPtPointSegment(sphere.Centre(), capsule.Start(), capsule.End(), closest);
	const HVec3 temp(sphere.Centre()-closest);
	const HF32 sqDist(temp|temp);
	return sqDist.LT(r*r);
}

NOALIAS inline U32 IntersectSegmentBox(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF extents, Maths::HVec3& minContacts, Maths::HVec3& maxContacts)
{
	using namespace Maths;
	const HVec3 d(q - p);
	const HVec3 ood(d.Reciprocal());
	const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon2());
	const HVec3 _max(Maths::Constants<HVec3>::One());
	const HVec3 _min(Maths::Constants<HVec3>::Zero());
	
	//Check to see if any of the extents are outside of the box...
	const HVec3 absD(Abs(d));
	const Vector3_Bool bSmall(absD < epsilon);
	const Vector3_Bool bOutside((bSmall) & ((p < -extents) | (p > extents)));
	
	const HVec3 _t1(((-extents) - p) * ood);
	const HVec3 _t2((extents - p) * ood);
	const Vector3_Bool vb(_t1 < _t2);
	const HVec3 t11(Select(vb, _t1, _t2));
	const HVec3 t21(Select(vb, _t2, _t1));

	const HVec3 t1(Select(bSmall, _min, Max(t11, _min)));
	const HVec3 t2(Select(bSmall, _max, Min(t21, _max)));

	const HF32 t1x(t1.X());
	const HF32 t1y(t1.Y());
	const HF32 t1z(t1.Z());
	const HF32 t2x(t2.X());
	const HF32 t2y(t2.Y());
	const HF32 t2z(t2.Z());


	const Scalar_Bool sb1(t1y.GT(t1z));
	const HF32 tmin_tmp(Select(sb1, t1y, t1z));
	const Scalar_Bool sb2(tmin_tmp.GT(t1x));
	const HF32 tmin(Select(sb2, tmin_tmp, t1x));

	const Scalar_Bool sb3(t2y.LT(t2z));
	const HF32 tmax_tmp=Select(sb3, t2y, t2z);
	const Scalar_Bool sb4(tmax_tmp.LT(t2x));
	const HF32 tmax=Select(sb4, tmax_tmp, t2x);
	
	//Work out what the contact point is...
	minContacts = p + d * tmin;
	maxContacts = p + d * tmax;
	const Scalar_Bool sb(X(bOutside) | Y(bOutside) | Z(bOutside));
	return ((tmin.LT(tmax)) & (!sb)) != 0;
}


NOALIAS inline bool IntersectSegmentBox(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF extents, Maths::HVec3& minContacts, Maths::HVec3& maxContacts, Maths::HF32& _tmin, Maths::HF32& _tmax )
{
	using namespace Maths;

	const HVec3 d(q-p);
	const HVec3 recipD=d.Reciprocal();
	const HVec3 _min(-extents);
	const HVec3 _max(extents);
	const HVec3 epsilon(Constants<HVec3>::Epsilon2());

	HVec3 tmin(Constants<HVec3>::Zero());
	HVec3 tmax(Constants<HVec3>::One());


	const Vector3_Bool smallVal(Abs(d) < epsilon);
	const Vector3_Bool outOfRange(p < _min | p > _max);
	const Vector3_Bool noHit(smallVal & outOfRange);
	const Scalar_Bool b(Maths::X(noHit) | Maths::Y(noHit) | Maths::Z(noHit));
	
	/*if(Abs(d).Any_LT(epison))
	{
		if(p.Any_LT(min) | p.Any_GT(max))
			return false;
	}
	else*/
	{		
		const HVec3 _t1=(_min - p)*recipD;
		const HVec3 _t2=(_max - p)*recipD;

		const Vector3_Bool con1(_t1 > _t2);
		const HVec3 t1=Select(con1, _t2, _t1);
		const HVec3 t2=Select(con1, _t1, _t2);

		const Vector3_Bool con2(t1 > tmin);
		tmin=Select(con2, t1, tmin);

		const Maths::Vector3_Bool con3(t2 < tmax);
		tmax=Select(con3, t2, tmax);

		/*if(tmin.Any_GT(tmax))
			return false;	*/
	}
	
	const HVec3 newTmin(Select(smallVal, Constants<HVec3>::Zero(), tmin));
	const HVec3 newTmax(Select(smallVal, Constants<HVec3>::One(), tmax));

	_tmin=Max(newTmin.X(), Max(newTmin.Y(), newTmin.Z()));
	_tmax=Min(newTmax.X(), Min(newTmax.Y(), newTmax.Z()));


	minContacts=p + d*_tmin;
	maxContacts=p + d*_tmax;

	
	return (_tmin.LE(_tmax) && (!(b)));
}

NOALIAS __forceinline Maths::Scalar_Bool IntersectRayAABBMinMax(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF min, const Maths::HVec3 PCREF max)
{
	using namespace Maths;

	const HVec3 vZero(Maths::Constants<Maths::HVec3>::Zero());
	const HVec3 vOne(Maths::Constants<Maths::HVec3>::One());
	const HVec3 epsilon(Constants<HVec3>::Epsilon2());

	const HVec3 d(q-p);
	const HVec3 recipD=d.Reciprocal();
	const Vector3_Bool smallVal(Abs(d) < epsilon);
	const Vector3_Bool outOfRange((p < min) | (p > max));
	const Vector3_Bool noHit(smallVal & outOfRange);
	const Scalar_Bool b(!(X(noHit) | Y(noHit) | Z(noHit)));

	const HVec3 _t1=(min - p)*recipD;
	const HVec3 _t2=(max - p)*recipD;

	const Vector3_Bool con1(_t1 > _t2);
	const HVec3 t1=Select(con1, _t2, _t1);
	const HVec3 t2=Select(con1, _t1, _t2);

	const HVec3 _tmin(Maths::Max(vZero, t1));
	const HVec3 _tmax(Maths::Min(t2, vOne));

	const HVec3 newTmin(Select(smallVal, vZero, _tmin));
	const HVec3 newTmax(Select(smallVal, vOne, _tmax));

	const HF32 tmax=Min(newTmax.X(), Min(newTmax.Y(), newTmax.Z()));
	const Maths::HF32 tmin(Max(newTmin.X(), Max(newTmin.Y(), newTmin.Z())));
	return (tmin.LT(tmax) & b);
}



NOALIAS __forceinline Maths::Scalar_Bool IntersectRayAABBMinMax(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF min, const Maths::HVec3 PCREF max, Maths::HF32& tmin)
{
	using namespace Maths;

	const HVec3 vZero(Maths::Constants<Maths::HVec3>::Zero());
	const HVec3 vOne(Maths::Constants<Maths::HVec3>::One());
	const HVec3 epsilon(Constants<HVec3>::Epsilon2());

	const HVec3 d(q-p);
	const HVec3 recipD=d.Reciprocal();
	const Vector3_Bool smallVal(Abs(d) < epsilon);
	const Vector3_Bool outOfRange((p < min) | (p > max));
	const Vector3_Bool noHit(smallVal & outOfRange);
	const Scalar_Bool b(!(X(noHit) | Y(noHit) | Z(noHit)));

	const HVec3 _t1=(min - p)*recipD;
	const HVec3 _t2=(max - p)*recipD;

	const Vector3_Bool con1(_t1 > _t2);
	const HVec3 t1=Select(con1, _t2, _t1);
	const HVec3 t2=Select(con1, _t1, _t2);

	const HVec3 _tmin(Maths::Max(vZero, t1));
	const HVec3 _tmax(Maths::Min(t2, vOne));

	const HVec3 newTmin(Select(smallVal, vZero, _tmin));
	const HVec3 newTmax(Select(smallVal, vOne, _tmax));

	const HF32 tmax=Min(newTmax.X(), Min(newTmax.Y(), newTmax.Z()));
	tmin=Max(newTmin.X(), Max(newTmin.Y(), newTmin.Z()));
	return (tmin.LT(tmax) & b);
}

NOALIAS inline bool IntersectRayAABB(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF extents, Maths::HF32& tmin)
{
	using namespace Maths;

	const HVec3 d(q-p);
	const HVec3 recipD=d.Reciprocal();
	const HVec3 _min(c-extents);
	const HVec3 _max(c+extents);
	const HVec3 epsilon(Constants<HVec3>::Epsilon2());

	HVec3 _tmin(Constants<HVec3>::Zero());
	HVec3 _tmax(Constants<HVec3>::One());


	const Vector3_Bool smallVal(Abs(d) < epsilon);
	const Vector3_Bool outOfRange(p < _min | p > _max);
	const Vector3_Bool noHit(smallVal & outOfRange);
	const Scalar_Bool b(X(noHit) | Y(noHit) | Z(noHit));
	
	
	const HVec3 _t1=(_min - p)*recipD;
	const HVec3 _t2=(_max - p)*recipD;

	const Vector3_Bool con1(_t1 > _t2);
	const HVec3 t1=Select(con1, _t2, _t1);
	const HVec3 t2=Select(con1, _t1, _t2);

	const Vector3_Bool con2(t1 > _tmin);
	_tmin=Select(con2, t1, _tmin);

	const Vector3_Bool con3(t2 < _tmax);
	_tmax=Select(con3, t2, _tmax);
	
	const HVec3 newTmin(Select(smallVal, Constants<HVec3>::Zero(), _tmin));
	const HVec3 newTmax(Select(smallVal, Constants<HVec3>::One(), _tmax));

	const HF32 tmax=Min(newTmax.X(), Min(newTmax.Y(), newTmax.Z()));
	tmin=Max(newTmin.X(), Max(newTmin.Y(), newTmin.Z()));

	
	return (tmin.LE(tmax) & (!(b)));
}

NOALIAS inline bool IntersectRayAABBInCell(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF c, 
										   const Maths::HVec3 PCREF extents, const Maths::HF32 PCREF inflation, 
										   const Collision::C_AxisBoxMinMax& containingBox, Maths::HF32& tmin)
{
	using namespace Maths;

	const Maths::HVec3 vZero(Constants<HVec3>::Zero());
	const Maths::HVec3 vOne(Constants<HVec3>::One());
	const HVec3 epsilon(Constants<HVec3>::Epsilon2());
	const HVec3 d(q-p);
	const HVec3 recipD=d.Reciprocal();
	const Maths::HVec3 vInflation(inflation);
	const Maths::HVec3 inflatedExtents(extents + vInflation);
	const HVec3 _min(c-inflatedExtents);
	const HVec3 _max(c+inflatedExtents);

	HVec3 _tmin(vZero);
	HVec3 _tmax(vOne);


	const Vector3_Bool smallVal(Abs(d) < epsilon);
	const Vector3_Bool outOfRange(p < _min | p > _max);
	const Vector3_Bool noHit(smallVal & outOfRange);
	const Scalar_Bool b(X(noHit) | Y(noHit) | Z(noHit));
	
	
	const HVec3 _t1=(_min - p)*recipD;
	const HVec3 _t2=(_max - p)*recipD;

	const Vector3_Bool con1(_t1 > _t2);
	const HVec3 t1=Select(con1, _t2, _t1);
	const HVec3 t2=Select(con1, _t1, _t2);

	const Vector3_Bool con2(t1 > _tmin);
	_tmin=Select(con2, t1, _tmin);

	const Vector3_Bool con3(t2 < _tmax);
	_tmax=Select(con3, t2, _tmax);
	
	const HVec3 newTmin(Select(smallVal, vZero, _tmin));
	const HVec3 newTmax(Select(smallVal, vOne, _tmax));

	const HF32 tmax=Min(newTmax.X(), Min(newTmax.Y(), newTmax.Z()));
	tmin=Max(newTmin.X(), Max(newTmin.Y(), newTmin.Z()));

	const Maths::HVec3 hitPoint(p + (tmin * d));
	
	const Maths::HVec3 localPt(hitPoint - c);
	const Maths::HVec3 clampedPosition(Maths::Max(-extents, Maths::Min(localPt, extents)) + c);

	return (tmin.LE(tmax) & (!(b)) & containingBox.Contains(clampedPosition));
}

NOALIAS inline bool IntersectRayAABBInCell(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF c, 
										   const Maths::HVec3 PCREF extents, const Maths::HF32 PCREF inflation, 
										   const Collision::C_AxisBoxMinMax& containingBox)
{
	using namespace Maths;

	const Maths::HVec3 vZero(Constants<HVec3>::Zero());
	const Maths::HVec3 vOne(Constants<HVec3>::One());
	const HVec3 epsilon(Constants<HVec3>::Epsilon2());
	const HVec3 d(q-p);
	const HVec3 recipD=d.Reciprocal();
	const Maths::HVec3 vInflation(inflation);
	const Maths::HVec3 inflatedExtents(extents + vInflation);
	const HVec3 _min(c-inflatedExtents);
	const HVec3 _max(c+inflatedExtents);

	const Vector3_Bool smallVal(Abs(d) < epsilon);
	const Vector3_Bool outOfRange(p < _min | p > _max);
	const Vector3_Bool noHit(smallVal & outOfRange);
	const Scalar_Bool b(!(X(noHit) | Y(noHit) | Z(noHit)));
	
	const HVec3 _t1=(_min - p)*recipD;
	const HVec3 _t2=(_max - p)*recipD;

	const Vector3_Bool con1(_t1 > _t2);
	const HVec3 t1=Select(con1, _t2, _t1);
	const HVec3 t2=Select(con1, _t1, _t2);

	const HVec3 _tmin(Maths::Max(t1, vZero));
	const HVec3 _tmax(Maths::Min(t2, vOne));
	const HVec3 newTmin(Select(smallVal, vZero, _tmin));
	const HVec3 newTmax(Select(smallVal, vOne, _tmax));

	const HF32 tmax(Min(newTmax.X(), Min(newTmax.Y(), newTmax.Z())));
	const Maths::HF32 tmin(Max(newTmin.X(), Max(newTmin.Y(), newTmin.Z())));
	const Maths::HVec3 hitPoint(p + (tmin * d));
	const Maths::HVec3 localPt(hitPoint - c);
	const Maths::HVec3 clampedPosition(Maths::Max(-extents, Maths::Min(localPt, extents)) + c);
	return (tmin.LE(tmax) & b & containingBox.Contains(clampedPosition));
}


NOALIAS inline U32 IntersectRayAABB(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF d, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF extents, Maths::HVec3& minContacts, Maths::HVec3& maxContacts)
{
	using namespace Maths;

	const HVec3 recipD=d.Reciprocal();
	const HVec3 _min(c-extents);
	const HVec3 _max(c+extents);
	const HVec3 epsilon(Constants<HVec3>::Epsilon2());

	HVec3 tmin(Constants<HVec3>::Zero());
	HVec3 tmax(Constants<HVec3>::One());


	const Vector3_Bool smallVal(Abs(d) < epsilon);
	const Vector3_Bool outOfRange(p < _min | p > _max);
	const Vector3_Bool noHit(smallVal & outOfRange);
	const Scalar_Bool b(X(noHit) | Y(noHit) | Z(noHit));
	
	/*if(Abs(d).Any_LT(epison))
	{
		if(p.Any_LT(min) | p.Any_GT(max))
			return false;
	}
	else*/
	{
		const HVec3 _t1=(_min - p)*recipD;
		const HVec3 _t2=(_max - p)*recipD;

		const Vector3_Bool con1(_t1 > _t2);
		const HVec3 t1=Select(con1, _t2, _t1);
		const HVec3 t2=Select(con1, _t1, _t2);

		const Vector3_Bool con2(t1 > tmin);
		tmin=Select(con2, t1, tmin);

		const Vector3_Bool con3(t2 < tmax);
		tmax=Select(con3, t2, tmax);

		/*if(tmin.Any_GT(tmax))
			return false;	*/
	}
	
	const HVec3 newTmin(Select(smallVal, Constants<HVec3>::Zero(), tmin));
	const HVec3 newTmax(Select(smallVal, Constants<HVec3>::One(), tmax));

	const HF32 _tmin=Max(newTmin.X(), Max(newTmin.Y(), newTmin.Z()));
	const HF32 _tmax=Min(newTmax.X(), Min(newTmax.Y(), newTmax.Z()));


	minContacts=p + d*_tmin;
	maxContacts=p + d*_tmax;

	
	return (_tmin.LE(_tmax) && (!(b)));
}

};

#endif
