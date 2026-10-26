#include "stdafx.h"

#include "SectionPhysics.h"
#include "TriangleSetting.h"
#include "SegmentSetting.h"
#include "gravity/narrowphase/ConvexHull.h"
#include "gravity/narrowphase/Primitivetest.inl"
#include "gravity/narrowphase/triangletriangle.inl"
#include "gravity/narrowphase/CylinderTriangle.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/CylinderRay.h"
#include "gravity/narrowphase/CylinderSphere.h"
#include "gravity/narrowphase/Ray.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/GJK.inl"
#include "gravity/narrowphase/Primitive.inl"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>
#include "SphereSetting.h"
//#include "gravity/narrowphase/isa_gjk.inl"


void C_SectionPhysics::GJKCylinderCyliner()
{
	using namespace Maths;
	static Collision::C_Cylinder a;
	HVec3 start=HVec3(-8.f, 4.f, -2.f);
	HVec3 end=HVec3(-6.f, 3.f, 2.f);
	HVec3 u=end-start;
	C_Normal normal(u);
	const HF32 half(Maths::Constants<HF32>::Half());
	//HVec3 mid=u*half;
	a.m_Centre=(end+start)*half;
	a.m_halfHeight=u.Length()*half;
	a.m_Radius=2.f;
	a.m_CentreAxis=normal;
	//a[0]=a.start;

	static bool iniA=false;
	//if(!iniA)
	{
		a.wStart=a.m_Centre;
		//iniA = true;
	}
	
	
	
	
	//a.wStart=a.c;
	

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

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	start=start*matra;
	end=end*matra;
	u=u*matra;
	C_Normal n(u);
	a.m_CentreAxis=n;
	a.m_Centre=(end+start)*half;
	//a.wStart = matra * a.wStart;
	//a.wStart=a.c;


	

	static Collision::C_Cylinder b;
	HVec3 startb=HVec3(4.f, 6.f, -5.5f);
	HVec3 endb=HVec3(-3.f, -5.f, 1.f);
	HVec3 ub=endb-startb;
	C_Normal normalb(ub);
	//mid=ub*half;
	b.m_Centre=(endb+startb)*half;
	b.m_halfHeight=ub.Length()*half;
	b.m_Radius=3.f;
	b.m_CentreAxis=normalb;
	//a[0]=a.start;
	static bool iniB=false;
	//if(!iniB)
	{
		b.wStart=b.m_Centre;
	//	iniB = true;
	}
	
	//b.wStart=b.c;

	HF32 dis(Maths::UNDEFINED);
	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	F32 s;
	for(int i=0; i<1000; ++i)
	{
		s=Collision::GJKCPair<Collision::C_Cylinder, Collision::C_Cylinder>(a, b, dis, closestA, closestB);
		//HMat33 inv = matra.InverseRotation();
		//a.wStart = inv * a.wStart;
	}


	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a.m_Centre, 0.5f,  RGBA(0, 0, 255));

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(start+(end-start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, a.m_CentreAxis, a.m_Radius, RGBA(255, 0, 0));
	}

	if(s==0)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(startb, endb, RGBA(0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(b.m_Centre, 0.5f,  RGBA(255, 0, 0));
		for(int i=0; i<20; i++)
		{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(startb+(endb-startb)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, b.m_CentreAxis, b.m_Radius, RGBA(0, 0, 255));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.5f,  RGBA(255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.5f,  RGBA(0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, RGBA(0,255,0));
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(startb, endb, RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(b.m_Centre, 0.5f,  RGBA(255, 0, 0));
		for(int i=0; i<20; i++)
		{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(startb+(endb-startb)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, b.m_CentreAxis, b.m_Radius, RGBA(0, 255, 0));
		}
	}
}

void C_SectionPhysics::GJKMarginCylinderTriangle()
{

	using namespace Maths;
	static Collision::C_Cylinder a;
	HVec3 start=HVec3(-8.f, 4.f, -2.f);
	HVec3 end=HVec3(-6.f, 3.f, 2.f);
	HVec3 u=end-start;
	C_Normal normal(u);
	const HF32 half(Maths::Constants<HF32>::Half());
	//HVec3 mid=u*half;
	a.m_Centre=(end+start)*half;
	a.m_halfHeight=u.Length()*half;
	a.m_Radius=2.f;
	a.m_CentreAxis=normal;
	//a[0]=a.start;

	static bool iniA=false;
	//if(!iniA)
	{
		a.wStart=a.m_Centre;
		//iniA = true;
	}
	
	
	
	
	//a.wStart=a.c;
	

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
	

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	start=start*matra;
	end=end*matra;
	u=u*matra;
	C_Normal n(u);
	a.m_CentreAxis=n;
	a.m_Centre=(end+start)*half;
	a.Epsilon(0.01f);
	//a.wStart = matra * a.wStart;
	//a.wStart=a.c;

	HF32 dis(Maths::UNDEFINED);
	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	F32 s;

	Collision::C_Triangle tri;
	tri.m_VertexA=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
	tri.m_VertexB=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
	tri.m_VertexC=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));
	tri.WarmStart(tri.m_VertexA);
	Maths::HVec3 contact(Maths::UNDEFINED);
	Maths::HVec3 norm(Maths::UNDEFINED);
	Maths::HF32 penetration(Maths::UNDEFINED);
	Collision::EPA epa;
	//s =GJKCPairMargin<Cylinder, C_Triangle>(a, tri, contact, norm, penetration, epa);
	//s = Collision::GJKCPairMargin<Cylinder, C_Triangle>(a, tri);
	s= Collision::GJKCPair<Collision::C_Cylinder, Collision::C_Triangle>(a, tri, contact, norm, penetration, epa);
	

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a.m_Centre, 0.5f,  RGBA(0, 0, 255));

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(start+(end-start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, a.m_CentreAxis, a.m_Radius, RGBA(255, 0, 0));
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexB, RGBA(0, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexC, RGBA(0, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexB, tri.m_VertexC, RGBA(0, 255, 0));

	if(s)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexB, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexC, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexB, tri.m_VertexC, RGBA(0, 0, 0));

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.2f, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact + norm * penetration, RGBA(0, 0, 0));

	}

}

