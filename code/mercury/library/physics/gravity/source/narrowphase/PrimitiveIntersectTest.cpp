#include <stdafx.h>
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/CapsuleRay.inl"

//using namespace Maths;

namespace Collision
{


U32 CollideCapsuleTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF normal, Collision::C_Capsule& capsule,
							  Maths::C_Normal& resultNormal, Maths::HVec3* contacts,
							  Maths::HF32* pens)
{
	using namespace Maths;

	const HF32 r(capsule.Radius());


	const HF32 dA(a | normal);
	const HF32 dS((capsule.Start() | normal) - dA);
	const HF32 dE((capsule.End() | normal) - dA);
	const HF32 absDS(Maths::Abs(dS));
	const HF32 absDE(Maths::Abs(dE));

	const HF32 dist(dS*dE);
	const HF32 zero(Constants<HF32>::Zero());

	if(dist.GT(zero) & absDS.GE(r) & absDE.GE(r))
		return 0;

	const HF32 three = Maths::Constants<HF32>::Three();
	const HVec3 v(capsule.End() - capsule.Start());
	const HF32 sqV(v|v);

	
	HVec3 contact(Maths::UNDEFINED);
	U32 result= 0;
	resultNormal = -normal;
	const HVec3 projS = capsule.Start() - normal*dS;
	const HVec3 projE = capsule.End() - normal*dE;

	/*const C_Normal norm(projE - projS);
	const HVec3 contactS =  projS- norm*r*three;
	const HVec3 contactE =  projE + norm*r*three;

	HVec3 res2(Maths::UNDEFINED);
	if(CollideCapsuleRay(&capsule, contactS, contactE, res2))
	{
		contacts[0] = res2;
		pens[0] = zero;
		
		result=1;
	}*/


	if(Collision::IntersectTriangleSegmentSB(a, b, c, capsule.Start(), capsule.End(), contact))
	{
		const HVec3 rayEnd = contact - normal*sqV;
		HVec3 res1(Maths::UNDEFINED);
		if(CollideCapsuleRay(&capsule, rayEnd, contact, res1))
		{
			const Maths::HVec3 difVec(res1 - contact);
			/*resultNormal = Maths::C_Normal(difVec);
			if((resultNormal | normal).GT(zero))
			{
				return 0;
			}*/
			contacts[result] = res1;			
			pens[result++] = difVec | resultNormal;
			
		}
	}
	else
	{
		
		HVec3 closestA(Maths::UNDEFINED);
		HVec3 closestB(Maths::UNDEFINED);
		HF32 sqDist(Maths::UNDEFINED);
		ClosestPtSegmentTriangle(capsule.Start(), capsule.End(), a, b, c, closestA, closestB, sqDist);

		if(sqDist.GT(r*r))
			return 0;

		const HVec3 rayStart = closestB - normal*sqV*three;
		HVec3 res1(Maths::UNDEFINED);

	/*	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(rayStart, 0.25f, Maths::RGBA(255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(rayStart, rayEnd, Maths::RGBA(0, 255, 0));*/

		/*if(Collision::CollideCapsuleRay(&capsule, rayStart, closestA, res1))
		{
			const Maths::HVec3 difVec(res1 - closestB);
			resultNormal = Maths::C_Normal(difVec);
			if((resultNormal | normal).GT(zero))
			{
				return 0;
			}
			contacts[result] = res1;
			pens[result++] = (difVec | resultNormal);
		}*/

		//if(Collision::CollideCapsuleRay(&capsule, rayStart, closestA, res1))
		{
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(res1, 0.1f, Maths::RGBA(0, 0, 255));
			//const Maths::HVec3 difVec(res1 - closestB);
			const Maths::HVec3 difVec(closestB - closestA);
			//resultNormal = Maths::C_Normal(closestB - closestA);//difVec);
			resultNormal = Maths::C_Normal(difVec);
			if((resultNormal | normal).GT(zero))
			{
				return 0;
			}
			contacts[result] = closestB;//res1;
			//pens[result++] = (difVec | resultNormal);
			pens[result++] = capsule.Radius() - (difVec | resultNormal);
		}
	}

	return result;
}


//void GetContactAndPenetration(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::C_Normal PCREF norm, const Maths::HF32 PCREF projDist, Maths::HVec3& contact, Maths::HF32& pen)
//{
//	using namespace Maths;
//	const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
//	HVec3 c01(Maths::UNDEFINED), c02(Maths::UNDEFINED), c03(Maths::UNDEFINED), c04(Maths::UNDEFINED), c05(Maths::UNDEFINED), c06(Maths::UNDEFINED);
//	Collision::ClosestPtSegmentSegment(p, q, a, b, c01, c02);
//	Collision::ClosestPtSegmentSegment(p, q, a, c, c03, c04);
//	Collision::ClosestPtSegmentSegment(p, q, b, c, c05, c06);
//
//	const HVec3 v00(c01 - c02);
//	const HVec3 v01(c03 - c04);
//	const HVec3 v02(c05 - c06);
//
//	const HF32 sqDist0(v00|v00);
//	const HF32 sqDist1(v01|v01);
//	const HF32 sqDist2(v02|v02);
//
//	//const HF32 epsilon(0.000001f);
//	Scalar_Bool con1(sqDist0.LT(epsilon));
//	Scalar_Bool con2(sqDist1.LT(epsilon));
//	Scalar_Bool con3(sqDist2.LT(epsilon));
//
//	const HVec3 intersectPoint = Select(con1, c01, Select(con2, c03, c05));
//
//	const HVec3 v1(q - p);
//	const HVec3 v2(q - intersectPoint);
//	const HF32 s1(v1.Length());
//	const HF32 s2(v2.Length());
//	pen = s2 * projDist * s1.Reciprocal();
//	contact = intersectPoint - norm*pen;
//
//}


