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
#include "gravity/narrowphase/RayTriangle.h"
#include "gravity/narrowphase/RayTriangle.inl"
#include "gravity/narrowphase/TriangleTriangle.inl"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/SweptTest.h"
#include "gravity/treebuilder/aabbtree.h"
#include "gravity/treebuilder/generictree.h"
using namespace Baking;
using namespace Collision;

namespace Samples
{
void C_SectionPhysics::TestIntersectSegmentTriangle()
{
	using namespace Maths;

	HVec3 a=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
	HVec3 b=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
	HVec3 c=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));

	HVec3 q=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	HVec3 p=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));

	HVec3 res(Maths::UNDEFINED);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, c, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b, c, RGBA(255, 0, 0, 255));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(p, q, RGBA(0, 0, 255, 255));

	const Maths::HVec3 ab(b-a);
	const Maths::HVec3 ac(c-a);
	bool collided=Collision::IntersectTriangleSegment(a, b, c, p, q, res);

	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(res, 0.25f, RGBA(0, 255, 0, 255));
	}

}

void C_SectionPhysics::TestClosestPtSegmentTriangle()
{
	using namespace Maths;

	HVec3 a=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
	HVec3 b=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
	HVec3 c=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));

	HVec3 p=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	HVec3 q=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(a, b, c, RGBA(255, 0, 0, 255));
	

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(p, q, RGBA(0, 0, 255, 255));

	const Maths::HVec3 ab(b-a);
	const Maths::HVec3 ac(c-a);
	HVec3 closestA;
	HVec3 closestB;
	HF32 sqDist;
	Collision::ClosestPtSegmentTriangle(p, q, a, b, c, closestA, closestB, sqDist);

	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.25f, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.25f, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(closestA, "ClosestA");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(closestB, "ClosestB");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, RGBA(0, 0, 255, 255));
}


void C_SectionPhysics::TestTriangleTriangle()
{

	using namespace Maths;

	HVec3 a0=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
	HVec3 b0=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
	HVec3 c0=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));
	C_Normal n0((b0-a0)^(c0 - a0));

	HVec3 a1=HVec3(-9.5f, 10.f, 1.f);
	HVec3 b1=HVec3(3.5f, 8.f, -6.f);
	HVec3 c1=HVec3(-3.f, 4.f, -6.5f);
	C_Normal n1((b1-a1)^(c1-a1));
	//C_Normal n1((c1-a1)^(b1-a1));

	static HF32 rot=0.f;
	
	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}

	const Maths::RotY rotx(rot);
	Maths::HMat33 m(rotx);

	a1=m*a1;
	b1=m*b1;
	c1=m*c1;
	n1=m*n1;


	HF32 pen0(Maths::UNDEFINED);
	HF32 pen1(Maths::UNDEFINED);
	HVec3 contact0(UNDEFINED);
	HVec3 contact1(UNDEFINED);
	C_Normal resultNormal;
	Collision::C_CollisionResult<4, 16> res;
	C_ContactPatch<16> patch=res.m_ContactPatches[0];
	patch.m_Normal = n0;
	bool collide=false;
	
	//collide=Collision::OrenCollideTriangleTriangle(a0, b0, c0, a1, b1, c1, n0, n1, patch, 0, 0, 0);
	//collide = Collision::TomasCollideTriangleTriangle(a0, b0, c0, a1, b1, c1, n0, n1, resultNormal, contact0, contact1, pen0, pen1);
	for(U32 i=0; i<1000; ++i)
	{
		collide = Collision::TomasCollideTriangleTriangle(a0, b0, c0, a1, b1, c1, n0, n1, contact0, contact1, pen0, pen1);
	}
	

	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a0, b0, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a0, c0, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b0, c0, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(a0, "a0");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(b0, "b0");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(c0, "c0");

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(a1, "a1");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(b1, "b1");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(c1, "c1");

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(a0, n0, 1.f, 0.2f, Maths::RGBA(0, 255,0));


	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a1, b1, RGBA(255, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a1, c1, RGBA(255, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b1, c1, RGBA(255, 255, 0, 255));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(a1, n1, 1.f, 0.2f, Maths::RGBA(0, 255,0));


	if(collide)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a1, b1, RGBA(0, 0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a1, c1, RGBA(0, 0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b1, c1, RGBA(0, 0, 0, 255));

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact0, 0.25f, RGBA(0, 255, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact1, 0.25f, RGBA(0, 255, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(contact0, "0");
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(contact1, "1");

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contact0, n1, pen0, 0.2f, Maths::RGBA(0, 255,0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contact1, n1, pen1, 0.2f, Maths::RGBA(0, 255,0));

		//for(U32 i=0; i<patch.m_NumContactPoints; ++i)
		//{
		//	const Maths::HF32 pen(patch.m_ContactPoints[i].m_PenetrationDepth);
		//	const Maths::HVec3 point1(patch.m_ContactPoints[i].m_ContactPoint + patch.m_Normal*pen);
		//	//const Maths::HVec3 point2(patch.m_ContactPoints[i].m_ContactPoint + n1*pen);
		//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(patch.m_ContactPoints[i].m_ContactPoint, 0.5f, RGBA(0, 0, 255, 255));
		//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(patch.m_ContactPoints[i].m_ContactPoint, point1, RGBA(0, 0, 255, 255));
		//	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(patch.m_ContactPoints[i].m_ContactPoint, point2, RGBA(0, 255, 255, 255));
		//	
		//}
	}
}

};