void C_SectionPhysics::TestCylinderTriangle()
{
	using namespace Maths;
	static Collision::C_Cylinder a;
	HVec3 start=HVec3(-8.f, 4.f, -2.f);
	HVec3 end=HVec3(-6.f, 3.f, 2.f);
	HVec3 u=end-start;
	C_Normal normal(u);
	const HF32 half(Maths::Constants<HF32>::Half());
	//HVec3 mid=u*half;
	a.m_Centre=(end+start)*half;
	a.m_halfHeight=u.Length()*half;
	a.m_Radius=2.f;
	a.m_CentreAxis=normal;
	//a[0]=a.start;

	static bool iniA=false;
	//if(!iniA)
	{
		a.wStart=a.m_Centre;
		//iniA = true;
	}
	
	
	
	
	//a.wStart=a.c;
	

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
	
	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	start=start*matra;
	end=end*matra;
	u=u*matra;
	C_Normal n(u);
	a.m_CentreAxis=n;
	a.m_Centre=(end+start)*half;
	a.Epsilon(0.01f);
	//a.wStart = matra * a.wStart;
	//a.wStart=a.c;

	HF32 dis(Maths::UNDEFINED);
	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	F32 s;

	Collision::C_Triangle tri;
	tri.m_VertexA=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
	tri.m_VertexB=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
	tri.m_VertexC=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));
	tri.WarmStart(tri.m_VertexA);
	Maths::HVec3 contact(Maths::UNDEFINED);
	Maths::HVec3 norm(Maths::UNDEFINED);
	Maths::HF32 penetration(Maths::UNDEFINED);

	//s =GJKCPairMargin<Cylinder, C_Triangle>(a, tri, contact, norm, penetration, epa);
	//s = Collision::GJKCPairMargin<Cylinder, C_Triangle>(a, tri);
	Collision::C_CollisionResult<4, 16> results;
	results.m_NumContactPatches = 1;
	const Maths::HVec3 ab(tri.m_VertexB - tri.m_VertexA);
	const Maths::HVec3 ac(tri.m_VertexC - tri.m_VertexA);
	results.m_ContactPatches[0].m_Normal = Maths::C_Normal(ab^ac);
	s=Collision::CollideCylinderTriangle<16>(a, tri.m_VertexA, tri.m_VertexB, tri.m_VertexC, results.m_ContactPatches[0], 0, 0, 0);
	

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a.m_Centre, 0.5f,  RGBA(0, 0, 255));

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(start+(end-start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, a.m_CentreAxis, a.m_Radius, RGBA(255, 0, 0));
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexB, RGBA(0, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexC, RGBA(0, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexB, tri.m_VertexC, RGBA(0, 255, 0));

	if(s)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexB, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexC, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexB, tri.m_VertexC, RGBA(0, 0, 0));

		for(U32 a = 0; a < results.m_NumContactPatches; ++a)
		{
			Collision::C_ContactPatch<16>& patch(results.m_ContactPatches[a]);
			for(U32 b = 0; b < patch.m_NumContactPoints; ++b)
			{
				Collision::C_ContactPoint& point(patch.m_ContactPoints[b]);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.2f, RGBA(0, 0, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point.m_ContactPoint, point.m_ContactPoint + patch.m_Normal * point.m_PenetrationDepth, RGBA(0, 0, 0));
			}
		}

	}

}