void GetContactAndPenetration(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::C_Normal PCREF norm, const Maths::HF32 PCREF projDist, Maths::HVec3& contact, Maths::HF32& pen)
{
	using namespace Maths;
	const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
	HVec3 c01(Maths::UNDEFINED), c02(Maths::UNDEFINED), c03(Maths::UNDEFINED), c04(Maths::UNDEFINED), c05(Maths::UNDEFINED), c06(Maths::UNDEFINED);
	Collision::ClosestPtSegmentSegment(p, q, a, b, c01, c02);
	Collision::ClosestPtSegmentSegment(p, q, a, c, c03, c04);
	Collision::ClosestPtSegmentSegment(p, q, b, c, c05, c06);

	const HVec3 v00(c01 - c02);
	const HVec3 v01(c03 - c04);
	const HVec3 v02(c05 - c06);

	const HF32 sqDist0(v00|v00);
	const HF32 sqDist1(v01|v01);
	const HF32 sqDist2(v02|v02);

	//const HF32 epsilon(0.000001f);
	Scalar_Bool con1(sqDist0.LT(epsilon));
	Scalar_Bool con2(sqDist1.LT(epsilon));
	Scalar_Bool con3(sqDist2.LT(epsilon));

	const HVec3 intersectPoint = Select(con1, c01, Select(con2, c03, c05));

	const HVec3 v1(q - p);
	const HVec3 v2(q - intersectPoint);
	const HF32 s1(v1.Length());
	const HF32 s2(v2.Length());
	pen = s2 * projDist * s1.Reciprocal();
	contact = intersectPoint - norm*pen;

}

