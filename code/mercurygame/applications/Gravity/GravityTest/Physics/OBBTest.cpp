#include <stdafx.h>

#define VERIFY_POINTS

#include "SectionPhysics.h"
#include "gravity/narrowphase/PrimitiveTest.h"
//#include "gravity/narrowphase/PrimitiveTest.inl"
#include "gravity/narrowphase/OBBSphere.h"
//#include "gravity/narrowphase/OBBSphere.inl"
#include "gravity/narrowphase/OBBCapsule.h"
//#include "gravity/narrowphase/OBBCapsule.inl"
#include "gravity/sharedphysics/SortedArray.inl"
#include "gravity/narrowphase/OBBTriangle.h"
#include "gravity/narrowphase/OBBTriangle.inl"
#include "gravity/narrowphase/OBBOBB.inl"
#include "Core/Maths/Hvec/VectorInt.h"
#include "Core/Maths/Hvec/VectorInt.inl"

#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
//#include <Render/Shaders/World/psWorld.h>
//#include <Render/Shaders/World/vsWorld.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/sharedphysics/ContactStreamIterator.h"
#include "gravity/sharedphysics/ContactStreamIterator.inl"
#include "TriangleSetting.h"
#include "CapsuleSetting.h"
#include "SphereSetting.h"
#include "SegmentSetting.h"
#include "gravity/narrowphase/OBBRay.h"
//#include "gravity/narrowphase/OBBRay.inl"
#include "gravity/sharedphysics/AllocationManager.inl"
#include "OBBSetting.h"
#include <Framework/Application/UserControls.h>

using namespace Baking;
using namespace Collision;

Debug::Logger &g_Log = Debug::GetLogger("michelle");

Debug::Bool g_TestOnSurface("Test.TestOnSurface", false);
Debug::Bool g_TestInside("Test.TestInside", false);