void C_SectionPhysics::TestCylinderRay()
{
	using namespace Maths;

	const HF32 half(Maths::Constants<HF32>::Half());
	Collision::C_CollisionFilter filter;
	Collision::C_CylinderShape cShape(&filter);
	Collision::C_Cylinder& a=cShape.m_Cylinder;
	a.m_Radius = 2.f;
	a.m_halfHeight = 2.f;
	HVec3 start=HVec3(-8.f, 4.f, -2.f);
	HVec3 end=HVec3(-6.f, 3.f, 2.f);
	HVec3 center=(start+end)*half;

	HVec3 u=end-start;
	a.m_halfHeight = u.Length() * half;	
	//a.wStart=a.c;
	

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
	
	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	cShape.Transform(Maths::HMat43(matra, center));

	start=start*matra;
	end=end*matra;
	//u=end-start;
	//C_Normal n(u);
	u=a.CentreAxis();//u*matra;
	C_Normal n(u);
	//a.m_CenterAxis=n;
	//a.m_Center=(end+start)*half;
	//a.Epsilon(0.01f);
	//a.wStart = matra * a.wStart;
	//a.wStart=a.c;

	Collision::C_Ray ray;
	ray.m_start=Maths::HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	ray.m_end=Maths::HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));


	HF32 dis(Maths::UNDEFINED);
	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	F32 s;

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(0, 255, 255));
	
	s=Collision::CollideCylinderRay(&cShape, ray);
	

	Maths::HVec3 heightVec(a.CentreAxis() * a.m_halfHeight);
	Maths::HVec3 capStart(a.Centre() - heightVec);
	Maths::HVec3 capEnd(a.Centre() + heightVec);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capStart, capEnd, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a.m_Centre, 0.5f,  RGBA(0, 0, 255));

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capStart+(capEnd-capStart)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, a.m_CentreAxis, a.m_Radius, RGBA(255, 0, 0));
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(0, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_start, 0.05f, RGBA(0, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(ray.m_start, "Start");


	if(s)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_end, 0.5f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(ray.m_end, ray.m_result.m_normal, 2.f, 0.5f, RGBA(0, 255, 0, 0));

	}

}