bool TomasCollideTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22, 
								  const Maths::C_Normal PCREF _normA, const Maths::C_Normal PCREF _normB, Maths::HVec3& contactA, Maths::HVec3& contactB, Maths::HF32& penA, Maths::HF32& penB)
{
	using namespace Maths;

	

	const HF32 zero(Maths::Constants<HF32>::Zero());

	const HF32 one(Maths::Constants<HF32>::One());
	const HF32 epsilon(0.001f);
	const C_Normal tNormA((t12 - t10)^(t11 - t10));
	const C_Normal tNormB((t22 - t20)^(t21 - t20));

	const C_Normal normA(Maths::Select((tNormA | _normA).GT(zero), tNormA, -tNormA));
	const C_Normal normB(Maths::Select((tNormB | _normB).GT(zero), tNormB, -tNormB));
	const HF32 d1(-(normA|t10)); //the constant-normal form of the t1 plane
	const HF32 d2(-(normB|t20));//the constant-normal form of the t2 plane


	//the signed distances of each individual vertex of t1 to the t2 plane
	const HF32 dt10((normB|t10) + d2);
	const HF32 dt11((normB|t11) + d2);
	const HF32 dt12((normB|t12) + d2);

	const HF32 mt101(dt10*dt11);
	const HF32 mt102(dt10*dt12);

	const HF32 dt20((normA|t20) + d1);
	const HF32 dt21((normA|t21) + d1);
	const HF32 dt22((normA|t22) + d1);

	const HF32 mt201(dt20*dt21);
	const HF32 mt202(dt20*dt22);

	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
	/*if(mt101.GT(zero) && mt102.GT(zero)) 
		return false;*/

	if((Abs(dt10).LE(epsilon) & Abs(dt11).LE(epsilon) & Abs(dt12).LE(epsilon)) | (mt101.GT(zero) & mt102.GT(zero))
		| (mt201.GT(zero) & mt202.GT(zero)))
	{
		//co-plane
		return false;
	} 

	
	/*
		the code below insure t10 is always on the other side of the t2 plane
	*/

	
	//const Scalar_Bool a0(mt101.LT(zero) & mt102.LT(zero)); //no change
	//const Scalar_Bool a1(mt101.GT(zero) & mt102.LT(zero)); //Swap 0 and 2, else Swap 0 and 1

	/*const HVec3 nt10(Select(a0, t10, Select(a1, t12, t11)));
	const HVec3 nt11(Select(a0, t11, Select(a1, t11, t10)));
	const HVec3 nt12(Select(a0, t12, Select(a1, t10, t12)));
	const HF32  ndt10(Select(a0, dt10, Select(a1, dt12, dt11)));
	const HF32  ndt11(Select(a0, dt11, Select(a1, dt11, dt10)));
	const HF32  ndt12(Select(a0, dt12, Select(a1, dt10, dt12)));*/

	const HF32 mt103(dt11*dt12);
	const Scalar_Bool a0(dt10.EQ(zero) & mt103.GT(zero));//one point on the plane but the other two points on the same side of the plane
	const Scalar_Bool a1(dt11.EQ(zero) & mt102.GT(zero));
	const Scalar_Bool a2(dt12.EQ(zero) & mt101.GT(zero));
	const Scalar_Bool a3(mt101.LT(zero)& mt103.LT(zero));
	const Scalar_Bool a4(mt102.LT(zero)& mt103.LT(zero));

	const HVec3 tnt10(Select(a0, t10, Select(a1, t11, Select(a2, t12, t10))));
	const HVec3 tnt11(Select(a0, t11, Select(a1, t12, Select(a2, t10, t11))));
	const HVec3 tnt12(Select(a0, t12, Select(a1, t10, Select(a2, t11, t12))));

	const HVec3 nt10(Select(a3, t11, Select(a4, t12, tnt10)));
	const HVec3 nt11(Select(a3, t12, Select(a4, t10, tnt11)));
	const HVec3 nt12(Select(a3, t10, Select(a4, t11, tnt12)));

	const HF32 ndt10((normB|nt10) + d2);
	const HF32 ndt11((normB|nt11) + d2);
	const HF32 ndt12((normB|nt12) + d2);


	/*
		the code below insure t20 is always on the other side of the t1 plane
	*/
	//const Scalar_Bool b0(mt201.LE(zero) & mt202.LE(zero)); //no change
	//const Scalar_Bool b1(mt201.GT(zero) & mt202.LT(zero)); //Swap 0 and 2, else swap 0 and 1

	//const HVec3 nt20(Select(b0, t20, Select(b1, t22, t21)));
	//const HVec3 nt21(Select(b0, t21, Select(b1, t21, t20)));
	//const HVec3 nt22(Select(b0, t22, Select(b1, t20, t22)));
	//const HF32  ndt20(Select(b0, dt20, Select(b1, dt22, dt21)));
	//const HF32  ndt21(Select(b0, dt21, Select(b1, dt21, dt20)));
	//const HF32  ndt22(Select(b0, dt22, Select(b1, dt20, dt22)));



	const HF32 mt203(dt21*dt22);
	const Scalar_Bool b0(dt20.EQ(zero) & mt203.GT(zero));//one point on the plane but the other two points on the same side of the plane
	const Scalar_Bool b1(dt21.EQ(zero) & mt202.GT(zero));
	const Scalar_Bool b2(dt22.EQ(zero) & mt201.GT(zero));
	const Scalar_Bool b3(mt201.LT(zero)& mt203.LT(zero));
	const Scalar_Bool b4(mt202.LT(zero)& mt203.LT(zero));


	/*const HVec3 nt10(Select(a0, t10, Select(a1, t12, t11)));
	const HVec3 nt11(Select(a0, t11, Select(a1, t11, t10)));
	const HVec3 nt12(Select(a0, t12, Select(a1, t10, t12)));
	const HF32  ndt10(Select(a0, dt10, Select(a1, dt12, dt11)));
	const HF32  ndt11(Select(a0, dt11, Select(a1, dt11, dt10)));
	const HF32  ndt12(Select(a0, dt12, Select(a1, dt10, dt12)));*/

	const HVec3 tnt20(Select(b0, t20, Select(b1, t21, Select(b2, t22, t20))));
	const HVec3 tnt21(Select(b0, t21, Select(b1, t22, Select(b2, t20, t21))));
	const HVec3 tnt22(Select(b0, t22, Select(b1, t20, Select(b2, t21, t22))));

	const HVec3 nt20(Select(b3, t21, Select(b4, t22, tnt20)));
	const HVec3 nt21(Select(b3, t22, Select(b4, t20, tnt21)));
	const HVec3 nt22(Select(b3, t20, Select(b4, t21, tnt22)));

	const HF32 ndt20((normA|nt20) + d1);
	const HF32 ndt21((normA|nt21) + d1);
	const HF32 ndt22((normA|nt22) + d1);
	

	/*
		after the above test, the t1 plane and the t2 plane will intersect in a line, whose direction is the
		cross product of these two plane's normal
	*/
	//const HVec3 dir(n1^n2);
	const C_Normal dir(normA^normB);


	//calculate the intersect intervals of t1 and the line
	const HF32 pt10(dir|nt10);
	const HF32 pt11(dir|nt11);
	const HF32 pt12(dir|nt12);

	const HF32 _t1_min=pt10+(pt11-pt10)*(ndt10*((ndt10-ndt11).Reciprocal()));
	const HF32 _t1_max=pt10+(pt12-pt10)*(ndt10*((ndt10-ndt12).Reciprocal()));


	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
	const Scalar_Bool con11(_t1_min.GT(_t1_max));
	const HF32 t1_min(Select(con11, _t1_max, _t1_min));
	const HF32 t1_max(Select(con11, _t1_min, _t1_max));


	//calculate the intersect intervals of t2 and the line
	const HF32 pt20(dir|nt20);
	const HF32 pt21(dir|nt21);
	const HF32 pt22(dir|nt22);


	const HF32 _t2_min=pt20+(pt21-pt20)*(ndt20*((ndt20-ndt21).Reciprocal()));
	const HF32 _t2_max=pt20+(pt22-pt20)*(ndt20*((ndt20-ndt22).Reciprocal()));


	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
	const Scalar_Bool con21(_t2_min.GT(_t2_max));
	const HF32 t2_min(Select(con21, _t2_max, _t2_min));
	const HF32 t2_max(Select(con21, _t2_min, _t2_max));
	
	if(t1_min.EQ(t1_max))
	{
		//Triangle intersect in one point
		contactA = contactB = nt10;
		penA = penB = zero;
		return true;
		//return false;

	}

	if(t2_min.EQ(t2_max))
	{
		contactA = contactB = nt20;
		penA = penB = zero;
		return true;
		//return false;
	}

	//Triangle intersect in one line
	if(t2_max.GE(t1_min) & t2_min.LE(t1_max))
	{
		const HVec3 nnt11(Select(con11, nt12, nt11));
		const HVec3 nnt12(Select(con11, nt11, nt12));
		const HF32 ddt11(Select(con11, ndt12, ndt11));
		const HF32 ddt12(Select(con11, ndt11, ndt12));

		const HVec3 nnt21(Select(con21, nt22, nt21));
		const HVec3 nnt22(Select(con21, nt21, nt22));
		const HF32 ddt21(Select(con21, ndt22, ndt21));
		const HF32 ddt22(Select(con21, ndt21, ndt22));

		const HF32 adt12(ddt12.Abs());
		const HF32 adt11(ddt11.Abs());
		const HF32 adt10(ndt10.Abs());

		const HF32 adt22(ddt22.Abs());
		const HF32 adt21(ddt21.Abs());
		const HF32 adt20(ndt20.Abs());

		const HVec3 v110(nnt11 - nt10);
		const HF32 p110=adt10*((adt10+adt11).Reciprocal());
		const HVec3 contact110(nt10 + p110 * v110);

		const HVec3 v120(nnt12 - nt10);
		const HF32 p120=adt10*((adt10+adt12).Reciprocal());
		const HVec3 contact120(nt10 + p120 * v120);

		const HVec3 v210(nnt21 - nt20);
		const HF32 p210=adt20 *((adt20+adt21).Reciprocal());
		const HVec3 contact210(nt20 + p210 * v210);

		const HVec3 v220(nnt22 - nt20);
		const HF32 p220=adt20*((adt20+adt22).Reciprocal());
		const HVec3 contact220(nt20 + p220 * v220);

		const Scalar_Bool con1(t1_min.LE(t2_min));
		const Scalar_Bool con2(t1_max.LE(t2_max));

		//Get the surface contact
		contactA = Select(con1, contact210, contact110);
		contactB = Select(con2, contact120, contact220);
	
		penA=zero;
		penB=zero;


		/*BIZ_ASSERT(Abs((contactA | normA) + d1).LE(epsilon) && Abs((contactA | normB) + d2).LE(epsilon));
		BIZ_ASSERT(Abs((contactB | normA) + d1).LE(epsilon) && Abs((contactB | normB) + d2).LE(epsilon));*/

		if(ndt10.LT(zero))
		{
			const HVec3 projP(nt10 + normB*adt10);
			
			if(PointInsideTriangle(projP, t20, t21, t22))
			{
				contactA = nt10;
				penA = adt10;
			}
			else
			{
				if(!con1)//contactA will be the contact110
				{
					GetContactAndPenetration(contactA, projP, t20, t21, t22, normB, adt10, contactA, penA);
				}

				if(con2)//contactB will be the contact120
				{
					GetContactAndPenetration(contactB, projP, t20, t21, t22, normB, adt10, contactB, penB);
				}

			}
			return true;
		}
		else
		{
		
			if(!con1)//contactA is contact110
			{
				const HVec3 projP11(nnt11 + normB*adt11);

				if(PointInsideTriangle(projP11, t20, t21, t22))
				{
					contactA = nnt11;
					penA = adt11;
				}
				else
				{
					GetContactAndPenetration(contactA, projP11, t20, t21, t22, normB, adt11, contactA, penA);
				}
			}
			

			if(con2)//contactB is contact120
			{
				const HVec3 projP12(nnt12 + normB*adt12);

				if(PointInsideTriangle(projP12, t20, t21, t22))
				{
					contactB = nnt12;
					penB = adt12;
				}
				else
				{
					GetContactAndPenetration(contactB, projP12, t20, t21, t22, normB, adt12, contactB, penB);
				}
			}
		}

		return true;
	}
	return false;

}