void C_SectionPhysics::GJKMarginOBB()
{
	static bool bFirstFrame = true;

	static C_OBB boxA;
	static C_OBB boxB;
	if(bFirstFrame)
	{
		boxA.c=HVec3(-1.f, 5.f, -2.f);
		//boxA.c=HVec3(0.f, 0.f, 0.f);
		boxA.u.SetIdentity();
		boxA.e=HVec3(0.1f,0.2f, 0.1f);
		//boxA.e=HVec3(1.f, 1.f, 1.f);
		boxA.Margin(0.0f);
		boxA.Epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		//
		//if(!bFirstFrame)
		{
			boxA.WarmStart(boxA.c);
		}

		
		boxB.c=HVec3(-1.05f, 5.f, -2.f);
		//boxB.c=HVec3(1.f, 0.f, 0.f);
		boxB.u.SetIdentity();
		boxB.e=HVec3(0.1f, 0.2f, 0.1f);
		boxB.Epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		//boxB.e=HVec3(1.f, 1.f, 1.f);
		//boxB[0]=boxB.c;
		boxB.Margin(0.0f);
		//if(!bFirstFrame)
		{
			boxB.WarmStart(boxB.c);
			//bFirstFrame = true;
		}
		bFirstFrame = false;
	}

	boxA.WarmStart(boxA.c);
	boxB.WarmStart(boxB.c);


	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	Input::C_GameInput* gameInput = Input::GetGameInput(0);

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA))
	{
		rot-=0.02f;
	}

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB))
	{
		rot+=0.02f;
	}
	
	const Maths::RotZ rotya(rot);
	HMat33 matra(rotya);
	const Maths::RotX rotxa(rot);
	HMat33 m(rotxa);
	boxA.u=matra*m;

	const HVec3 zero(Maths::Constants<HVec3>::Zero());
	HF32 penetration;
	
	C_CollisionResult<1, 32> result;
	

	bool s;
	EPA epa;
	bool fail=false; //GJK or EPA doesn't fail
	for(int i=0; i< 1; ++i)
	{
		C_ContactPatch<32>& patch(result.m_ContactPatches[0]);
		patch.m_NumContactPoints = 1;
		C_ContactPoint& point(patch.m_ContactPoints[0]);
		Maths::HF32 penetration;
		s=GJKCPairMargin<C_OBB, C_OBB>(boxA, boxB, point.m_ContactPoint, patch.m_Normal.m_V, penetration, epa);
		point.m_PenetrationDepth = penetration;
		if(s)
		{
			//s = Collision::CollideBoxBox(boxA, boxB, result, 0);

			const Maths::HVec3 q0(epa.aBuf[0]-epa.bBuf[0]);
			const Maths::HVec3 q1(epa.aBuf[1]-epa.bBuf[1]);
			const Maths::HVec3 q2(epa.aBuf[2]-epa.bBuf[2]);
			const Maths::HVec3 q3(epa.aBuf[3]-epa.bBuf[3]);
			const Maths::HVec3 q4(epa.aBuf[4]-epa.bBuf[4]);
			const Maths::HVec3 q5(epa.aBuf[5]-epa.bBuf[5]);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q0, RGBA(255, 0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q1, RGBA(255, 0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q2, RGBA(255, 0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q5, RGBA(255, 0, 0, 255));


			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q3, q0, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q3, q1, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q3, q2, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q3, q5, RGBA(255, 255, 0, 255));

			/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q0, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q1, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(q4, q2, RGBA(0, 255, 0, 255));
			*/

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(q0, q1, q2, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(q2, q1, q0, RGBA(255, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q0, Util::C_String("0"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q1, Util::C_String("1"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q2, Util::C_String("2"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q3, Util::C_String("3"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q4, Util::C_String("4"));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q5, Util::C_String("5"));


			const Maths::HVec3 c023((q0+q2+q3)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t0(q2-q3);
			const Maths::HVec3 t1(q0-q3);

			Maths::C_Normal n0(t0^t1);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c023, n0, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c213((q1+q3+q2)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t2(q1-q3);
			const Maths::HVec3 t3(q2-q3);

			Maths::C_Normal n2(t2^t3);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c213, n2, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c042((q0+q2+q4)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t4(q4-q2);
			const Maths::HVec3 t5(q0-q2);

			Maths::C_Normal n3(t4^t5);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c042, n3, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c241((q1+q4+q2)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t6(q4-q1);
			const Maths::HVec3 t7(q2-q1);

			Maths::C_Normal n4(t6^t7);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c241, n4, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c503((q5+q0+q3)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t8(q0-q3);
			const Maths::HVec3 t9(q5-q3);

			Maths::C_Normal n5(t8^t9);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c503, n5, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c153((q5+q3+q1)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t10(q5-q3);
			const Maths::HVec3 t11(q1-q3);

			Maths::C_Normal n6(t10^t11);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c153, n6, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c540((q5+q0+q4)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t12(q4-q0);
			const Maths::HVec3 t13(q5-q0);

			Maths::C_Normal n7(t12^t13);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c540, n7, 0.05f, 0.02f, RGBA(0, 255, 0, 255));

			const Maths::HVec3 c145((q5+q1+q4)/Maths::Constants<Maths::HF32>::Three());
			const Maths::HVec3 t14(q4-q5);
			const Maths::HVec3 t15(q1-q5);

			Maths::C_Normal n8(t14^t15);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(c145, n8, 0.05f, 0.02f, RGBA(0, 255, 0, 255));


			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(q0, q1, q5, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(q5, q1, q0, RGBA(0, 255, 0, 255));
			
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(q3, 0.05f, RGBA(0, 0, 255, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(q4, 0.05f, RGBA(0, 255, 255, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(q5, 0.05f, RGBA(255, 0, 255, 255));


			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(HVec3(0.f, 0.f, 0.f), 0.05f, RGBA(0, 0, 0, 255));

		}
	}

	//HF32 s=GJKCPair(boxA, boxB, dis, closestA, closestB);
	HMat43 matA(boxA.u, boxA.c);
	HVec3 minA(-boxA.e);
	HVec3 maxA(boxA.e);

	HMat43 matB(boxB.u, boxB.c);
	HVec3 minB(-boxB.e);
	HVec3 maxB(boxB.e);


	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(0, 255, 0));
	

	if(s)
	{
		C_ContactPatch<32>& patch(result.m_ContactPatches[0]);
		for(U32 a = 0; a < patch.m_NumContactPoints; ++a)
		{
			C_ContactPoint& point(patch.m_ContactPoints[a]);
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minBm, maxBm, matB, Maths::RGBA(0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point.m_ContactPoint, point.m_ContactPoint + patch.m_Normal * point.m_PenetrationDepth, Maths::RGBA(255, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.05f, Maths::RGBA(64, 0, 255));
		}
	}
}

void C_SectionPhysics::TestTriangleOBB()
{
	const HVec3 a(-9.f, 10.f, -7.f);
	const HVec3 b(4.f, 8.f, -4.f);
	const HVec3 d(-1.f, 4.f, -8.f );

	C_OBB boxA;
	boxA.c=HVec3(3.f, 5.f, -2.f);
	boxA.u=HMat33(UNDEFINED);
	boxA.u.SetIdentity();
	boxA.e=HVec3(2.f, 3.f, 4.f);

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;
	rot+=0.02f;

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	boxA.u=matra;
	

	HF32 s;
	for(int i=0; i<1000; ++i)
	{
		s=IntersectOBBTriangle(boxA, a, b,d);
	}
	HMat43 matA(boxA.u, boxA.c);
	HVec3 minA(-boxA.e);
	HVec3 maxA(boxA.e);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, d, Maths::RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, d, b, Maths::RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(b, 0.5f, Maths::RGBA(0, 255, 0));

	LOGINFO(g_Log, "%f", (F32)s);
}

void C_SectionPhysics::TestOBBSegment()
{
	C_OBB box;
	box.c=HVec3(3.f, 5.f, -2.f);
	box.u=HMat33(UNDEFINED);
	box.u.SetIdentity();
	box.e=HVec3(1.f, 2.f, 3.f);

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;
	rot+=0.02f;

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	box.u=matra;
	

	const HVec3 a(1.f, 5.f, -7.f);
	const HVec3 b(8.f, 6.f, -2.f);
	const HVec3 half(Maths::Constants<HVec3>::Half());
	HVec3 mid((a+b)*half);
	
	HF32 s1;
	HF32 s2;
	//HF32 s=IntersectAABBTriangle(box, a, d, b);
	//s=IntersectAABBTriangle2(box, a, d, b);
	for(int i=0; i<1000; ++i)
	{
		s1=IntersectOBBSegment(box, a, b);
	}
	
	//HMat33 matA(Maths::UNDEFINED);
	//matA.SetIdentity();

	HMat43 trans(box.u, box.c);

	HVec3 minA(-box.e);
	HVec3 maxA(box.e);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, trans, RGBA(0, 0, 255));

	if(s1)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, Maths::RGBA(255, 0, 0));
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, Maths::RGBA(0, 255, 0));
	}
	
	LOGINFO(g_Log, "%f", (F32)s1);

}

class C_OBBRayCollisionFilter : public C_CollisionFilter
{
public:

	C_OBBRayCollisionFilter() : C_CollisionFilter(0)
	{
	}

	virtual bool CanCollide(C_CollisionFilter* pOther)
	{
		return true;
	}
	//Triangles are given individual collision flags, which can be checked against
	virtual bool CanCollide(U32 collisionFlags)
	{
		return true;
	}
};


void C_SectionPhysics::TestOBBRay()
{
	C_OBBRayCollisionFilter filter;
	C_BoxShape shape(&filter);
	C_OBB& box=shape.m_Box;
	box.c=HVec3(3.f, 5.f, -2.f);
	box.u=HMat33(UNDEFINED);
	box.u.SetIdentity();
	box.e=HVec3(1.f, 2.f, 3.f);

	HMat43 trans(box.u, box.c);

	HVec3 minA(-box.e);
	HVec3 maxA(box.e);

	

	C_Ray ray;

	ray.m_start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	ray.m_end=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));

	bool collide;
	//for(U32 i=0; i<1000; ++i)
	{
		collide=CollideOBBRay(&shape, ray);
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, trans, RGBA(0, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(255,  0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_start, 0.05f, RGBA(0, 255, 0, 255));

	if(collide)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(255,  255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_start, 0.05f, RGBA(0, 255, 0, 255));
	}

}

void C_SectionPhysics::OBBsIntersectionTest()
{
	//Test GJK OBB OBB intersection
	C_OBB boxA;
	/*boxA.c=HVec3(3.f, 5.f, -2.f);
	boxA.u=HMat33(UNDEFINED);
	boxA.u.SetIdentity();
	boxA.e=HVec3(1.f, 2.f, 3.f);*/

	boxA.c=HVec3(HF32(obb_cX), HF32(obb_cY), HF32(obb_cZ));
	boxA.u=HMat33(UNDEFINED);
	boxA.u.SetIdentity();
	boxA.e=HVec3(HF32(obb_eX), HF32(obb_eY), HF32(obb_eZ));

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;
	rot+=0.02f;

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	boxA.u=matra;

	const HVec3 c=boxA.c;
	const HVec3 ea=boxA.e;
	const HMat33 u=boxA.u;


	C_OBB boxB;
	boxB.c=HVec3(2.f, 4.f, 3.f);
	boxB.u=HMat33(UNDEFINED);
	boxB.u.SetIdentity();
	boxB.e=HVec3(2.f, 1.f, 2.f);
	//boxB[0]=boxB.c;

	const Maths::RotY rotyb(rot);
	HMat33 matrb(rotyb);
	boxB.u=matrb;

	const HVec3 cb=boxB.c;
	const HVec3 eb=boxB.e;
	const HMat33 ub=boxB.u;

	
	
	HF32 s;
	for(int i=0; i<10000; ++i)
	{
		s=IntersectOBBs2(boxA, boxB);
	}


	HMat43 matA(boxA.u, boxA.c);
	HVec3 minA(-boxA.e);
	HVec3 maxA(boxA.e);

	HMat43 matB(boxB.u, boxB.c);
	HVec3 minB(-boxB.e);
	HVec3 maxB(boxB.e);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(0, 0, 255));

	if(s)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(255, 0, 0));
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(0, 255, 0));
	}
	//LOGINFO(g_MichelleLog, "%f", s1);

}

void C_SectionPhysics::TestOBBOBB()
{
	//const HVec3 a(-9.f, 10.f, -7.f);
	//const HVec3 b(4.f, 8.f, -4.f);
	//const HVec3 c(-1.f, 4.f, -8.f );

	C_OBB boxa;
	/*boxa.c=HVec3(3.f, 5.f, -2.f);
	boxa.u.SetIdentity();
	boxa.e=HVec3(2.f, 3.f, 4.f);*/

	boxa.c=HVec3(HF32(obb_cX), HF32(obb_cY), HF32(obb_cZ));
	boxa.u.SetIdentity();
	boxa.e=HVec3(HF32(obb_eX), HF32(obb_eY), HF32(obb_eZ));

	C_OBB boxb;
	boxb.c=HVec3(4.5f, 10.5f, -3.f);
	boxb.u.SetIdentity();
	boxb.e=HVec3(2.f, 3.f, 4.f);

	const F32 smll(0.02f);
	const HF32 _small(Load<HF32>(&smll));
	static HF32 rot=Maths::Constants<HF32>::Zero();
	static HF32 rotB = Maths::Constants<HF32>::Zero();
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	Input::C_GameInput* gameInput = Input::GetGameInput(0);

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA))
	{
		rot-=0.02f;
	}

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB))
	{
		rot+=0.02f;
	}

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonY))
	{
		rotB+=_small;
	}

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	boxa.u=matra;

	const Maths::RotX rotxb(rotB);
	HMat33 matrb(rotxb);
	boxb.u = matrb;
	
	
	const int MaxPatches=4;
	const int MaxContacts=32;
	const int DesiredPatches = 2;
	const int DesiredContacts = 4;
	SharedPhysics::C_ContactStream stream(4096);
	Collision::C_CollisionResult<MaxPatches, MaxContacts> result;

	Maths::HMat43 mata(boxa.u, boxa.c);
	Maths::HMat43 matb(boxb.u, boxb.c);

	Collision::C_CollisionObject objA, objB;
	SharedPhysics::C_GraphNode nodeA, nodeB;
	objA.m_pNodePtr = &nodeA;
	objB.m_pNodePtr = &nodeB;

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-boxa.e, boxa.e, mata, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-boxb.e, boxb.e, matb, Maths::RGBA(0, 255, 0));
	U32 materialIndex(0);
	//for(U32 a = 0; a < 100; ++a)
	//Collision::CollideBoxBox(boxa, boxb, result, materialIndex);
	//result.m_pObjectA = &objA;
	//result.m_pObjectB = &objB;
	//Collision::ReduceContactsAndPackToStream<MaxPatches, MaxContacts, DesiredPatches, DesiredContacts>(result, stream);
	//{
	//	//Draw points of contact...
	//	SharedPhysics::C_ContactStreamIterator iter(&stream);

	//	while(iter.hasNextContact())
	//	{
	//		SharedPhysics::C_ContactStreamResult* pResult = iter.nextContact();
	//		while(iter.hasNextPatch())
	//		{
	//			SharedPhysics::C_ContactStreamPatch* pPatch = iter.nextPatch();
	//			const Maths::HVec3 norm(Maths::Load<Maths::HVec3>(&pPatch->m_Normal));

	//			while(iter.hasNextPoint())
	//			{
	//				SharedPhysics::C_ContactStreamPoint* pPoint = iter.nextPoint();
	//				const Maths::HVec3 point(Maths::Load<Maths::HVec3>(&pPoint->m_ContactPoint));
	//				const Maths::HF32 penDepth(Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth));
	//				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point, 0.05f, Maths::RGBA(0,255, 255));
	//				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point, 
	//				point - (norm * penDepth), Maths::RGBA(0,255, 255));
	//			}
	//		}
	//	}

	//	/*C_ContactPatch<MaxContacts>& patch = result.m_ContactPatches[0];
	//	for(U32 a = 0; a < patch.m_NumContactPoints; ++a)
	//	{
	//		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(patch.m_ContactPoints[a].m_ContactPoint, 0.05f, Maths::RGBA(0,255, 255));
	//		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(patch.m_ContactPoints[a].m_ContactPoint, 
	//		patch.m_ContactPoints[a].m_ContactPoint + patch.m_Normal * Maths::HF32(5.f), Maths::RGBA(0,255, 255));
	//	}*/
	//}
}

