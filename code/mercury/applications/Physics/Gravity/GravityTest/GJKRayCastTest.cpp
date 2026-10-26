#include "stdafx.h"

#include "SectionPhysics.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/GJK.inl"
#include "TriangleSetting.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/narrowphase/PersistentContactManifold.h"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>


namespace Samples
{
	void C_SectionPhysics::GJKRayCastCapsuleTriangleTest()
	{
		using namespace Maths;

		const HVec3 a=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
		const HVec3 b=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
		const HVec3 c=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));

		const HF32 _lambda = HF32(triLambda);

		Collision::C_Triangle triangle;
		triangle.m_VertexA = a;
		triangle.m_VertexB = b;
		triangle.m_VertexC = c;

		triangle.epsilon = 0.001f;

		const HVec3 ab = b-a;
		const HVec3 ac = c-a;
		HVec3 t = Constants<HVec3>::Zero();;

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(a, b, c, RGBA(255, 0, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(triangle.Centre(), t, 1.f, 0.25f, RGBA(255, 255, 0));

		Collision::C_Capsule capsule;
		capsule.m_Start=HVec3(3.f, 4.f, -2.f);
		capsule.m_End=HVec3(3.f, 3.f, 2.f);

		/*capsule.m_Start=HVec3(9.f, 4.f, -2.f);
		capsule.m_End=HVec3(9.f, 3.f, 2.f);*/
		capsule.m_Radius=1.f;

		const HVec3 na(capsule.m_End-capsule.m_Start);
		capsule.epsilon = (na.Length() + capsule.m_Radius) * 0.01f;

		const HVec3 center = (capsule.m_End + capsule.m_Start)*Constants<HF32>::Half();

		HVec3 s(-6.f, 3.f, 9.f);

		HF32 _zero(Constants<HF32>::Zero());
		HF32 _one(Constants<HF32>::One());
		HVec3 zero(Constants<HVec3>::Zero());
		HF32 maxLambda(_one);
		HF32 lambda(_zero);
		HVec3 normal(zero);
		HVec3 closestA(zero);
		HVec3 closestB(zero);

		//HVec3 r = center - triangle.Centre();
		HVec3 r = t - s;

		bool collided = Collision::GJKRayCast<Collision::C_Capsule, Collision::C_Triangle>(capsule, triangle,  r , maxLambda, lambda, normal, closestA, closestB);

		//if(collided)
		{

			HF32 base(20.f);
			{
				for(int i=0; i<20; i++)
				{
					HF32 ratio(((HF32)i)/base);
					HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(0, 255, 0));
				}
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(0, 255, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(0, 255, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(0, 255, 0));
			}
		
			lambda = lambda + _lambda;
			if(collided)
			{
				Maths::HVec3 _s(s * lambda);
				for(int i=0; i<20; i++)
				{
					HF32 ratio(((HF32)i)/base);
					HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio + _s);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(0, 0, 255));
				}
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start + _s, capsule.m_End + _s, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start + _s, capsule.m_Radius, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End + _s, capsule.m_Radius, RGBA(0, 0, 255));

				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_Start + _s, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_End, capsule.m_End + _s, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA + _s, 0.5f, RGBA(0, 0, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(closestA + _s, normal, 1.f, 0.2f, RGBA(0, 255, 0));

				
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.5f, RGBA(0, 0, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.5f, RGBA(255, 255, 255));
			}
			else
			{
				Maths::HVec3 _s(s * lambda);
				for(int i=0; i<20; i++)
				{
					HF32 ratio(((HF32)i)/base);
					HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio + _s);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 255));
				}
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start + _s, capsule.m_End + _s, RGBA(255, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start + _s, capsule.m_Radius, RGBA(255, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End + _s, capsule.m_Radius, RGBA(255, 0, 255));

				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_Start + _s, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_End, capsule.m_End + _s, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA + _s, 0.5f, RGBA(0, 0, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(closestA + _s, normal, 1.f, 0.2f, RGBA(0, 255, 0));

				
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.5f, RGBA(0, 0, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.5f, RGBA(255, 255, 255));
			}

		}
		/*else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(center, s, 1.f, 0.25f, RGBA(0, 255, 0));
			HF32 base(20.f);
			for(int i=0; i<20; i++)
			{
				HF32 ratio(((HF32)i)/base);
				HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 0));
			}
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(255, 0, 0));
		}*/

		{
			Maths::HVec3 _s(s);
			HF32 base(20.f);
			for(int i=0; i<20; i++)
			{
				HF32 ratio(((HF32)i)/base);
				HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio + _s);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(0, 0, 255));
			}
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start + _s, capsule.m_End + _s, RGBA(0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start + _s, capsule.m_Radius, RGBA(0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End + _s, capsule.m_Radius, RGBA(0, 0, 255));

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_Start + _s, RGBA(0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_End, capsule.m_End + _s, RGBA(0, 0, 255));
		}
	}


	void C_SectionPhysics::GJKRayCastOBBTriangleTest()
	{
		using namespace Maths;

		const HVec3 a=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
		const HVec3 b=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
		const HVec3 c=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));

		Collision::C_Triangle triangle;
		triangle.m_VertexA = a;
		triangle.m_VertexB = b;
		triangle.m_VertexC = c;
		triangle.epsilon = 0.001f;

		const HVec3 ab = b-a;
		const HVec3 ac = c-a;
		HVec3 t = Constants<HVec3>::Zero();;

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(a, b, c, RGBA(255, 0, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(triangle.Centre(), t, 1.f, 0.25f, RGBA(255, 255, 0));

		Collision::C_OBB boxA;
		boxA.c=HVec3(3.f, 5.f, -2.f);
		boxA.u=HMat33(UNDEFINED);
		boxA.u.SetIdentity();
		boxA.e=HVec3(2.f, 3.f, 4.f);
		boxA.epsilon = Maths::Min(boxA.e.X(), Maths::Min(boxA.e.Y(), boxA.e.Z())) * 0.01f;

		HVec3 s(-6.f, 3.f, 9.f);

		HF32 _one(Constants<HF32>::One());
		HF32 _zero(Constants<HF32>::Zero());
		HVec3 zero(Constants<HVec3>::Zero());
		HF32 maxLambda(_one);
		HF32 lambda(_zero);
		HVec3 normal(zero);
		HVec3 closestA(zero);
		HVec3 closestB(zero);

		//HVec3 r = center - triangle.Centre();
		HVec3 r = t - s;
	
		bool collided = Collision::GJKRayCast<Collision::C_OBB, Collision::C_Triangle>(boxA, triangle, r , maxLambda, lambda,  normal, closestA, closestB);

		HMat43 matA(boxA.u, boxA.c);
		HVec3 minA(-boxA.e);
		HVec3 maxA(boxA.e);

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.5f, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.5f, RGBA(255, 255, 255));
		
		if(collided)
		{
			Maths::HVec3 _s(s * lambda);
			HMat43 matA(boxA.u, boxA.c + _s);
			HVec3 minA(-boxA.e);
			HVec3 maxA(boxA.e);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA + _s, 0.5f, RGBA(0,0,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(closestA + _s, normal, 1.f, 0.2f, RGBA(0, 255, 0));
			
		}
		
		{
			HMat43 matA(boxA.u, boxA.c + s);
			HVec3 minA(-boxA.e);
			HVec3 maxA(boxA.e);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(255, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(boxA.c, boxA.c+s, Maths::RGBA(0, 0, 255));
		}
	}


	void C_SectionPhysics::GJKRayCastCapsuleOBBTest()
	{
		using namespace Maths;

	
		Collision::C_OBB boxA;
		boxA.c=HVec3(3.f, 5.f, 8.f);
		boxA.u=HMat33(UNDEFINED);
		boxA.u.SetIdentity();
		boxA.e=HVec3(2.f, 3.f, 4.f);

		HVec3 t = Constants<HVec3>::Zero();
		boxA.epsilon = Maths::Min(boxA.e.X(), Maths::Min(boxA.e.Y(), boxA.e.Z())) * 0.01f;

		Collision::C_Capsule capsule;
		capsule.m_Start=HVec3(3.f, 4.f, -2.f);
		capsule.m_End=HVec3(3.f, 3.f, 2.f);
		capsule.m_Radius=1.f;

		const HVec3 v (capsule.m_End - capsule.m_Start);
		capsule.epsilon = (v.Length() + capsule.m_Radius) * 0.01f;

		HVec3 na=capsule.m_End-capsule.m_Start;
		const HVec3 center = (capsule.m_End + capsule.m_Start)*Constants<HF32>::Half();

		HVec3 s(-6.f, 3.f, 9.f);
		//HVec3 s(6.f, -3.f, -9.f);

		HF32 _one(Constants<HF32>::One());
		HF32 _zero(Constants<HF32>::Zero());
		HVec3 zero(Constants<HVec3>::Zero());
		HF32 maxLambda(_one);
		HF32 lambda(_zero);
		HVec3 normal(zero);
		HVec3 closestA(zero);
		HVec3 closestB(zero);

		//HVec3 r = center - triangle.Centre();
		HVec3 r = t - s;
	
		bool collided = Collision::GJKRayCast<Collision::C_Capsule, Collision::C_OBB>(capsule, boxA, r , maxLambda, lambda,  normal, closestA, closestB);

		{
			HMat43 matA(boxA.u, boxA.c);
			HVec3 minA(-boxA.e);
			HVec3 maxA(boxA.e);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(255, 0, 0));

			HF32 base(20.f);
			{
				for(int i=0; i<20; i++)
				{
					HF32 ratio(((HF32)i)/base);
					HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(0, 255, 0));
				}
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(0, 255, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(0, 255, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(0, 255, 0));
			}
		
			{

				Maths::HVec3 _s(s * lambda);
				for(int i=0; i<20; i++)
				{
					HF32 ratio(((HF32)i)/base);
					HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio + _s);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(0, 0, 255));
				}
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start + _s, capsule.m_End + _s, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start + _s, capsule.m_Radius, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End + _s, capsule.m_Radius, RGBA(0, 0, 255));

				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_Start + _s, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_End, capsule.m_End + _s, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA + _s, 0.5f, RGBA(0,0,0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(closestA + _s, normal, 1.f, 0.2f, RGBA(0, 255, 0));
			}

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.5f, RGBA(0, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.5f, RGBA(255, 255, 255));

			
		}
		/*else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(center, s, 1.f, 0.25f, RGBA(0, 255, 0));
			HF32 base(20.f);
			for(int i=0; i<20; i++)
			{
				HF32 ratio(((HF32)i)/base);
				HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 0));
			}
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(255, 0, 0));
		}*/

		{
			Maths::HVec3 _s(s);
			HF32 base(20.f);
			for(int i=0; i<20; i++)
			{
				HF32 ratio(((HF32)i)/base);
				HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio + _s);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(0, 0, 255));
			}
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start + _s, capsule.m_End + _s, RGBA(0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start + _s, capsule.m_Radius, RGBA(0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End + _s, capsule.m_Radius, RGBA(0, 0, 255));

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_Start + _s, RGBA(0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_End, capsule.m_End + _s, RGBA(0, 0, 255));
		}
	}
}