//bool TomasCollideTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22, 
//								   const Maths::C_Normal PCREF normA, const Maths::C_Normal PCREF normB, Maths::C_Normal& resultNormal,
//								   Maths::HVec3& contactA, Maths::HVec3& contactB, Maths::HF32& penA, Maths::HF32& penB)
//{
//	using namespace Maths;
//
//	const HF32 zero(Maths::Constants<HF32>::Zero());
//	const HF32 one(Maths::Constants<HF32>::One());
//	const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
//	
//	const HF32 d1(-(normA|t10)); //the constant-normal form of the t1 plane
//	const HF32 d2(-(normB|t20));//the constant-normal form of the t2 plane
//
//	//the signed distances of each individual vertex of t1 to the t2 plane
//	const HF32 dt10((normB|t10) + d2);
//	const HF32 dt11((normB|t11) + d2);
//	const HF32 dt12((normB|t12) + d2);
//
//	const HF32 mt101(dt10*dt11);
//	const HF32 mt102(dt10*dt12);
//	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
//	if(mt101.GT(zero) && mt102.GT(zero)) 
//		return false;
//
//	const HF32 dt20((normA|t20) + d1);
//	const HF32 dt21((normA|t21) + d1);
//	const HF32 dt22((normA|t22) + d1);
//
//	const HF32 mt201(dt20*dt21);
//	const HF32 mt202(dt20*dt22);
//	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
//	if(mt201.GT(zero) && mt202.GT(zero))
//		return false;
//
//	
//	/*
//		the code below insure t10 is always on the other side of the t2 plane
//	*/
//	const Maths::Scalar_Bool a0(mt101.LT(zero) & mt102.LT(zero)); //no change
//	const Maths::Scalar_Bool a1(mt101.GT(zero) & mt102.LT(zero)); //Swap 0 and 2, else Swap 0 and 1
//
//	const Maths::HVec3 nt10(Select(a0, t10, Select(a1, t12, t11)));
//	const Maths::HVec3 nt11(Select(a0, t11, Select(a1, t11, t10)));
//	const Maths::HVec3 nt12(Select(a0, t12, Select(a1, t10, t12)));
//	const Maths::HF32  ndt10(Select(a0, dt10, Select(a1, dt12, dt11)));
//	const Maths::HF32  ndt11(Select(a0, dt11, Select(a1, dt11, dt10)));
//	const Maths::HF32  ndt12(Select(a0, dt12, Select(a1, dt10, dt12)));
//
//
//	/*
//		the code below insure t20 is always on the other side of the t1 plane
//	*/
//	const Maths::Scalar_Bool b0(mt201.LT(zero) & mt202.LT(zero)); //no change
//	const Maths::Scalar_Bool b1(mt201.GT(zero) & mt202.LT(zero)); //Swap 0 and 2, else swap 0 and 1
//
//	const Maths::HVec3 nt20(Select(b0, t20, Select(b1, t22, t21)));
//	const Maths::HVec3 nt21(Select(b0, t21, Select(b1, t21, t20)));
//	const Maths::HVec3 nt22(Select(b0, t22, Select(b1, t20, t22)));
//	const Maths::HF32  ndt20(Select(b0, dt20, Select(b1, dt22, dt21)));
//	const Maths::HF32  ndt21(Select(b0, dt21, Select(b1, dt21, dt20)));
//	const Maths::HF32  ndt22(Select(b0, dt22, Select(b1, dt20, dt22)));
//
//	/*
//		after the above test, the t1 plane and the t2 plane will intersect in a line, whose direction is the
//		cross product of these two plane's normal
//	*/
//	//const HVec3 dir(n1^n2);
//	const HVec3 dir(normA^normB);
//
//	//calculate the intersect intervals of t1 and the line
//	const HF32 pt10(dir|nt10);
//	const HF32 pt11(dir|nt11);
//	const HF32 pt12(dir|nt12);
//
//	const HF32 _t1_min=pt10+(pt11-pt10)*(ndt10/(ndt10-ndt11));
//	const HF32 _t1_max=pt10+(pt12-pt10)*(ndt10/(ndt10-ndt12));
//
//	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
//	const Maths::Scalar_Bool con1(_t1_min.GT(_t1_max));
//	const Maths::HF32 t1_min(Select(con1, _t1_max, _t1_min));
//	const Maths::HF32 t1_max(Select(con1, _t1_min, _t1_max));
//	const Maths::HVec3 nnt11(Select(con1, nt12, nt11));
//	const Maths::HVec3 nnt12(Select(con1, nt11, nt12));
//	const Maths::HF32 ddt11(Select(con1, ndt12, ndt11));
//	const Maths::HF32 ddt12(Select(con1, ndt11, ndt12));
//
//
//	//calculate the intersect intervals of t2 and the line
//	const HF32 pt20(dir|nt20);
//	const HF32 pt21(dir|nt21);
//	const HF32 pt22(dir|nt22);
//
//	const HF32 _t2_min=pt20+(pt21-pt20)*(ndt20/(ndt20-ndt21));
//	const HF32 _t2_max=pt20+(pt22-pt20)*(ndt20/(ndt20-ndt22));
//
//	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
//	const Maths::Scalar_Bool con2(_t2_min.GT(_t2_max));
//	const Maths::HF32 t2_min(Select(con2, _t2_max, _t2_min));
//	const Maths::HF32 t2_max(Select(con2, _t2_min, _t2_max));
//	const Maths::HVec3 nnt21(Select(con2, nt22, nt21));
//	const Maths::HVec3 nnt22(Select(con2, nt21, nt22));
//	const Maths::HF32 ddt21(Select(con2, ndt22, ndt21));
//	const Maths::HF32 ddt22(Select(con2, ndt21, ndt22));
//
//
//	//Triangle intersect in one line
//	if(t2_max.GE(t1_min) && t2_min.LE(t1_max))
//	{
//		const Maths::HF32 adt12(ddt12.Abs());
//		const Maths::HF32 adt11(ddt11.Abs());
//		const Maths::HF32 adt10(ndt10.Abs());
//		//const HF32 pen2(Min(adt10, Min(adt11, adt12)));
//
//
//		const Maths::HF32 adt22(ddt22.Abs());
//		const Maths::HF32 adt21(ddt21.Abs());
//		const Maths::HF32 adt20(ndt20.Abs());
//		//const HF32 pen(Min(adt20, Min(adt21, adt22)));
//		penA=(adt10);
//		penB=(adt20);
//		
//		//resultNormal = Maths::C_Normal(Maths::Select(pen.LE(pen2), normA, normB), Maths::C_Normal::UNIT_LENGTH_VECTOR);
//		resultNormal = Maths::C_Normal(normA, Maths::C_Normal::UNIT_LENGTH_VECTOR);
//
//		const Maths::HVec3 v210(nnt21 - nt20);
//		const Maths::HF32 p210=adt20/(adt20+adt21);
//		const Maths::HVec3 contact210(nt20 + p210 * v210);
//
//		const Maths::HVec3 v110(nnt11 - nt10);
//		const Maths::HF32 p110=adt10/(adt10+adt11);
//		const Maths::HVec3 contact110(nt10 + p110 * v110);
//
//		const Maths::HVec3 v120(nnt12 - nt10);
//		const Maths::HF32 p120=adt10/(adt10+adt12);
//		const Maths::HVec3 contact120(nt10 + p120 * v120);
//
//		const Maths::HVec3 v220(nnt22 - nt20);
//		const Maths::HF32 p220=adt20/(adt20+adt22);
//		const Maths::HVec3 contact220(nt20 + p220 * v220);
//
//		const Maths::Scalar_Bool con1(t1_min.LE(t2_min));
//		const Maths::Scalar_Bool con2(t1_max.LE(t2_max));
//
//		//penetration = Maths::Min(pen, pen2);
//		contactA = Select(con1, contact210, contact110);
//		contactB = Select(con2, contact120, contact220);
//		return true;
//	}
//	return false;
//}

