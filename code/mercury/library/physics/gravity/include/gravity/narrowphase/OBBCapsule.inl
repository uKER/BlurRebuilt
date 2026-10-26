#ifndef OBBCAPSULE_INL
#define OBBCAPSULE_INL

#include "gravity/narrowphase/PrimitiveTest.h"


namespace Collision
{
/*
	This method is based on the idea that if the projection points of capsule.start(a) and capsule.end(b) point is on the same surface,
	the closest point from the segment ab to the surface is eigther a or b(if the segment ab is parrallel to the surface, the closest point
	is the mid point between a and b). If a and b in different surface, pick the closest three edges of the box to the segment ab, calcuate
	the closest point, which will be the contact point.
*/
template <int MaxPatches, int MaxContacts>
bool CollideOBBCapsule1(const C_OBB& obb, const C_Capsule& capsule, C_CollisionResult<MaxPatches, MaxContacts>& results)
{
	using namespace Maths;

	const HVec3 localOrigin(Constants<HVec3>::Zero());
	const HF32 half(Constants<HF32>::Half());
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one (Constants<HF32>::One());
	const HVec3 dA(capsule.Start() - obb.Centre());
	const HVec3 dB(capsule.End() - obb.Centre());

	/*
	In the math libary, Mat*HVec in mathatical term need to be implemented as HVec*Mat
	Also HVec*InverseMat == Mat*HVec in this libary
	*/
	//the corresponding local space points for the capsule.start and capsule.end
	const HMat33& u(obb.Orientation());
	const HVec3 c(obb.Centre());
	const HVec3 startL(u * dA);//inverseMatrix * HVec3
	const HVec3 endL(u * dB);


	const HVec3 e(obb.Extent());
	const HVec3 ne(-e);

	//Get startL and endL's projection on to the box surface
	const HVec3 projAL(Max(ne, Min(startL, e)));
	const HVec3 projBL(Max(ne, Min(endL, e)));

	
	/*const Maths::HVec3 projAW(obb.c + projAL * obb.u);
	const Maths::HVec3 projBW(obb.c + projBL * obb.u);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(projAW, 0.5f, RGBA(0, 255, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(projBW, 0.5f, RGBA(0, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(projAW, projBW, RGBA(0, 0, 255, 255));*/


	//pick the mid point betwen projAL and projBL to determind whether pL is one of the surface point
	const HVec3 pL((projAL+projBL)*half);

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(obb.c + pL * obb.u, 0.5f, RGBA(255, 255, 255, 255));

	HVec3 contactPoint(UNDEFINED);
	C_Normal normal(UNDEFINED);
	HF32 penetrationDepth(UNDEFINED);

	const HVec3 epi(Constants<HVec3>::Epsilon2());

	const HVec3 con(e-Abs(pL));
	//if any component of pL is equal to the coresponding component of e, then chose the shortest projection between A and B
	if(con.Any_LE(epi))
	{
		//pL is one of the surface point

		const HVec3 da(startL - projAL);
		const HVec3 db(endL - projBL);

		const HF32 distA(da.Length());
		const HF32 distB(db.Length());

			//The capsule and obb isn't intersect
		if(distA.GT(capsule.m_Radius) & distB.GT(capsule.m_Radius))
			return false;


		const HVec3 condi(da^db);
		const HVec3 z(Constants<HVec3>::Zero());
		if(condi.All_EQ(z)) //da and db paralle in the same direction
		{
			//The contactPoint can be one of the three points: projAL, projBL and mid point between projAL and projBL
			const HVec3 v(projBL - projAL);
			const HVec3 w(capsule.End() - capsule.Start());
			Scalar_Bool b1(distA==distB);
			Scalar_Bool b2(distA > distB);
			
			const HF32 s1=Select(b1, half, zero);
			const HF32 s2=Select(b2, one, zero);

			const HF32 scalar(s1+s2);

			const HVec3 p1(projAL + v*scalar);
			const HVec3 p2(capsule.Start() + w*scalar);

			contactPoint= c + p1*u;

			const HVec3 d(p2 - contactPoint);
			penetrationDepth=capsule.Radius() - d.Length();
			normal=C_Normal(d);
		}
		else
		{
			
			Scalar_Bool con1(distA < distB);
			const HVec3 proj=Select(con1, projAL, projBL);
			HVec3 point;
			ClosestPtPointSegment(proj, startL, endL, point);
			contactPoint=c + proj*u;
			const HVec3 p(c + point*u);
			const HVec3 d(p - contactPoint);
			penetrationDepth=capsule.Radius() - d.Length();
			normal=Maths::C_Normal(d);
			
		}
	}
	else
	{
		//p is one of the internal point
		HVec3 sign(pL.X().Sign(), pL.Y().Sign(), pL.Z().Sign());

		//pick one of the conner point in the local space
		const HVec3 pickL(e*sign);

		HVec3 p1L(pickL);
		HVec3 p2L(pickL);
		HVec3 p3L(pickL);

		p1L.X(-pickL.X());
		p2L.Y(-pickL.Y());
		p3L.Z(-pickL.Z());

		/*const Maths::HVec3 pickW = obb.c + pickL *obb.u ;
		const Maths::HVec3 p1W=obb.c + p1L * obb.u;
		const Maths::HVec3 p2W=obb.c + p2L * obb.u;
		const Maths::HVec3 p3W=obb.c + p3L * obb.u;*/
		
		

		//Get the pair of closest points in the local space
		HVec3 c10(UNDEFINED);
		HVec3 c11(UNDEFINED);
		ClosestPtSegmentSegment(pickL, p1L, startL, endL, c10, c11);
		const HVec3 d1(c11-c10);
		const HF32 sqDist1(d1|d1);

		HVec3 c20(UNDEFINED);
		HVec3 c21(UNDEFINED);
		ClosestPtSegmentSegment(pickL, p2L, startL, endL, c20, c21);
		const HVec3 d2(c21-c20);
		const HF32 sqDist2(d2|d2);

		HVec3 c30(UNDEFINED);
		HVec3 c31(UNDEFINED);
		ClosestPtSegmentSegment(pickL, p3L, startL, endL, c30, c31);
		const HVec3 d3(c31-c30);
		const HF32 sqDist3(d3|d3);


		const HF32 sqR(capsule.Radius()*capsule.Radius());

		
		
		//the capsule and the obb doesn't intersect
		if(sqDist1.GT(sqR) & sqDist2.GT(sqR) & sqDist3.GT(sqR))
			return false;

		/*const Maths::Scalar_Bool con1(sqDist1.LT(sqDist2) & sqDist1.LT(sqDist3));
		const Maths::Scalar_Bool con2(sqDist2.LT(sqDist3));
		const Maths::HVec3 d=Select(con1, d1, Select(con2, d2, d3));
		const Maths::HVec3 c=Select(con1, c10, Select(con2, c20, c30));
		const Maths::HF32 s=Select(con1, sqDist1.Sqrt(), Select(con2, sqDist2.Sqrt(), sqDist3.Sqrt()));

		normal=Maths::C_Normal(d*obb.u);
		contactPoint=obb.c + c*obb.u;
		penetrationDepth=capsule.r - s;*/

		if(sqDist1.LT(sqDist2) & sqDist1.LT(sqDist3))
		{
			normal =C_Normal(d1*u);
			contactPoint=c + c10*u;
			penetrationDepth=capsule.Radius() - sqDist1.Sqrt();
			
		}
		else if(sqDist2.LT(sqDist3))
		{
			normal=C_Normal(d2*u);
			contactPoint=c+ c20*u;
			penetrationDepth=capsule.Radius() - sqDist2.Sqrt();
			
		}
		else
		{
			normal=C_Normal(d3*u);
			contactPoint=c + c30* u;
			penetrationDepth=capsule.Radius()-sqDist3.Sqrt();
		}

	}

	C_ContactPatch<MaxContacts> & patch=results.m_ContactPatches[0];
	patch.m_NumContactPoints = 1;
	patch.m_Normal=normal;
	C_ContactPoint& point=patch.m_ContactPoints[0];
	point.m_ContactPoint=contactPoint;
	Maths::Store(penetrationDepth, &point.m_PenetrationDepth);
	//point.m_PenetrationDepth=penetrationDepth;

	return true;
}


template <int MaxPatches, int MaxContacts>
bool CollideOBBCapsule(const C_OBB& obb, const C_Capsule& capsule, C_CollisionResult<MaxPatches, MaxContacts>& results)
{

	using namespace Maths;
	//const Maths::HVec3 vecZero(Maths::Constants<HVec3>::Zero());
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one (Constants<HF32>::One());
	const HF32 half (Constants<HF32>::Half());
	const HVec3 dA(capsule.Start() - obb.Centre());
	const HVec3 dB(capsule.End() - obb.Centre());

	/*
	In the math libary, Mat*HVec in mathatical term need to be implemented as HVec*Mat
	Also HVec*InverseMat == Mat*HVec in this libary
	*/
	//the corresponding local space points for the capsule.start and capsule.end
	const HVec3 startL(obb.Orientation() * dA);//inverseMatrix * HVec3
	const HVec3 endL(obb.Orientation() * dB);


	const HVec3 e(obb.Extent());
	const HVec3 ne(-obb.Extent());

	//Get startL and endL's projection on to the box surface
	const HVec3 projAL(Max(ne, Min(startL, e)));
	const HVec3 projBL(Max(ne, Min(endL, e)));

	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(obb.c + projAL*obb.u, 0.5f, RGBA(255, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(obb.c + projBL*obb.u, 0.5f, RGBA(255, 255, 0, 255));*/


	
	const HVec3 a(startL - projAL);
	const HVec3 b(endL - projBL);
	const HF32 sDistS(a|a);
	const HF32 sDistE(b|b);

	//Deal with the case when startL and endL both project on the same surface

	if(projAL.Any_EQ(projBL) & Abs(projAL).Any_EQ(e))
	{
		const HF32 _min(Min(sDistS, sDistE));
		const HF32 pDepth1(_min.Sqrt());
		const Scalar_Bool conA(sDistS.LT(sDistE));
		const HVec3 contact1=Select(conA, projAL, projBL);
		const HVec3 n1=Select(conA, a, b);
		C_ContactPatch<MaxContacts> & patch=results.m_ContactPatches[0];
		patch.m_NumContactPoints = 1;
		patch.m_Normal=C_Normal(n1*obb.Orientation());
		C_ContactPoint& point=patch.m_ContactPoints[0];
		point.m_ContactPoint=obb.Centre() + contact1*obb.Orientation();
		const HF32 pDepth(capsule.Radius() - pDepth1);
		Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth=capsule.m_Radius - pDepth1;
		return (pDepth1 <= capsule.Radius()) == 1;
	}


	//Deal with the case when startL and endL both are internal point or one of them are inside point
	const Scalar_Bool insidePoint1(sDistS.EQ(zero));
	const Scalar_Bool insidePoint2(sDistE.EQ(zero));
 
	//const Maths::Scalar_Bool bb(insidePoint1 | insidePoint2);

	if(insidePoint1 | insidePoint2)
	{

		const HVec3 offSetX(one, zero, zero);
		const HVec3 offSetY(zero, one, zero);
		const HVec3 offSetZ(zero, zero, one);

		const HVec3 xS(projAL.X().Sign()*e.X(), projAL.Y(), projAL.Z());
		const HVec3 yS(projAL.X(), projAL.Y().Sign()*e.Y(), projAL.Z());
		const HVec3 zS(projAL.X(), projAL.Y(), projAL.Z().Sign()*e.Z());

		const HVec3 aS(e - Abs(projAL));

		const HVec3 dS(Abs(aS));

		const HF32 dXS(dS.X());
		const HF32 dYS(dS.Y());
		const HF32 dZS(dS.Z());


		const Scalar_Bool conS0(dXS.LT(dYS) & dXS.LT(dZS));
		const Scalar_Bool conS1(dYS.LT(dZS));

		const HF32 pDepth00=Select(conS0, dXS,  Select(conS1, dYS, dZS));
		const HVec3 n00=Select(conS0, offSetX, Select(conS1, offSetY, offSetZ));
		const HVec3 contact00= Select(conS0, xS, Select(conS1, yS, zS));


		const HVec3 xE(projBL.X().Sign()*e.X(), projBL.Y(), projBL.Z());
		const HVec3 yE(projBL.X(), projBL.Y().Sign()*e.Y(), projBL.Z());
		const HVec3 zE(projBL.X(), projBL.Y(), projBL.Z().Sign()*e.Z());

		const HVec3 aE(e - Abs(projBL));

		const HVec3 dE(Abs(aE));
		const HF32 dXE(dE.X());
		const HF32 dYE(dE.Y());
		const HF32 dZE(dE.Z());

		const Scalar_Bool conE0(dXE.LT(dYE) & dXE.LT(dZE));
		const Scalar_Bool conE1(dYE.LT(dZE));

		const HF32 pDepth01=Select(conE0, dXE,  Select(conE1, dYE, dZE));
		const HVec3 n01=Select(conE0, offSetX, Select(conE1, offSetY, offSetZ));
		const HVec3 contact01=Select(conE0, xE, Select(conE1, yE, zE));


		const Scalar_Bool con0(pDepth00.GT(pDepth01));
		const HF32 pDepth0=Select(con0, pDepth00, pDepth01);
		const HVec3 n0=Select(con0, n00, n01);
		const HVec3 contact0=Select(con0,contact00, contact01);

		const Vector3_Bool bb1(insidePoint1&insidePoint2);
		const Vector3_Bool bb2(insidePoint1 & (!insidePoint2));
		const C_Normal norm_tmp(Select(bb1, n0, Select(bb2, n00, n01)), Maths::C_Normal::UNIT_LENGTH_VECTOR);


		const HVec3 cpoint(Select(bb1, contact0, Select(bb2, contact00, contact01)));

		const HF32 dd(cpoint | norm_tmp);
		const C_Normal norm(Select(dd.LE(zero), -norm_tmp, norm_tmp));

		const HF32 cdepth(Select(Maths::X(bb1), pDepth0, Select(Maths::X(bb2), pDepth00, pDepth01)));

		C_ContactPatch<MaxContacts> & patch=results.m_ContactPatches[0];
		patch.m_NumContactPoints = 1;
		patch.m_Normal=C_Normal(norm*obb.Orientation());
		C_ContactPoint& point=patch.m_ContactPoints[0];
		point.m_ContactPoint=obb.Centre() + cpoint*obb.Orientation();
		const HF32 pDepth(capsule.Radius() + cdepth);
		Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth=capsule.m_Radius + cdepth;

		return true;
	}


	
	//Deal with the case when startL and endL project on different surface
	const HVec3 mid((projAL+projBL)*half);

	HVec3 sign(mid.X().Sign(), mid.Y().Sign(), mid.Z().Sign());

	//pick one of the corner point in the local space
	const HVec3 pickSL(e*sign);
	
	HVec3 pick1(pickSL);
	HVec3 pick2(pickSL);
	HVec3 pick3(pickSL);

	pick1.X(-pickSL.X());
	pick2.Y(-pickSL.Y());
	pick3.Z(-pickSL.Z());

	HVec3 c1(UNDEFINED);
	HVec3 c2(UNDEFINED);
	HVec3 c3(UNDEFINED);
	HVec3 c4(UNDEFINED);
	HVec3 c5(UNDEFINED);
	HVec3 c6(UNDEFINED);

	Collision::ClosestPtSegmentSegment(pickSL, pick1, startL, endL, c1, c2);
	Collision::ClosestPtSegmentSegment(pickSL, pick2, startL, endL, c3, c4);
	Collision::ClosestPtSegmentSegment(pickSL, pick3, startL, endL, c5, c6);

	const HVec3 d1(c2 - c1);
	const HF32 sqDist1(d1|d1);

	const HVec3 d2(c4 - c3);
	const HF32 sqDist2(d2|d2);

	const HVec3 d3(c6 - c5);
	const HF32 sqDist3(d3|d3);

	const Scalar_Bool con1(sqDist1.LT(sqDist2) & sqDist1.LT(sqDist3));
	const Scalar_Bool con2(sqDist2.LT(sqDist3));
	const HVec3 n2=Select(con1, d1, Select(con2, d2, d3));
	const HVec3 contact2=Select(con1, c1, Select(con2, c3, c5));
	const HF32 pDepth2=Select(con1, sqDist1.Sqrt(), Select(con2, sqDist2.Sqrt(), sqDist3.Sqrt()));

	const Maths::HVec3 pickEL=Select(con1, pick1, Select(con2, pick2, pick3));
	c1=Select(con1, c1, Select(con2, c3, c5));
	c2=Select(con1, c2, Select(con2, c4, c6));

	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(obb.c + pickSL*obb.u, 0.5f, RGBA(0, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(obb.c + pickEL*obb.u, 0.5f, RGBA(0, 255, 255, 255));*/

	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(obb.c + c1*obb.u, 0.05f, RGBA(0, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(obb.c + c2*obb.u, 0.05f, RGBA(0, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(obb.c + c1*obb.u, obb.c + c2*obb.u, RGBA(0, 0, 0, 255));*/
		

	/*const Maths::HVec3 normal=Select(surface, n1, n2);
	const Maths::HVec3 contact=Select(surface, contact1, contact2);
	const Maths::HF32 penetrationDepth=Select(surface, pDepth1, pDepth2);*/
	
	C_ContactPatch<MaxContacts> & patch=results.m_ContactPatches[0];
	patch.m_NumContactPoints = 1;
	patch.m_Normal=C_Normal(n2*obb.Orientation());
	C_ContactPoint& point=patch.m_ContactPoints[0];
	point.m_ContactPoint=obb.Centre() + contact2*obb.Orientation();
	const HF32 pDepth(capsule.Radius() - pDepth2);
	Store(pDepth, &point.m_PenetrationDepth);
	//point.m_PenetrationDepth=capsule.m_Radius - pDepth2;

	/*const Maths::HVec3 end=point.m_ContactPoint - patch.m_Normal * point.m_PenetrationDepth ;

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.5f, RGBA(255,255, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point.m_ContactPoint, end, RGBA(255, 0, 0, 255));*/

	return true;
	
}



};

#endif

