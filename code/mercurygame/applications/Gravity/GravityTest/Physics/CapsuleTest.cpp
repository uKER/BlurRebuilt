#include <stdafx.h>

#include "SectionPhysics.h"

#include "gravity/narrowphase/CapsuleCapsule.h"
#include "gravity/narrowphase/CapsuleSphere.h"
#include "gravity/narrowphase/CapsuleTriangle.h"
#include "gravity/narrowphase/CapsuleRay.h"


#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>

#include "CapsuleSetting.h"
#include "SegmentSetting.h"
#include "SphereSetting.h"
#include "gravity/narrowphase/primitive.inl"


using namespace Baking;
using namespace Collision;

void C_SectionPhysics::TestCollideCapsuleCapsule()
{
	using namespace Maths;

	const HF32 half(Maths::Constants<HF32>::Half());
	Collision::C_Capsule a;
	a.m_Start=HVec3(3.f, 4.f, -2.f);
	a.m_End=HVec3(-6.f, 3.f, 2.f);
	a.m_Radius=2.f;

	HVec3 mida((a.m_Start+a.m_End)*half);
	HVec3 na(a.m_End-a.m_Start);
	Collision::C_Capsule b;
	b.m_Start=HVec3(4.f, 6.f, -5.5f);
	b.m_End=HVec3(-3.f, -5.f, 1.f);
	b.m_Radius=3.f;
	HVec3 midb((b.m_Start+b.m_End)*half);
	HVec3 nb(b.m_End-b.m_Start);

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


	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	a.m_Start=a.m_Start*matra;
	a.m_End=a.m_End*matra;
	mida=mida*matra;
	
	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	HVec3 contact(Maths::UNDEFINED);
	C_Normal normal(Maths::UNDEFINED);
	HF32 pDepth(Maths::UNDEFINED);

	bool collided;

	const int MaxPatches=2;
	const int MaxContacts=2;
	C_CollisionResult<MaxPatches, MaxContacts> results;

	for(int i=0; i<1000; ++i)
	{
		collided=Collision::CollideCapsuleCapsule(a, b, results.m_ContactPatches[0], 0, 0);
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a.m_Start, a.m_End, RGBA(255, 0, 0));


	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(a.m_Start+(a.m_End-a.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, a.m_Radius, RGBA(255, 0, 0));
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b.m_Start, b.m_End, RGBA(0, 0, 255));
	

	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(b.m_Start+(b.m_End-b.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, nb, b.m_Radius, RGBA(0, 0, 255));
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(b.m_Start, nb, b.m_Radius, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(b.m_End, nb, b.m_Radius, RGBA(0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(midb, nb, b.m_Radius, RGBA(0, 0, 255));

	C_ContactPatch<MaxContacts>& patch=results.m_ContactPatches[0];
	C_Normal n=patch.m_Normal;
	
	C_ContactPoint& point=patch.m_ContactPoints[0];
	Maths::HF32 penetrationDepth=point.m_PenetrationDepth;
	Maths::HVec3 end=point.m_ContactPoint-n*penetrationDepth;

	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.5f,  RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(point.m_ContactPoint, end,  RGBA(0, 255, 0));
	}

}

void C_SectionPhysics::TestCapsuleSphere()
{
	using namespace Maths;
	Collision::C_Capsule capsule;
	capsule.m_Start=HVec3(3.f, 4.f, -2.f);
	capsule.m_End=HVec3(-6.f, 3.f, 2.f);
	capsule.m_Radius=2.f;

	HVec3 na=capsule.m_End-capsule.m_Start;

	Collision::C_Sphere sphere;
	sphere.Centre(HVec3(2.f, 3.f, -7.f));
	sphere.Radius(Maths::HF32(3.f));
	

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

	sphere.Centre(sphere.Centre()*matra);
	

	F32 s;
	for(int i=0; i<1000; ++i)
	{
		s=IntersectCapsuleSphere(capsule, sphere);
	}

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 0));
	}
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(255, 0, 0));
	

	if(s)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(255, 0, 0));
	} 
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(0, 255, 0));
	}


}


void C_SectionPhysics::TestCapsuleTriangle()
{
	using namespace Maths;
	Collision::C_Capsule capsule;
	capsule.m_Start=HVec3(3.f, 4.f, -2.f);
	capsule.m_End=HVec3(-6.f, 3.f, 2.f);
	capsule.m_Radius=2.f;

	HVec3 na=capsule.m_End-capsule.m_Start;

	HVec3 a(4.f,6.f, -7.f);
	HVec3 b(8.f, 2.f, 4.f);
	HVec3 c(-3.f, 4.f, -8.f );

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

	a=a*matra;
	b=b*matra;
	c=c*matra;
	

	F32 s;
	for(int i=0; i<1000; ++i)
	{
		s=IntersectCapsuleTriangle(capsule, a, b, c);
	}

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 0));
	}
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(255, 0, 0));
	

	if(s)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, c, RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, c, b, RGBA(0, 255, 0));
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, c, RGBA(255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, c, b, RGBA(255, 0, 0));

	}

}