void C_Plane::ProjectQuat(C_CRAxisBox& _axisBox, Maths::HVec3& aVex, Maths::HVec3& bVex, Maths::HVec3& cVex, Maths::HVec3& dVex)
{
	using namespace Maths;

	C_CRAxisBox axisBox(_axisBox);
	axisBox.centre -= m_n * m_d;	//Translate back into local space...

	const Maths::C_Normal upNorm(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), Maths::C_Normal::UNIT_LENGTH_VECTOR);
	const Maths::HVec3 nor(m_n);
	Maths::HVec3 tU(Maths::UNDEFINED), tV(Maths::UNDEFINED);
	nor.PerpendicularAxes(tU, tV);

	const Maths::HMat33 rot(tU, nor, tV);
	const Maths::HMat43 trans(rot, Maths::Constants<Maths::HVec3>::Zero());

	C_CRAxisBox transBox;
	axisBox.Transform(trans.InverseRotTrans(), transBox);

	const HVec3 center = transBox.centre;
	const HF32 cx = center.X();
	const HF32 cy = center.Y();
	const HF32 cz = center.Z();
	const HVec3 extents = transBox.extents;
	const HF32 x = extents.X();
	const HF32 y = extents.Y();
	const HF32 z = extents.Z();

	const HVec3 p1 (cx+x, cy+y, cz+z);
	const HVec3 p2 (cx+x, cy+y, cz-z);
	const HVec3 p3 (cx-x, cy+y, cz-z);
	const HVec3 p4 (cx-x, cy+y, cz+z);
	const HVec3 p5 (cx+x, cy-y, cz+z);
	const HVec3 p6 (cx+x, cy-y, cz-z);
	const HVec3 p7 (cx-x, cy-y, cz-z);
	const HVec3 p8 (cx-x, cy-y, cz+z);

	/*const HVec3 proj1 = p1 - Distance(p1)*m_n;
	const HVec3 proj2 = p2 - Distance(p2)*m_n;
	const HVec3 proj3 = p3 - Distance(p3)*m_n;
	const HVec3 proj4 = p4 - Distance(p4)*m_n;
	const HVec3 proj5 = p5 - Distance(p5)*m_n;
	const HVec3 proj6 = p6 - Distance(p6)*m_n;
	const HVec3 proj7 = p7 - Distance(p7)*m_n;
	const HVec3 proj8 = p8 - Distance(p8)*m_n;*/

	const HVec3 proj1 = p1 - p1.Y()*upNorm;
	const HVec3 proj2 = p2 - p2.Y()*upNorm;
	const HVec3 proj3 = p3 - p3.Y()*upNorm;
	const HVec3 proj4 = p4 - p4.Y()*upNorm;
	const HVec3 proj5 = p5 - p5.Y()*upNorm;
	const HVec3 proj6 = p6 - p6.Y()*upNorm;
	const HVec3 proj7 = p7 - p7.Y()*upNorm;
	const HVec3 proj8 = p8 - p8.Y()*upNorm;


	const HVec3 tmax1 = Max(proj1, proj2);
	const HVec3 tmin1 = Min(proj1, proj2);
	const HVec3 tmax2 = Max(proj3, proj4);
	const HVec3 tmin2 = Min(proj3, proj4);
	const HVec3 tmax3 = Max(proj5, proj6);
	const HVec3 tmin3 = Min(proj5, proj6);
	const HVec3 tmax4 = Max(proj7, proj8);
	const HVec3 tmin4 = Min(proj7, proj8);

	const HVec3 qmax1 = Max(tmax1, tmax2);
	const HVec3 qmin1 = Min(tmin1, tmin2);
	const HVec3 qmax2 = Max(tmax3, tmax4);
	const HVec3 qmin2 = Min(tmin3, tmin4);

	const HVec3 max = Max(qmax1, qmax2);
	const HVec3 min = Min(qmin1, qmin2);

	const HVec3 mid = (max + min)*Constants<HF32>::Half();

	const HVec3 dif = max - min;
	const HF32 length =dif.Length();
	const HVec3 n = dif/length;

	const HVec3 dir = upNorm ^ n;

	const C_Normal d(dir);

	const HVec3 v1 =  d*length;
	const HVec3 v2 =  - d*length;

	const HVec3 vmin = min - mid;
	const HVec3 vmax = max - mid;

	aVex = (mid + vmax + v1) * trans;
	bVex = (mid + vmax + v2) * trans;
	cVex = (mid + vmin + v2) * trans;
	dVex = (mid + vmin + v1) * trans;
}

