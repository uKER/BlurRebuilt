#include <stdafx.h>

#include "SectionPhysics.h"
#include "gravity/narrowphase/PrimitiveTest.h"
//#include "gravity/narrowphase/PrimitiveTest.inl"

#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include "TriangleSetting.h"
#include "SegmentSetting.h"
#include "OBBSetting.h"
#include "CapsuleSetting.h"
#include "gravity/narrowphase/RayTriangle.h"
#include "gravity/narrowphase/RayTriangle.inl"
#include "gravity/narrowphase/TriangleTriangle.inl"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/SweptTest.h"
using namespace Baking;
using namespace Collision;

namespace Samples
{
void C_SectionPhysics::TestSweptRayTriangle()
{
	using namespace Maths;

	HVec3 a=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
	HVec3 b=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
	HVec3 c=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));

	HVec3 p=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	HVec3 q=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(a, b, c, RGBA(255, 0, 0, 255));
	

	const Maths::HF32 radius(3.f);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(p, q, RGBA(0, 0, 255, 255));

	const Maths::HVec3 ab(b-a);
	const Maths::HVec3 ac(c-a);
	const C_Normal normal(ab^ac);

	Maths::HVec3 contact;
	Maths::C_Normal conNormal;
	Maths::HF32 dis;
	bool collide=Collision::SweptSphereTriangleTest(p, q, radius, a, b, c, normal, contact, conNormal, dis);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(p, radius, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(a, normal, 0.5f, 0.2f, RGBA(0, 255, 0, 255));

	if(collide)
	{
	
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(0, 255, 0, 255));
		const Maths::HVec3 center(contact + conNormal*radius);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(center, 0.25f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(center, radius, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contact, conNormal, dis, 0.2f, RGBA(0, 255, 0, 255));
	}
}