void C_SectionPhysics::TestCollidedCapsuleSphere()
{
	using namespace Maths;

	Collision::C_Capsule capsule;
	capsule.m_Start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	capsule.m_End=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));
	capsule.m_Radius=2.f;

	HVec3 na=capsule.m_End-capsule.m_Start;

	Collision::C_Sphere sphere;
	sphere.Centre(HVec3(HF32(sX), HF32(sY), HF32(sZ)));
	sphere.Radius(HF32(r));
	

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

	sphere.Centre(sphere.Centre()*matra);

	HVec3 contact(Maths::UNDEFINED);
	C_Normal normal(Maths::UNDEFINED);
	HF32 penetrationDepth(Maths::UNDEFINED);
	

	
	const int MaxPatches=2;
	const int MaxContacts=2;
	C_CollisionResult<MaxPatches, MaxContacts> results;

	bool collided=false;

	collided=Collision::CollideCapsuleSphere(capsule, sphere, contact, penetrationDepth, normal);
	

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 0));
	}
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(255, 0, 0));
	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(255, 255, 0, 255));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(255, 0, 0));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(sphere.Centre(), 0.025f, RGBA(255, 0, 255));

	//C_ContactPatch<MaxContacts>& patch=results.m_ContactPatches[0];
	//C_Normal n=patch.m_Normal;
	//
	//C_ContactPoint& point=patch.m_ContactPoints[0];
	//Maths::HF32 penetrationDepth=point.m_PenetrationDepth;
	Maths::HVec3 end=contact-normal*penetrationDepth;

	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.5f,  RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, end,  RGBA(0, 255, 0));
	}

}

void C_SectionPhysics::TestCollidedCapsuleTriangle()
{
	using namespace Maths;

	Collision::C_Capsule capsule;
	capsule.m_Start=HVec3(HF32(capsuleSX), HF32(capsuleSY), HF32(capsuleSZ));
	capsule.m_End=HVec3(HF32(capsuleEX), HF32(capsuleEY), HF32(capsuleEZ));
	capsule.m_Radius=HF32(capsuleRadius);

	HVec3 na=capsule.m_End-capsule.m_Start;

	HVec3 b(4.f, 6.f, -9.f);
	HVec3 a(8.f, 2.f, 6.f);
	HVec3 c(-4.f, 4.f, -10.f );

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

	a=a*matra;
	b=b*matra;
	c=c*matra;
	

	HVec3 contact(Maths::UNDEFINED);
	C_Normal normal(Maths::UNDEFINED);
	HF32 pDepth(Maths::UNDEFINED);

	const int MaxPatches=2;
	const int MaxContacts=3;

	C_ContactPatch<MaxContacts> patch;
	patch.m_NumContactPoints = 0;
	bool collided=CollideCapsuleTriangle<3>(capsule, a, b, c, patch);
	

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 0));
	}
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(255, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(capsule.m_Start, "start");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(capsule.m_End, "end");

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(a, b,c, RGBA(0, 255, 0));

	
	

	
	if(collided)
	{
		for(U32 i=0; i<patch.m_NumContactPoints; ++i)
		{
			Collision::C_ContactPoint contact = patch.m_ContactPoints[i];
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact.m_ContactPoint, 0.5f,  RGBA(0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact.m_ContactPoint, contact.m_ContactPoint + patch.m_Normal * contact.m_PenetrationDepth, Maths::RGBA(255, 255, 255));
		}
		
	}
	
}

class C_CapsuleRayCollisionFilter : public C_CollisionFilter
{
public:

	C_CapsuleRayCollisionFilter() : C_CollisionFilter(0)
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


void C_SectionPhysics::TestCapsuleRay()
{
	using namespace Maths;

	C_CapsuleRayCollisionFilter filter;
	C_CapsuleShape shape(&filter);
	shape.m_Capsule.m_Start=HVec3(HF32(capsuleSX), HF32(capsuleSY), HF32(capsuleSZ));
	shape.m_Capsule.m_End=HVec3(HF32(capsuleEX), HF32(capsuleEY), HF32(capsuleEZ));
	shape.m_Capsule.m_Radius=3.f;

	Collision::C_Capsule& capsule=shape.m_Capsule;

	C_Ray ray;
	ray.m_start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	ray.m_end=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));

	bool collide;
	for(U32 i=0; i<1000; ++i)
	{
		collide=CollideCapsuleRay(&shape, ray);
	}

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, capsule.m_End-capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 0));
	}
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.m_Radius, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.m_Radius, RGBA(255, 0, 0));

	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_start, 0.25f, RGBA(0, 0, 0, 255));
	if(collide)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(255, 255, 0, 255));
	}




}