void C_SectionPhysics::TestCollidedOBBTriangle()
{
	/*HVec3 a=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
	HVec3 b=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
	HVec3 c=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));


	C_OBB box;
	box.c=HVec3(3.f, 5.f, -2.f);
	box.u=HMat33(UNDEFINED);
	box.u.SetIdentity();
	box.e=HVec3(2.f, 3.f, 4.f);

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	box.u=matra;
	

	const int MaxPatches=8;
	const int MaxContacts=8;
	Collision::C_CollisionResult<MaxPatches, MaxContacts> result;
	bool collided;
	for(int i=0; i<10; ++i)
	{
		collided=Collision::CollideOBBTriangle(box, a, b,c, result);
	}
	HMat43 mat(box.u, box.c);
	HVec3 min(-box.e);
	HVec3 max(box.e);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(min, max, mat, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, Maths::RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, c, Maths::RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b, c, Maths::RGBA(0, 0, 255));

	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, Maths::RGBA(0, 255, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, c, Maths::RGBA(0, 255, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b, c, Maths::RGBA(0, 255, 255));
	}*/

}

void C_SectionPhysics::TestCollidedOBBCapsule()
{
	C_OBB box;
	box.c=HVec3(-1.f, 5.f, -2.f);
	box.u=HMat33(UNDEFINED);
	box.u.SetIdentity();
	box.e=HVec3(2.f, 3.f, 4.f);

	Collision::C_Capsule capsule;
	if(g_TestOnSurface)
	{
		capsule.m_Start=HVec3(HF32(4.f), HF32(4.6666f), HF32(-0.6664f));
		capsule.m_End=HVec3(HF32(2.f), HF32(5.3333f), HF32(-3.33334f));
	}
	else if(g_TestInside)
	{
		capsule.m_Start=HVec3(HF32(-1.33333f), HF32(4.f), HF32(-3.33334f));
		capsule.m_End=HVec3(HF32(-1.33333f), HF32(3.3333f), HF32(-2.f));
	}
	else
	{
		capsule.m_Start=HVec3(HF32(capsuleSX), HF32(capsuleSY), HF32(capsuleSZ));
		capsule.m_End=HVec3(HF32(capsuleEX), HF32(capsuleEY), HF32(capsuleEZ));
	}
	capsule.m_Radius=2.f;
	

	Input::C_GameInput* gameInput = Input::GetGameInput(0);
	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA))
	{
		rot-=0.02f;
	}

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB))
	{
		rot+=0.02f;
	}
	
	const Maths::RotZ rotza(rot);
	HMat33 matra(rotza);
	box.u=matra;

	const Maths::HVec3 _min(-box.e);
	const Maths::HVec3 _max(box.e);
	HMat43 mat(box.u, box.c);


	const int MaxContacts =2;
	const int MaxPatches=2;
	C_CollisionResult<MaxContacts, MaxPatches> results;

	bool collided=false;

	for(U32 i=0; i<10; ++i)
	{
		collided=CollideOBBCapsule(box, capsule, results);
	}

	
	C_ContactPatch<MaxContacts> & patch=results.m_ContactPatches[0];
	
	C_ContactPoint& point=patch.m_ContactPoints[0];

	const Maths::HVec3 v(point.m_ContactPoint - patch.m_Normal* point.m_PenetrationDepth);
	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, mat, Maths::RGBA(0, 0, 255));
	

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(capsule.m_Start, 0.05f, Maths::RGBA(0,255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(capsule.m_End, 0.05f, Maths::RGBA(0, 255, 255));

	const Maths::HVec3 d(capsule.m_End - capsule.m_Start);

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capsule.m_Start+ d*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, d, capsule.m_Radius, RGBA(255, 0, 0));
	}
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(0, 255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(0, 255, 0, 0));

	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point.m_ContactPoint, v, Maths::RGBA(0, 0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.05f, Maths::RGBA(64, 0, 255, 255));
	}
	
}