bool PointInTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c)
{
	
	using namespace Maths;

	const HVec3 v0(c-a);
	const HVec3 v1(b-a);	
	const HVec3 v2(p-a);
	
	const HF32 v0v0(v0|v0);
	const HF32 v1v1(v1|v1);
	const HF32 v0v1(v0|v1);
	const HF32 v2v1(v2|v1);
	const HF32 v2v0(v2|v0);

	const HF32 demon(v0v0*v1v1 - v0v1*v0v1);

	const HF32 nmonU(v1v1*v2v0 - v0v1*v2v1);
	const HF32 nmonV(v0v0*v2v1 - v0v1*v2v0);

	const HF32 u(nmonU/demon);
	const HF32 v(nmonV/demon);

	
	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const HF32 one(Maths::Constants<Maths::HF32>::One());
	const HF32 negEpsilon(-0.0001f);
	return (u.GE(negEpsilon) ) && (v.GE(negEpsilon) ) && ((u + v).LE(one));

}

bool PointInsideTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c)
{
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
	const Maths::HVec3 v0(c-a);
	const Maths::HVec3 v1(b-a);	
	const Maths::HVec3 v2(p-a);
	
	const Maths::HF32 v0v0(v0|v0);
	const Maths::HF32 v1v1(v1|v1);
	const Maths::HF32 v0v1(v0|v1);
	const Maths::HF32 v2v1(v2|v1);
	const Maths::HF32 v2v0(v2|v0);

	const Maths::HF32 demon(Maths::Reciprocal(v0v0*v1v1 - v0v1*v0v1));

	const Maths::HF32 nmonU(v1v1*v2v0 - v0v1*v2v1);
	const Maths::HF32 nmonV(v0v0*v2v1 - v0v1*v2v0);

	const Maths::HF32 u(nmonU*demon);
	const Maths::HF32 v(nmonV*demon);

	return (u.GE(zero)) & (v.GE(zero)) & ((u + v).LE(one));

}

bool TomasIntersectTriangleTriangle(const Maths::HVec3 PCREF t10, const Maths::HVec3 PCREF t11, const Maths::HVec3 PCREF t12, const Maths::HVec3 PCREF t20, const Maths::HVec3 PCREF t21, const Maths::HVec3 PCREF t22)
{
	using namespace Maths;

	const HF32 zero(Maths::Constants<HF32>::Zero());
	//calculate the normal of the triangle t1 plane
	const HVec3 vt10(t11-t10);
	const HVec3 vt11(t12-t10);
	const HVec3 n1(vt10^vt11);
	const HF32 d1=-(n1|t10); //the constant-normal form of the t1 plane


	//calculate the normal of the traingle t2 plane
	const HVec3 vt20=t21-t20;
	const HVec3 vt21=t22-t20;
	const HVec3 n2=vt20^vt21;
	const HF32 d2=-(n2|t20);//the constant-normal form of the t2 plane

	//the signed distances of each individual vertex of t1 to the t2 plane
	const HF32 dt10= (n2|t10) + d2;
	const HF32 dt11= (n2|t11) + d2;
	const HF32 dt12= (n2|t12) + d2;

	const HF32 mt101=dt10*dt11;
	const HF32 mt102=dt10*dt12;
	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
	if(mt101.GT(zero) && mt102.GT(zero)) 
		return false;

	const HF32 dt20= (n1|t20) + d1;
	const HF32 dt21= (n1|t21) + d1;
	const HF32 dt22= (n1|t22) + d1;

	const HF32 mt201=dt20*dt21;
	const HF32 mt202=dt20*dt22;
	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
	if(mt201.GT(zero) && mt202.GT(zero))
		return false;

	/*
		the code below insure t10 is always on the other side of the t2 plane
	*/
	//HVec3 nt10(t10);
	//HVec3 nt11(t11);
	//HVec3 nt12(t12);
	//{
	//	const HVec3 temp(nt10);
	//	//t10 and t12 is on the same side and t11 on the other side
	//	/*const Scalar_Bool b(mt101.LT(zero) && mt102.GT(zero));
	//	const VEec b2(mt101.GT(zero) && mt102.LT(zero));
	//	nt10 = Select(b, t11, Select(b2, t12, nt10));
	//	nt11 = Select(b, temp, nt11);
	//	nt12 = Select((!b) && b2, temp, nt12);
	//*/
	//	if(mt101.LT(zero) && mt102.GT(zero))
	//	{
	//		//swap the value of t10 and t11
	//		nt10=t11;
	//		nt11=temp;
	//	}
	//	else if(mt101.GT(zero)&&mt102.LT(zero))//t10 and t11 on the same side and t12 on the other side
	//	{
	//		nt10=t12;
	//		nt12=temp;
	//	}
	//}

	const Scalar_Bool swap11(mt101.LT(zero) & mt102.GT(zero));
	const Scalar_Bool swap12(mt101.GT(zero) & mt102.LT(zero));
	const HVec3 nt10 =Select(swap11, t11, Select(swap12, t12, t10));
	const HVec3 nt11 =Select(swap11, t10, t11);
	const HVec3 nt12 =Select(swap12, t10, t12);
	

	/*
		the code below insure t20 is always on the other side of the t1 plane
	*/
	
	//HVec3 nt20(t20);
	//HVec3 nt21(t21);
	//HVec3 nt22(t22);
	//{
	//	const HVec3 temp(nt20);
	//	/*const Scalar_Bool b(mt201.LT(zero) && mt202.GT(zero));
	//	const Scalar_Bool b2(mt201.GT(zero)&&mt202.LT(zero));
	//	nt20 = Select(b, nt21, Select(b2, nt22, nt20));
	//	nt21 = Select(b, temp, nt21);
	//	nt22 = Select((!b) && b2, temp, nt22);*/
	//	//t20 and t22 is on the same side and t21 on the other side
	//	if(mt201.LT(zero) && mt202.GT(zero))
	//	{
	//		//swap the value of t10 and t11
	//		
	//		nt20=nt21;
	//		nt21=temp;
	//	}
	//	else if(mt201.GT(zero)&&mt202.LT(zero))//t20 and t21 on the same side and t22 on the other side
	//	{
	//		//const HVec3 temp(nt20);
	//		nt20=t22;
	//		nt22=temp;
	//	}
	//}

	const Scalar_Bool swap21(mt201.LT(zero) & mt202.GT(zero));
	const Scalar_Bool swap22(mt201.GT(zero) & mt202.LT(zero));
	const HVec3 nt20 =Select(swap21, t21, Select(swap22, t22, t20));
	const HVec3 nt21 =Select(swap21, t20, t21);
	const HVec3 nt22 =Select(swap22, t20, t22);
	
	/*
		after the above test, the t1 plane and the t2 plane will intersect in a line, whose direction is the
		cross product of these two plane's normal
	*/
	const HVec3 dir=n1^n2;

	//calculate the intersect intervals of t1 and the line
	const HF32 pt10=dir|nt10;
	const HF32 pt11=dir|nt11;
	const HF32 pt12=dir|nt12;

	const HF32 _t1_min=pt10+(pt11-pt10)*(dt10/(dt10-dt11));
	const HF32 _t1_max=pt10+(pt12-pt10)*(dt10/(dt10-dt12));
	const HF32 t1_min=Min(_t1_min, _t1_max);
	const HF32 t1_max=Max(_t1_min, _t1_max);

	const HF32 pt20=dir|nt20;
	const HF32 pt21=dir|nt21;
	const HF32 pt22=dir|nt22;

	const HF32 _t2_min=pt20+(pt21-pt20)*(dt20/(dt20-dt21));
	const HF32 _t2_max=pt20+(pt22-pt20)*(dt20/(dt20-dt22));
	const HF32 t2_min=Min(_t2_min, _t2_max);
	const HF32 t2_max=Max(_t2_min, _t2_max);

	return t2_max.GE(t1_min) && t2_min.LE(t1_max);
}

