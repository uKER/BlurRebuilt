#include <stdafx.h>

#include "SectionPhysics.h"
#include "gravity/narrowphase/CapsuleCapsule.h"
#include "gravity/narrowphase/CapsuleSphere.h"
#include "gravity/narrowphase/CapsuleTriangle.h"
#include "gravity/narrowphase/ConeSphere.h"
#include "gravity/narrowphase/CapsuleRay.h"
#include "gravity/narrowphase/ConeRay.h"
#include "gravity/narrowphase/ConeTriangle.inl"

#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
//#include <Render/Shaders/World/psWorld.h>
//#include <Render/Shaders/World/vsWorld.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>

#include "CapsuleSetting.h"
#include "SegmentSetting.h"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/GJKCollision.h"
#include "TriangleSetting.h"
#include "SphereSetting.h"
#include "gravity/narrowphase/GJK.inl"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>


using namespace Baking;
using namespace Collision;

namespace Samples
{
	void C_SectionPhysics::GJKConeSphere()
	{
		using namespace Maths;

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
		

		C_Cone cone;
		const HVec3 position(1.f, 2.f, 3.f);
		const HVec3 offSet(0.f, 0.25f, 0.f);
		const HF32 r(0.5f);
		const C_Normal axis(HVec3(0.f, 1.f, 0.f), C_Normal::UNIT_LENGTH_VECTOR);
		
		const HF32 height(2.f);
		const HF32 margin(0.01f);
		cone.SetUpCone(position, offSet, axis, r, height);
		cone.m_MidPoint = cone.m_CenterOfMass + cone.m_OffSet * matra;
		cone.m_MidPointAxis = cone.m_LocalCenterAxis * matra;
		const Maths::HVec3 s = cone.m_MidPointAxis * cone.m_Height * Maths::Constants<Maths::HF32>::Half();
		const HVec3 start = cone.m_MidPoint + s;
		const HVec3 end = cone.m_MidPoint - s;
		cone.Margin(margin);

		Collision::C_Sphere sphere;
		const HVec3 c(1.f, 3.f, 4.f);
		const HF32 sr(1.f);
		const HF32 marginB(sr);
		sphere.Centre(c);
		sphere.Radius(sr);
		sphere.Margin(marginB);


		HVec3 contact(UNDEFINED);
		HVec3 normal(UNDEFINED);
		EPA epa;
		HF32 penetration(UNDEFINED);
		//bool collided =GJKSphereCone(sphere, cone, contact, normal, penetration, epa);
		bool collided = GJKCPairMargin<C_Cone, Collision::C_Sphere>(cone,sphere, contact, normal, penetration, epa);
		//bool collided = GJKCPairMargin<C_Cone, C_Triangle>(cone,triangle, contact, normal, penetration, epa);
	
		if(collided)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(0, 255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact -normal*penetration, RGBA(0, 0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(0, 255, 255, 0));
		}
		else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(255, 255, 255, 0));
		}
		//Draw cone
		const HF32 sCosTheta = Maths::Constants<Maths::HF32>::One() - cone.m_SinTheta * cone.m_SinTheta;
		const HF32 length = cone.m_Height;
		const C_Normal n(-cone.m_MidPointAxis);
		const HF32 gap = length/20;
		for(U32 i=0; i<20; ++i)
		{
			const HF32 dist = i*gap;
			const HF32 sDist = dist*dist;
			const HF32 ss = sDist/sCosTheta;
			const HF32 r =Maths::Sqrt(ss - sDist);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(start + n*dist, cone.m_MidPointAxis, r, Maths::RGBA(255, 0, 0, 0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 0));
		/*pRender->Draw3dLine(start, p1, Maths::RGBA(255, 0, 0, 0));
		pRender->Draw3dLine(start, p2, Maths::RGBA(255, 0, 0, 0));*/
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(end, cone.m_MidPointAxis, cone.m_BaseRadius, Maths::RGBA(255, 0, 0, 0));
	}

	void C_SectionPhysics::TestConeSphere()
	{
		using namespace Maths;

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
		

		C_Cone cone;
		const HVec3 position(1.f, 2.f, 3.f);
		const HVec3 offSet(0.f, 0.25f, 0.f);
		const HF32 r(0.5f);
		const C_Normal axis(HVec3(0.f, 1.f, 0.f), C_Normal::UNIT_LENGTH_VECTOR);
		
		const HF32 height(2.f);
		const HF32 margin(0.01f);
		cone.SetUpCone(position, offSet, axis, r, height);
		cone.m_MidPoint = cone.m_CenterOfMass + cone.m_OffSet * matra;
		cone.m_MidPointAxis = cone.m_LocalCenterAxis * matra;
		const Maths::HVec3 s = cone.m_MidPointAxis * cone.m_Height * Maths::Constants<Maths::HF32>::Half();
		const HVec3 start = cone.m_MidPoint + s;
		const HVec3 end = cone.m_MidPoint - s;
		cone.Margin(margin);

		Collision::C_Sphere sphere;
		sphere.Centre(HVec3(HF32(sX), HF32(sY), HF32(sZ)));
		sphere.Radius(Maths::HF32(r));

		HVec3 contact(Maths::UNDEFINED);
		HF32 pDepth(Maths::UNDEFINED);
		C_Normal normal(Maths::UNDEFINED);

		//bool collided = CollideConeSphere(cone,sphere);
		bool collided = Collision::CollideConeSphere(cone, sphere, contact, pDepth, normal);

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(sphere.Centre(), 0.25f, RGBA(0, 255,0));
	
		if(collided)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(0, 255, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contact - normal*pDepth, normal, pDepth, 0.25f, RGBA(0, 0, 255));
		}
		else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(255, 255, 255, 0));
		}
		//Draw cone
		const HF32 sCosTheta = Maths::Constants<Maths::HF32>::One() - cone.m_SinTheta * cone.m_SinTheta;
		const HF32 length = cone.m_Height;
		const C_Normal n(-cone.m_MidPointAxis);
		const HF32 gap = length/20;
		for(U32 i=0; i<20; ++i)
		{
			const HF32 dist = i*gap;
			const HF32 sDist = dist*dist;
			const HF32 ss = sDist/sCosTheta;
			const HF32 r =Maths::Sqrt(ss - sDist);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(start + n*dist, cone.m_MidPointAxis, r, Maths::RGBA(255, 0, 0, 0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 0));
		/*pRender->Draw3dLine(start, p1, Maths::RGBA(255, 0, 0, 0));
		pRender->Draw3dLine(start, p2, Maths::RGBA(255, 0, 0, 0));*/
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(end, cone.m_MidPointAxis, cone.m_BaseRadius, Maths::RGBA(255, 0, 0, 0));
	}



	void C_SectionPhysics::GJKConeCylinder()
	{
		using namespace Maths;

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
		

		C_Cone cone;
		const HVec3 position(1.f, 2.f, 3.f);
		const HVec3 offSet(0.f, 0.25f, 0.f);
		const HF32 r(0.5f);
		const C_Normal axis(HVec3(0.f, 1.f, 0.f), C_Normal::UNIT_LENGTH_VECTOR);
		const HF32 height(2.f);
		const HF32 margin(0.01f);
		cone.SetUpCone(position, offSet, axis, r, height);
		cone.m_MidPoint = cone.m_CenterOfMass + cone.m_OffSet * matra;
		cone.m_MidPointAxis = cone.m_LocalCenterAxis * matra;
		const Maths::HVec3 s = cone.m_MidPointAxis * cone.m_Height * Maths::Constants<Maths::HF32>::Half();
		const HVec3 coneStart = cone.m_MidPoint + s;
		const HVec3 coneEnd = cone.m_MidPoint - s;
		cone.Margin(margin);

		Collision::C_Cylinder a;
		HVec3 start=HVec3(-8.f, 4.f, -2.f);
		HVec3 end=HVec3(-6.f, 3.f, 2.f);
		HVec3 u=end-start;
		
		const HF32 half(Maths::Constants<HF32>::Half());
		//HVec3 mid=u*half;
		a.m_Centre=(end+start)*half;
		a.m_halfHeight=u.Length()*half;
		a.m_Radius=2.f;
		a.m_CentreAxis=C_Normal(u);


		HVec3 contact(UNDEFINED);
		C_Normal normal(UNDEFINED);
		EPA epa;
		HF32 penetration(UNDEFINED);
		bool collided =GJKCylinderCone(a, cone, contact, normal, penetration, epa);
	
		if(collided)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(0, 255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact + normal*penetration, RGBA(0, 0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a.m_Centre, 0.5f,  RGBA(0, 0, 255));

			HF32 base(20.f);
			for(int i=0; i<20; i++)
			{
				HF32 ratio(((HF32)i)/base);
				HVec3 e(start+(end-start)*ratio);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, a.m_CentreAxis, a.m_Radius, RGBA(255, 0, 0));
			}
		}
		else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, RGBA(0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a.m_Centre, 0.5f,  RGBA(0, 0, 255));

			HF32 base(20.f);
			for(int i=0; i<20; i++)
			{
				HF32 ratio(((HF32)i)/base);
				HVec3 e(start+(end-start)*ratio);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, a.m_CentreAxis, a.m_Radius, RGBA(0, 255, 0));
			}
		}
		//Draw cone
		const HF32 sCosTheta = Maths::Constants<Maths::HF32>::One() - cone.m_SinTheta * cone.m_SinTheta;
		const HF32 length = cone.m_Height;
		const C_Normal n(-cone.m_MidPointAxis);
		const HF32 gap = length/20;
		for(U32 i=0; i<20; ++i)
		{
			const HF32 dist = i*gap;
			const HF32 sDist = dist*dist;
			const HF32 ss = sDist/sCosTheta;
			const HF32 r =Maths::Sqrt(ss - sDist);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(start + n*dist, cone.m_MidPointAxis, r, Maths::RGBA(255, 0, 0, 0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(coneStart, coneEnd, Maths::RGBA(255, 0, 0, 0));
		/*pRender->Draw3dLine(start, p1, Maths::RGBA(255, 0, 0, 0));
		pRender->Draw3dLine(start, p2, Maths::RGBA(255, 0, 0, 0));*/
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(coneEnd, cone.m_MidPointAxis, cone.m_BaseRadius, Maths::RGBA(255, 0, 0, 0));
	}


	void C_SectionPhysics::GJKConeCapsule()
	{
		using namespace Maths;

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
		

		C_Cone cone;
		const HVec3 position(1.f, 2.f, 3.f);
		const HVec3 offSet(0.f, 0.25f, 0.f);
		const HF32 r(0.5f);
		const C_Normal axis(HVec3(0.f, 1.f, 0.f), C_Normal::UNIT_LENGTH_VECTOR);
	
		const HF32 height(2.f);
		const HF32 margin(0.01f);
		cone.SetUpCone(position, offSet, axis, r, height);
		cone.m_MidPoint = cone.m_CenterOfMass + cone.m_OffSet * matra;
		cone.m_MidPointAxis = cone.m_LocalCenterAxis * matra;
		const Maths::HVec3 s = cone.m_MidPointAxis * cone.m_Height * Maths::Constants<Maths::HF32>::Half();
		const HVec3 start = cone.m_MidPoint + s;
		const HVec3 end = cone.m_MidPoint - s;
		cone.Margin(margin);

		Collision::C_Capsule capsule;
		const HF32 marginB(0.001f);
		capsule.m_Start = HVec3(2.f, 5.f, 4.f);
		capsule.m_End = HVec3(2.f, 3.f, 4.f);
		capsule.m_Radius = HF32(1.f);
		capsule.Margin();
	
		HVec3 na(capsule.m_End-capsule.m_Start);

		HVec3 contact(UNDEFINED);
		C_Normal normal(UNDEFINED);
		EPA epa;
		HF32 penetration(UNDEFINED);
		bool collided =GJKCapsuleCone(capsule, cone, contact, normal, penetration, epa);
	
		if(collided)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(0, 255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact + normal*penetration, RGBA(0, 0, 255, 0));

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(255, 0, 0));

			HF32 base(20.f);
			for(int i=0; i<20; i++)
			{
				HF32 ratio(((HF32)i)/base);
				HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(255, 0, 0));
			}
		}
		else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(capsule.m_Start, capsule.m_End, RGBA(0, 255, 0, 0));

			HF32 base(20.f);
			for(int i=0; i<20; i++)
			{
				HF32 ratio(((HF32)i)/base);
				HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, capsule.m_Radius, RGBA(0, 255, 0, 0));
			}
			
		}
		//Draw cone
		const HF32 sCosTheta = Maths::Constants<Maths::HF32>::One() - cone.m_SinTheta * cone.m_SinTheta;
		const HF32 length = cone.m_Height;
		const C_Normal n(-cone.m_MidPointAxis);
		const HF32 gap = length/20;
		for(U32 i=0; i<20; ++i)
		{
			const HF32 dist = i*gap;
			const HF32 sDist = dist*dist;
			const HF32 ss = sDist/sCosTheta;
			const HF32 r =Maths::Sqrt(ss - sDist);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(start + n*dist, cone.m_MidPointAxis, r, Maths::RGBA(255, 0, 0, 0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 0));
		/*pRender->Draw3dLine(start, p1, Maths::RGBA(255, 0, 0, 0));
		pRender->Draw3dLine(start, p2, Maths::RGBA(255, 0, 0, 0));*/
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(end, cone.m_MidPointAxis, cone.m_BaseRadius, Maths::RGBA(255, 0, 0, 0));
	}

	void C_SectionPhysics::GJKConeTriangle()
	{
		using namespace Maths;

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
		

		C_Cone cone;
		const HVec3 position(0.f, 4.f, 3.f);
		const HVec3 offSet(0.f, 0.25f, 0.f);
		const HF32 r(0.5f);
		const C_Normal axis(HVec3(0.f, 1.f, 0.f), C_Normal::UNIT_LENGTH_VECTOR);
		const HF32 height(2.f);
		const HF32 margin(0.01f);
		cone.SetUpCone(position, offSet, axis, r, height);
		cone.m_MidPoint = cone.m_CenterOfMass + cone.m_OffSet * matra;
		cone.m_MidPointAxis = cone.m_LocalCenterAxis * matra;
		const Maths::HVec3 s = cone.m_MidPointAxis * cone.m_Height * Maths::Constants<Maths::HF32>::Half();
		const HVec3 start = cone.m_MidPoint + s;
		const HVec3 end = cone.m_MidPoint - s;
		cone.Margin(margin);

		Collision::C_Triangle triangle;
		triangle.m_VertexA = HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
		triangle.m_VertexB = HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
		triangle.m_VertexC = HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));

		const HVec3 ab = triangle.m_VertexB - triangle.m_VertexA;
		const HVec3 ac = triangle.m_VertexC - triangle.m_VertexA;
		const C_Normal nor(ab^ac);
		triangle.Margin(Constants<HF32>::Zero());


		HVec3 contact(UNDEFINED);
		HVec3 normal(UNDEFINED);
		EPA epa;
		HF32 penetration(UNDEFINED);
		bool collided = false;
		for(U32 i=0; i<500; ++i)
		{
			 collided=GJKCPairMargin<C_Cone, C_Triangle>(cone,triangle, contact, normal, penetration, epa);
		}
	
		if(collided)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(0, 255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact - normal*penetration, RGBA(0, 0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, RGBA(255, 0, 0, 0));
		}
		else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, RGBA(0, 255, 0, 0));
		}
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(triangle.m_VertexA, nor, 2.f, 0.2f, RGBA(255, 0, 0, 0));
		//Draw cone
		const HF32 sCosTheta = Maths::Constants<Maths::HF32>::One() - cone.m_SinTheta * cone.m_SinTheta;
		const HF32 length = cone.m_Height;
		const C_Normal n(-cone.m_MidPointAxis);
		const HF32 gap = length/20;
		for(U32 i=0; i<20; ++i)
		{
			const HF32 dist = i*gap;
			const HF32 sDist = dist*dist;
			const HF32 ss = sDist/sCosTheta;
			const HF32 r =Maths::Sqrt(ss - sDist);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(start + n*dist, cone.m_MidPointAxis, r, Maths::RGBA(255, 0, 0, 0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 0));
		/*pRender->Draw3dLine(start, p1, Maths::RGBA(255, 0, 0, 0));
		pRender->Draw3dLine(start, p2, Maths::RGBA(255, 0, 0, 0));*/
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(end, cone.m_MidPointAxis, cone.m_BaseRadius, Maths::RGBA(255, 0, 0, 0));
	}

	void C_SectionPhysics::TestConeTriangle()
	{
		using namespace Maths;

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
		

		C_Cone cone;
		const HVec3 position(0.f, 4.f, 3.f);
		const HVec3 offSet(0.f, 0.25f, 0.f);
		const HF32 r(0.5f);
		const C_Normal axis(HVec3(0.f, 1.f, 0.f), C_Normal::UNIT_LENGTH_VECTOR);
		const HF32 height(2.f);
		const HF32 margin(0.01f);
		cone.SetUpCone(position, offSet, axis, r, height);
		cone.m_MidPoint = cone.m_CenterOfMass + cone.m_OffSet * matra;
		cone.m_MidPointAxis = cone.m_LocalCenterAxis * matra;
		const Maths::HVec3 s = cone.m_MidPointAxis * cone.m_Height * Maths::Constants<Maths::HF32>::Half();
		const HVec3 start = cone.m_MidPoint + s;
		const HVec3 end = cone.m_MidPoint - s;
		cone.Margin(margin);

		Collision::C_Triangle triangle;
		triangle.m_VertexA = HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
		triangle.m_VertexB = HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
		triangle.m_VertexC = HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));

		const HVec3 ab = triangle.m_VertexB - triangle.m_VertexA;
		const HVec3 ac = triangle.m_VertexC - triangle.m_VertexA;
		const C_Normal nor(ab^ac);

		U32 MaxContacts = 4;
		C_ContactPatch<4> patch;
		bool collided = CollideConeTriangle<4>(triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, nor, cone, patch, 0, 0,  0, 6);

	
		if(collided)
		{
		//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(0, 255, 0, 0));
		//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact - normal*penetration, RGBA(0, 0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, RGBA(255, 0, 0, 0));
		}
		else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, RGBA(0, 255, 0, 0));
		}
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(triangle.m_VertexA, nor, 2.f, 0.2f, RGBA(255, 0, 0, 0));
		//Draw cone
		const HF32 sCosTheta = Maths::Constants<Maths::HF32>::One() - cone.m_SinTheta * cone.m_SinTheta;
		const HF32 length = cone.m_Height;
		const C_Normal n(-cone.m_MidPointAxis);
		const HF32 gap = length/20;
		for(U32 i=0; i<20; ++i)
		{
			const HF32 dist = i*gap;
			const HF32 sDist = dist*dist;
			const HF32 ss = sDist/sCosTheta;
			const HF32 r =Maths::Sqrt(ss - sDist);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(start + n*dist, cone.m_MidPointAxis, r, Maths::RGBA(255, 0, 0, 0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 0));
		/*pRender->Draw3dLine(start, p1, Maths::RGBA(255, 0, 0, 0));
		pRender->Draw3dLine(start, p2, Maths::RGBA(255, 0, 0, 0));*/
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(end, cone.m_MidPointAxis, cone.m_BaseRadius, Maths::RGBA(255, 0, 0, 0));

	}

	void C_SectionPhysics::GJKConeBox()
	{
		using namespace Maths;

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
		

		C_Cone cone;
		const HVec3 midPoint(1.f, 2.f, 3.f);
		const HVec3 offSet(0.f, 0.25f, 0.f);
		const HVec3 center = midPoint - offSet;
		const HF32 r(0.5f);
		const C_Normal localAxis(HVec3(0.f, 1.f, 0.f), C_Normal::UNIT_LENGTH_VECTOR);
		const HF32 height(2.f);
		const HF32 margin(0.01f);
		cone.SetUpCone(center, offSet, localAxis, r, height);
		cone.m_MidPoint = cone.m_CenterOfMass + cone.m_OffSet* matra;
		cone.m_MidPointAxis = cone.m_LocalCenterAxis*matra;
		const Maths::HVec3 s = cone.m_MidPointAxis * cone.m_Height * Maths::Constants<Maths::HF32>::Half();
		const HVec3 start = cone.m_MidPoint + s;
		const HVec3 end = cone.m_MidPoint - s;
		cone.Margin(margin);

		Collision::C_OBB box;
		box.c=HVec3(1.f, 3.f, -2.f);
		box.u=HMat33(UNDEFINED);
		box.u.SetIdentity();
		box.e=HVec3(2.f, 3.f, 4.f);

		HMat43 mat(box.u, box.c);
		HVec3 _min(-box.e);
		HVec3 _max(box.e);	


		HVec3 contact(UNDEFINED);
		C_Normal normal(UNDEFINED);
		EPA epa;
		HF32 penetration(UNDEFINED);
		bool collided =GJKBoxCone(box, cone, contact, normal, penetration, epa);
	
		if(collided)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.25f, RGBA(0, 255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact + normal*penetration, RGBA(0, 0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, mat, RGBA(255, 0, 0, 0));
		}
		else
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, mat, RGBA(0, 255, 0, 0));
		}
		//Draw cone
		const HF32 sCosTheta = Maths::Constants<Maths::HF32>::One() - cone.m_SinTheta * cone.m_SinTheta;
		const HF32 length = cone.m_Height;
		const C_Normal n(-cone.m_MidPointAxis);
		const HF32 gap = length/20;
		for(U32 i=0; i<20; ++i)
		{
			const HF32 dist = i*gap;
			const HF32 sDist = dist*dist;
			const HF32 ss = sDist/sCosTheta;
			const HF32 r =Maths::Sqrt(ss - sDist);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(start + n*dist, cone.m_MidPointAxis, r, Maths::RGBA(255, 0, 0, 0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 0));
		/*pRender->Draw3dLine(start, p1, Maths::RGBA(255, 0, 0, 0));
		pRender->Draw3dLine(start, p2, Maths::RGBA(255, 0, 0, 0));*/
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(end, cone.m_MidPointAxis, cone.m_BaseRadius, Maths::RGBA(255, 0, 0, 0));
	}

	