void C_SectionPhysics::TestSweptSphereSphere()
{
	using namespace Maths;

	const Maths::HVec3 center(2.f, 4.f, 5.f);
	const Maths::HF32 r(3.f);
	Collision::C_Sphere sphere(center, r);

	Maths::HVec3 start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	Maths::HVec3 end=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));

	const Maths::HF32 radius(2.f);

	Maths::HVec3 contact;
	Maths::C_Normal conNormal;
	Maths::HF32 dis;
	bool collide=Collision::SweptSphereSphereTest(start, end, radius, sphere, contact, conNormal, dis);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(center, r, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(start, radius, RGBA(0, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(center, 0.2f, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(start, 0.2f, RGBA(0,0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(0,255, 255, 255));

	if(collide)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(255, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(contact + conNormal*radius, radius, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact + conNormal*radius, 0.2f, RGBA(0, 0, 255, 255));
		
	}
}

void C_SectionPhysics::TestSweptSphereOBB()
{
	using namespace Maths;
	//Set up sphere(center=start, radius=radius)
	Maths::HVec3 start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	Maths::HVec3 end=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));
	const Maths::HF32 radius(2.f);

	//Set up obb
	C_OBB obb;
	obb.c=HVec3(HF32(obb_cX), HF32(obb_cY), HF32(obb_cZ));
	obb.u=HMat33(UNDEFINED);
	obb.u.SetIdentity();
	obb.e=HVec3(HF32(obb_eX), HF32(obb_eY), HF32(obb_eZ));

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.2f;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.2f;
	}

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	obb.u=matra;


	Maths::HVec3 contact;
	Maths::C_Normal conNormal;
	Maths::HF32 dis;
	bool collided=Collision::SweptSphereOBBTest(start, end, radius, obb, contact, conNormal, dis);

	Collision::C_OBB box(obb);
	const Maths::HVec3 r(radius, radius, radius);
	box.e+=r;

	const Maths::HMat43 mat(obb.u, obb.c);
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-box.e, box.e, mat, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-obb.e, obb.e, mat, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(start, radius, RGBA(255, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(255, 0, 0, 255));

	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-obb.e, obb.e, mat, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.2f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(contact + conNormal*radius, radius, RGBA(255, 0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact + conNormal*radius, 0.2f, RGBA(0, 0, 255, 255));
	}

}
	
void C_SectionPhysics::TestSweptSphereCapsule()
{
	using namespace Maths;
	//Set up sphere(center=start, radius=radius)
	const Maths::HVec3 start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	const Maths::HVec3 end=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));
	
	const Maths::HF32 radius(2.f);

	//Set up capsule
	Collision::C_Capsule capsule;
	capsule.m_Start=HVec3(HF32(capsuleSX), HF32(capsuleSY), HF32(capsuleSZ));
	capsule.m_End=HVec3(HF32(capsuleEX), HF32(capsuleEY), HF32(capsuleEZ));
	capsule.m_Radius=3.f;
	const Maths::HVec3 na(capsule.m_End-capsule.m_Start);

	Maths::HVec3 contact;
	Maths::C_Normal conNormal;
	Maths::HF32 dis;
	bool collided=Collision::SweptSphereCapsuleTest(start, end, radius, capsule, contact, conNormal, dis);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(start, radius, RGBA(255, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(255, 0, 0, 255));


	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.Radius(), RGBA(255, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(255, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.Radius(), RGBA(255, 255, 0, 255));

	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 255, 0));
	}

	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, capsule.Radius(), RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, capsule.Radius(), RGBA(0, 255, 0, 255));

		HF32 base(20.f);
		for(int i=0; i<20; i++)
		{
			HF32 ratio(((HF32)i)/base);
			HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(0, 255, 0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.2f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(contact + conNormal*radius, radius, RGBA(255, 0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact + conNormal*radius, 0.2f, RGBA(0, 0, 255, 255));
	}

}

void C_SectionPhysics::TestSweptSphereCylinder()
{
	using namespace Maths;
	//Set up sphere(center=start, radius=radius)
	const Maths::HVec3 start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	const Maths::HVec3 end=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));
	
	const Maths::HF32 radius(2.f);


	static Collision::C_Cylinder cylinder;
	HVec3 s=HVec3(-8.f, 4.f, -2.f);
	HVec3 e=HVec3(-6.f, 3.f, 2.f);
	HVec3 u=e-s;
	C_Normal normal(u);
	const HF32 half(Maths::Constants<HF32>::Half());
	//HVec3 mid=u*half;
	cylinder.m_Centre=(e+s)*half;
	cylinder.m_halfHeight=u.Length()*half;
	cylinder.m_Radius=2.f;
	cylinder.m_CentreAxis=normal;

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

	s=s*matra;
	e=e*matra;
	//u=end-start;
	//C_Normal n(u);
	u=u*matra;
	C_Normal n(u);
	cylinder.m_CentreAxis=Maths::C_Normal(e - s);
	cylinder.m_Centre=(e+s)*half;
	cylinder.Epsilon(0.01f);
	//a.wStart = matra * a.wStart;
	//a.wStart=a.c;


	
	Maths::HVec3 contact(Maths::UNDEFINED);
	Maths::C_Normal conNormal(Maths::UNDEFINED);
	Maths::HF32 dis(Maths::UNDEFINED);

	
	bool collided=Collision::SweptSphereCylinderTest(start, end, radius, cylinder, contact, conNormal, dis);
	

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(start, radius, RGBA(255, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(255, 0, 0, 255));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(0, 0, 255));
//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a.m_Center, 0.5f,  RGBA(0, 0, 255));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(s, 0.2f, RGBA(255, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(e, 0.2f, RGBA(255, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(s, "os");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(e, "oe");
	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(s+(e-s)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, cylinder.m_CentreAxis, cylinder.m_Radius, RGBA(255, 0, 0));
	}


	if(collided)
	{
		HF32 base(20.f);
		for(int i=0; i<20; i++)
		{
			HF32 ratio(((HF32)i)/base);
			HVec3 e(s+(e-s)*ratio);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, cylinder.m_CentreAxis, cylinder.m_Radius, RGBA(0, 255, 0));
		}
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.2f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(contact + conNormal*radius, radius, RGBA(255, 0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact + conNormal*radius, 0.2f, RGBA(0, 0, 255, 255));

	}

}



};