bool CEIntersectTriangleSegment(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HVec3& res)
{	
	using namespace Maths;

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HVec3 qp(p-q);
	const HVec3 ab(b-a);
	const HVec3 ac(c-a);

	HVec3 normal(ab^ac);
	
	HF32 d(qp|normal);
	if(d.Abs().LE(Constants<HF32>::Epsilon2()))
		return false;
	
	const HVec3 ap(p-a);
	const HF32 t(ap|normal);
	/*if(t.LT(zero))
		return false;
	if(t.GT(d))
		return false;*/

	if(t.LT(zero) | t.GT(d))
		return false;

	const HVec3 e(qp^ap);
	const HF32 v(ac|e);
	if(v.LT(zero) | v.GT(d))
		return false;
	const HF32 w(-(ab|e));
	if(w.LT(zero)|(v+w).GT(d))
		return false;

	const HF32 recipD(d.Reciprocal());
	res=p - qp*t*recipD;
	return true;
}

bool IntersectTriangleSegment(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c,  const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q)
{
	using namespace Maths;

	const HVec3 pa(p-a);
	const HVec3 ba(b-a);
	const HVec3 ca(c-a);
	const HVec3 n(ba^ca);
	const HVec3 pq(p-q);
	
	const HVec3 pb(p-b);
	const HVec3 pc(p-c);
	
	const HVec3 pqpc(pq^pc);
	const HVec3 pqpb(pq^pb);

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 sign=n|pa;
	/*
	if sign>0, triangle abc is counter clockwise
	else, abc is clockwise
	*/
	const HF32 u(pqpc|pb); //triple scalar [pq, pc, pb]
	if((u*sign).GT(zero))
		return false;
	
	const HF32 v(-(pqpc|pa)); //triple scalar [pq, pa, pc] = -[pq, pc, pa]
	if((v*sign).GT(zero))
		return false;

	const HF32 w(pqpb|pa); //triple scalar [pq, pb, pa]
	return !((w*sign).GT(zero)==1);

}


bool IntersectCapsuleTriangle(const C_Capsule& capsule, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c)
{
	using namespace Maths;

	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	HF32 sqDist(Maths::UNDEFINED);
	const HF32 r(capsule.Radius());

	ClosestPtSegmentTriangle(capsule.Start(), capsule.End(), a, b, c, closestA, closestB, sqDist);
	return sqDist.LT(r*r);
}

bool CollideCapsuleSphere(const C_Capsule& capsule, const C_Sphere& sphere)
{
	using namespace Maths;
	const HVec3 c_start(capsule.Start());
	const HVec3 c_end(capsule.End());
	const HVec3 center(sphere.Centre());
	const HF32 radius(sphere.Radius());
	const HF32 r(radius+capsule.Radius());
	HVec3 closest(UNDEFINED);
	ClosestPtPointSegment(center, c_start, c_end, closest);
	const HVec3 temp(closest-center);
	const HF32 sqDist(temp|temp);
	if(sqDist > r*r)
		return false;		//non collide

	return true;
}

//bool CollideSphereSphere(const C_Sphere& sphereA, const C_Sphere& sphereB)
//{
//	const Maths::HVec3 d(sphereB.Centre() - sphereA.Centre());
//	const Maths::HF32 rad(sphereA.Radius() + sphereB.Radius());
//	const Maths::HF32 d2(d | d);
//	return d2 > (rad * rad);
//	/*if(d2.GT(rad * rad))
//		return false;
//
//	return true;*/
//}

bool CollideOBBSphere(const C_OBB& obb, const C_Sphere& sphere)
{
	/*
		P is a point in the world space, Q is the corresponding point represented in the local space.
		Therefore, 
		P = M*Q (M is the translation matrix)
		Q=Inver(M)*P;
	*/

	using namespace Maths;
	//const Maths::HVec3 lc(obb.u * (sphere.Centre()) ;
	const HVec3 d(sphere.Centre() - obb.Centre());
	const HMat33& u=obb.Orientation();

	//cL is the point representing sphere.c in the local space
	const HVec3 cL(u * d); //M*d(Math) = d*M(Implementation), d*Inver(M) = M*d 
	const HVec3 e(obb.Extent());
	//const HVec3 ne(-e);

	//Get the projection of cL of box obb
	const Maths::HVec3 proj(Max(-e, Min(cL, e)));

	const Maths::HVec3 contactPoint(obb.Centre() + proj*u);

	const Maths::HVec3 v(sphere.Centre() - contactPoint);
	//const Maths::HF32 length(v.Length());
	const Maths::HF32 l2(v | v);
	const Maths::HF32 r2(sphere.Radius() * sphere.Radius());

	return l2 <= r2;
	/*if(l2.GT(r2))
		return false;

	return true;*/
	
}