void C_SectionPhysics::GJKMarginCylinderCapsule()
{
	using namespace Maths;

	const HF32 half(Maths::Constants<HF32>::Half());
	
	Collision::C_Cylinder cylinder;
	cylinder.m_Radius = 2.f;
	cylinder.m_halfHeight = 2.f;
	HVec3 start=HVec3(-8.f, 4.f, -2.f);
	HVec3 end=HVec3(-6.f, 3.f, 2.f);
	HVec3 center=(start+end)*half;

	HVec3 u=end-start;
	cylinder.m_halfHeight = u.Length() * half;	
	//a.wStart=a.c;
	

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

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	start=start*matra;
	end=end*matra;
	
	u=u*matra;
	C_Normal n(u);
	
	cylinder.m_CentreAxis=n;
	cylinder.m_Centre=(end+start)*half;


	Collision::C_Capsule capsule;
	capsule.m_Start=HVec3(3.f, 4.f, -2.f);
	capsule.m_End=HVec3(-6.f, 3.f, 2.f);
	capsule.m_Radius=2.f;

	HVec3 mida((capsule.m_Start+capsule.m_End)*half);
	HVec3 na(capsule.m_End-capsule.m_Start);
	HVec3 contact(Maths::UNDEFINED);
	HVec3 normal(Maths::UNDEFINED);
	HF32 penetration(Maths::UNDEFINED);
	Collision::EPA epa;
	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);

	bool collided=Collision::GJKCPairMargin<Collision::C_Capsule, Collision::C_Cylinder>(capsule, cylinder, contact, normal, penetration, epa, &closestA, &closestB);
	

	//Draw capsule
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

	//Draw cylinder

	Maths::HVec3 heightVec(cylinder.CentreAxis() * cylinder.m_halfHeight);
	Maths::HVec3 cylStart(cylinder.Centre() - heightVec);
	Maths::HVec3 cylEnd(cylinder.Centre() + heightVec);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(cylStart, cylEnd, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(cylinder.m_Centre, 0.5f,  RGBA(0, 0, 255));

	//HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(cylStart+(cylEnd-cylStart)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, cylinder.m_CentreAxis, cylinder.m_Radius, RGBA(255, 0, 0));
	}


	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.5f, RGBA(0, 255, 0, 255));
	

		HF32 base(20.f);
		for(int i=0; i<20; i++)
		{
			HF32 ratio(((HF32)i)/base);
			HVec3 e(cylStart+(cylEnd-cylStart)*ratio);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, cylinder.m_CentreAxis, cylinder.m_Radius, RGBA(255, 255, 0,0));
		}

	}

}

void C_SectionPhysics::TestCylinderSphere()
{
	using namespace Maths;

	const HF32 half(Maths::Constants<HF32>::Half());
	
	Collision::C_Cylinder cylinder;
	cylinder.m_Radius = 2.f;
	cylinder.m_halfHeight = 2.f;
	HVec3 start=HVec3(-8.f, 4.f, -2.f);
	HVec3 end=HVec3(-6.f, 3.f, 2.f);
	HVec3 center=(start+end)*half;

	HVec3 u=end-start;
	cylinder.m_halfHeight = u.Length() * half;	
	//a.wStart=a.c;
	

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
	
	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	start=start*matra;
	end=end*matra;
	
	u=u*matra;
	C_Normal n(u);
	
	cylinder.m_CentreAxis=n;
	cylinder.m_Centre=(end+start)*half;

	Collision::C_Sphere sphere;
	sphere.Centre(HVec3(HF32(sX), HF32(sY), HF32(sZ)));
	sphere.Radius(Maths::HF32(r));

	//Draw sphere
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), Maths::RGBA(0, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(sphere.Centre(), 0.5f, Maths::RGBA(255, 0, 0));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(sphere.Centre(), cylinder.Centre(), Maths::RGBA(255, 0, 255));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(cylinder.Centre(), cylinder.CentreAxis(), cylinder.HalfHeight() , 0.25f,  Maths::RGBA(0, 0, 255));

	//Draw Cylinder

	Maths::HVec3 heightVec(cylinder.CentreAxis() * cylinder.m_halfHeight);
	Maths::HVec3 cylStart(cylinder.Centre() - heightVec);
	Maths::HVec3 cylEnd(cylinder.Centre() + heightVec);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(cylStart, cylEnd, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(cylinder.m_Centre, 0.5f,  RGBA(0, 0, 255));


	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(cylStart+(cylEnd-cylStart)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, cylinder.m_CentreAxis, cylinder.m_Radius, RGBA(255, 0, 0));
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(cylStart, cylinder.m_CentreAxis, cylinder.m_Radius, RGBA(255, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(cylEnd, cylinder.m_CentreAxis, cylinder.m_Radius, RGBA(255, 255, 0));

	HVec3 contact(Maths::UNDEFINED);
	HF32 pDepth(Maths::UNDEFINED);
	C_Normal normal(Maths::UNDEFINED);
	//bool collided = CollideCylinderSphere(cylinder, sphere);
	bool collided = Collision::CollideCylinderSphere(cylinder, sphere, contact, pDepth, normal);
	if(collided)
	{

		HF32 base(20.f);
		for(int i=0; i<20; i++)
		{
			HF32 ratio(((HF32)i)/base);
			HVec3 e(cylStart+(cylEnd-cylStart)*ratio);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, cylinder.m_CentreAxis, cylinder.m_Radius, RGBA(255, 255, 0,0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact + normal* pDepth, RGBA(0, 255, 255));
	}

}

