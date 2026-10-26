#include <stdafx.h>

#include "SectionPhysics.h"
#include "gravity/narrowphase/PrimitiveTest.h"
//#include "gravity/narrowphase/PrimitiveTest.inl"
#include "gravity/narrowphase/SphereTriangle.h"
#include "gravity/narrowphase/SphereTriangle.inl"
#include "gravity/narrowphase/Ray.h"
#include "gravity/narrowphase/SphereRay.h"
#include "gravity/narrowphase/SphereRay.inl"

#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include "SphereSetting.h"
#include "SegmentSetting.h"
#include "gravity/sharedphysics/AllocationManager.inl"


using namespace Baking;
using namespace Collision;

namespace Samples
{



void C_SectionPhysics::TestCollidedShpereTriangle()
{
	
	using namespace Maths;

	Collision::C_Sphere sphere;
	sphere.Centre(HVec3(HF32(sX), HF32(sY), HF32(sZ)));
	sphere.Radius(Maths::HF32(r));

	const Maths::HVec3 a(-2.f, 5.f, 2.f);
	const Maths::HVec3 b(2.f, 5.f, -2.f);
	const Maths::HVec3 c(1.f, 3.f, 6.f);

	const int MaxPatches =2;
	const int MaxContacts=2;
	Collision::C_CollisionResult<MaxPatches, MaxContacts> result;
	
	bool collided=Collision::CollideSphereTriangle(sphere, a, b, c, result);

	C_ContactPatch<MaxContacts>& patch=result.m_ContactPatches[0];
	C_Normal normal=patch.m_Normal;
	C_ContactPoint& point=patch.m_ContactPoints[0];
	Maths::HVec3 contact(point.m_ContactPoint);
	Maths::HF32 penetration(point.m_PenetrationDepth);


	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(sphere.Centre(), 0.5f, RGBA(0, 255, 0, 255));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, RGBA(0, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, c, RGBA(0, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b, c, RGBA(0, 0, 255, 255));




	if(collided)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.5f, RGBA(255, 255, 0, 255));
		Maths::HVec3 end(contact-normal*penetration);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, end, RGBA(255, 255, 255, 255));
	}
}

class C_MyCollisionFilter : public C_CollisionFilter
{
public:

	C_MyCollisionFilter() : C_CollisionFilter(0)
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

void C_SectionPhysics::TestSphereRay()
{

	using namespace Maths;

	C_Ray ray;
	ray.m_start=HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	ray.m_end=HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));

	C_MyCollisionFilter filter;
	C_SphereShape shape(&filter);
	shape.m_Sphere.Centre(HVec3(HF32(sX), HF32(sY), HF32(sZ)));
	shape.m_Sphere.Radius(Maths::HF32(r));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(shape.m_Sphere.Centre(), r, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(0, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPoint(ray.m_start, 0.5f, RGBA(0, 0, 0, 255));

	bool collide;
	for(U32 i=0; i<1000; ++i)
	{
		collide=CollideSphereRay(&shape, ray);
	}

	if(collide)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(0, 255, 255, 255));
	}

}


};