bool CollideCylinderSphere(const C_Cylinder& cylinder, const C_Sphere& sphere)
{
	using namespace Maths;
	
	const HF32 sRadius(sphere.Radius());
	const HF32 cRadius(cylinder.Radius());
	const HF32 cHalfHeight(cylinder.HalfHeight());
	const HVec3 v(sphere.Centre() - cylinder.Centre());
	const HF32 dist(v|cylinder.CentreAxis());

	const HF32 sqC(v|v);
	const HF32 sqB(dist*dist);
	const HF32 sqA(sqC - sqB);

	const HF32 sqDial(cRadius*cRadius + cHalfHeight*cHalfHeight);
	const HF32 dial(sqDial.Sqrt());

	const HF32 con2Radius(sRadius + cRadius);
	const HF32 sqCon2Radius(con2Radius * con2Radius);

	const HF32 con3Radius(sRadius + dial);
	const HF32 sqCon3Radius(con3Radius * con3Radius);

	const Scalar_Bool con1((sRadius + cHalfHeight).LT(dist));
	const Scalar_Bool con2(sqCon2Radius.LT(sqA));
	const Scalar_Bool con3(sqCon3Radius.LT(sqC));

	return !(con1|con2|con3);

}

bool CollideConeSphere(const C_Cone& cone, const C_Sphere& sphere )
{
	using namespace Maths;

	const HVec3 sCenter(sphere.Centre());
	const HVec3 dir(sCenter - cone.Centre());

	const HVec3 conner = cone.Support(dir);
	HVec3 u1(cone.Bottom() - conner);
	HVec3 u2(cone.Tip() - conner);

	const HVec3 v(sCenter - conner);

	const HF32 dist1(v|u1);
	const HF32 dist2(v|u2);
	const HF32 sRadius(sphere.Radius());
	const HF32 sqDistC(v|v);
	const HF32 sqRadius(sRadius*sRadius);
	
	const HF32 zero(Constants<HF32>::Zero());

	const HF32 mulDist(dist1*dist2);

	if(mulDist.GT(zero))
	{
		return dist1.GE(zero) | sqDistC.LE(sqRadius);
		/*if(dist1.LT(zero))
			return sqDistC.LE(sqRadius);
		else
			return true;*/
	}

	HVec3 u(Select(dist1.GT(zero), u1, u2));
	u.Normalise();
	const HF32 dist(v|u);
	const HF32 sqDistA(dist*dist);
	const HF32 sqDistB(sqDistC - sqDistA);


	return sqDistB.LE(sqRadius);
}

bool CollideConvexOBBTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, 
		const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF centre, const Maths::HVec3 PCREF extents,
		const Maths::HMat33 PCREF rot)
{
	using namespace Maths;

	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	const HVec3 ca(centre);
	const HVec3 e(extents);
	
	//Translate triangle as conceptually moving AABB to origin
	const HVec3 v0(rot * (a-ca)); 
	const HVec3 v1(rot * (b-ca));
	const HVec3 v2(rot * (c-ca));

	const HVec3 max012(Max(v0, Max(v1, v2)));
	const HVec3 min012(Min(v0, Min(v1, v2)));
	//Test axis u[0]=(1,0,0), u[1]={0,1,0}, u[2]={0, 0, 1}
	if(Maths::AnyTrue3((max012 < (-e) | min012 > (e))))
		return false;



	//Compute edge vectors for triangle
	const HVec3 f0(v1-v0);
	const HVec3 f1(v2-v1);
	const HVec3 f2(v0-v2);
	

	const HVec3 absF0(Maths::Abs(f0));
	const HVec3 crossP00(v0 ^ f0);
	const HVec3 crossP01(v2 ^ f0);

	const HVec3 crossRA0(e.Y()*absF0.Z() + e.Z()*absF0.Y(),
			e.X()*absF0.Z()+e.Z()*absF0.X(),
			e.X()*absF0.Y()+e.Y()*absF0.X());

	const Maths::HVec3 vMin0(Maths::Min(crossP00, crossP01));
	const Maths::HVec3 vMax0(Maths::Max(crossP00, crossP01));

	if(Maths::AnyTrue3((vMax0 < (-crossRA0)) | (vMin0 > crossRA0)))
		return false;

	const HVec3 absF1(Maths::Abs(f1));
	//Test axis a01=u0 X f1, all triangle vertexs project onto axis a01
	const HVec3 crossP10(v0^f1);
	const HVec3 crossP11(v1^f1);

	const HVec3 crossRA1(e.Y()*absF1.Z() + e.Z()*absF1.Y(),
			e.X()*absF1.Z()+e.Z()*absF1.X(),
			e.X()*absF1.Y()+e.Y()*absF1.X());


	const HVec3 vMax1(Max(crossP10, crossP11));
	const HVec3 vMin1(Min(crossP10, crossP11));

	if(Maths::AnyTrue3((vMax1 < (-crossRA1)) | (vMin1 > crossRA1)))
		return false;


	const HVec3 absF2(Maths::Abs(f2));
	//Test axis a01=u0 X f1, all triangle vertexs project onto axis a01
	const HVec3 crossP20(v0^f2);
	const HVec3 crossP21(v1^f2);


	const HVec3 vMax2(Max(crossP20, crossP21));
	const HVec3 vMin2(Min(crossP20, crossP21));

	const HVec3 crossRA2(e.Y()*absF2.Z() + e.Z()*absF2.Y(),
			e.X()*absF2.Z()+e.Z()*absF2.X(),
			e.X()*absF2.Y()+e.Y()*absF2.X());

	if(Maths::AnyTrue3((vMax2 < (-crossRA2)) | (vMin2 > crossRA2)))
		return false;
	
	//Test plane separation test - probably not that useful!
	const HVec3 n(f0^f1);
	const HF32 d(n|v0);
	const HVec3 absn(Abs(n));
	//const HF32 r=ex*Abs(n.X())+ey*Abs(n.Y())+ez*Abs(n.Z());
	const HF32 r(e|absn);
	//const HF32 s=(n|aabb.c)-d;
	return Abs(d).LE(r);
}



}; // namespace Collision