class C_ConeRayCollisionFilter : public C_CollisionFilter
{
public:

	C_ConeRayCollisionFilter() : C_CollisionFilter(0)
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

	void C_SectionPhysics::TestConeRay()
	{
		using namespace Maths;

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
		

		C_ConeRayCollisionFilter filter;
		C_ConeShape shape(&filter);
		C_Cone& cone = shape.m_Cone;
		const HVec3 midPoint(1.f, 2.f, 3.f);
		const HVec3 offSet(0.f, 0.25f, 0.f);
		const HVec3 center = midPoint - offSet;
		const HF32 r(0.5f);
		const C_Normal localAxis(HVec3(0.f, 1.f, 0.f), C_Normal::UNIT_LENGTH_VECTOR);
		const HF32 height(2.f);
		const HF32 margin(0.01f);
		cone.SetUpCone(center, offSet, localAxis, r, height);
		cone.m_MidPoint = cone.m_CenterOfMass + cone.m_OffSet* matra;
		cone.m_MidPointAxis = C_Normal(cone.m_LocalCenterAxis*matra);
		const Maths::HVec3 s = cone.m_MidPointAxis * cone.m_Height * Maths::Constants<Maths::HF32>::Half();
		const HVec3 start = cone.m_MidPoint + s;
		const HVec3 end = cone.m_MidPoint - s;
		cone.Margin(margin);

		Collision::C_Ray ray;
		const HVec3 rayStart=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
		const HVec3 rayEnd=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));
		ray.m_start = rayStart;
		ray.m_end = rayEnd;

		bool collided =CollideConeRay(&shape,ray);
		
		
		

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(rayStart, rayEnd, RGBA(0, 0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(rayStart, "start");
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(rayEnd, "end");
	
		if(collided)
		{
			Collision::C_RayTracingResult result = ray.m_result;
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(result.m_contact, 0.25f, RGBA(0, 255, 0, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(result.m_contact, result.m_normal, 2.f, 0.025f, RGBA(0, 255, 0, 0));
			
		}
		
		//Draw cone
		const HF32 sCosTheta = Maths::Constants<Maths::HF32>::One() - cone.m_SinTheta * cone.m_SinTheta;
		const HF32 length = cone.m_Height;
		const C_Normal n(-cone.m_MidPointAxis);
		const HF32 gap = length/20;
		for(U32 i=0; i<20; ++i)
		{
			const HF32 dist = i*gap;
			const HF32 sDist = dist*dist;
			const HF32 ss = sDist/sCosTheta;
			const HF32 r =Maths::Sqrt(ss - sDist);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(start + n*dist, cone.m_MidPointAxis, r, Maths::RGBA(255, 0, 0, 0));
		}

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(end, cone.m_MidPointAxis, cone.m_BaseRadius, Maths::RGBA(255, 0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(start, n, cone.m_Height, 0.025f, Maths::RGBA(255, 0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(end, 0.5f, Maths::RGBA(0, 255, 255, 0));


	}
}