void C_SectionPhysics::TestCollidedOBBSphere()
{
	
	
	C_OBB box;
	box.c=HVec3(-1.f, 5.f, -2.f);
	box.u=HMat33(UNDEFINED);
	box.u.SetIdentity();
	box.e=HVec3(2.f, 3.f, 4.f);

	Collision::C_Sphere sphere;
	sphere.Centre(HVec3(HF32(sX), HF32(sY), HF32(sZ)));
	sphere.Radius(Maths::HF32(r));
	

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	Input::C_GameInput* gameInput = Input::GetGameInput(0);

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA))
	{
		rot-=0.02f;
	}

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB))
	{
		rot+=0.02f;
	}
	
	const Maths::RotZ rotza(rot);
	HMat33 matra(rotza);
	box.u=matra;

	const Maths::HVec3 _min(-box.e);
	const Maths::HVec3 _max(box.e);
	HMat43 mat(box.u, box.c);


	const int MaxContacts =2;
	const int MaxPatches=2;
	C_CollisionResult<MaxContacts, MaxPatches> results;

	HVec3 contact(Maths::UNDEFINED);
	C_Normal normal(Maths::UNDEFINED);
	HF32 pDepth(Maths::UNDEFINED);

	bool collided=CollideOBBSphere(box, sphere, contact, pDepth, normal);
	
	const Maths::HVec3 v(contact - normal* pDepth);
	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(sphere.Centre(), 0.05f, Maths::RGBA(64, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, mat, Maths::RGBA(0, 0, 255));

	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, v, Maths::RGBA(255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, Maths::RGBA(64, 255, 255));
	}

	
	
}

