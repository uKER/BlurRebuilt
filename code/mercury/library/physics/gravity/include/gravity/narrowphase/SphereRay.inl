#ifndef SPHERERAY_INL
#define SPHERERAY_INL

#include "gravity/narrowphase/shapes.h"

namespace Collision
{

NOALIAS inline bool CollideSphereRay(const C_Shape* const shape, C_Ray& ray)
{
	using namespace Maths;

	C_SphereShape* pSphereShape = (C_SphereShape*)shape;
	const HF32 zero(Constants<HF32>::Zero());
	C_Sphere& sphere=pSphereShape->m_Sphere;
	const HVec3 v(ray.m_end - ray.m_start);
	const HVec3 m(ray.m_start - sphere.Centre());
	const HF32 a(v|v);
	const HF32 b(m|v);
	const HF32 c((m|m)-sphere.Radius()*sphere.Radius());
	
	const HF32 discriminant(b*b - a*c);
	
	if(discriminant.LT(zero)) // not intersect
		return false;
	
	const HF32 sqrt(discriminant.Sqrt());
	const HF32 aRecip(a.Reciprocal());
	const HF32 t1((-b-sqrt)*aRecip);
	const HF32 t2((-b+sqrt)*aRecip);
	const Scalar_Bool con(t1.LT(zero));
	const HF32 t(Select(con, t2, t1));
	const HVec3 w(v*t);
	const HVec3 contact(ray.m_start + w);
	C_RayTracingResult& result=ray.m_result;
	result.m_contact=contact;
	result.m_normal=C_Normal(contact-sphere.Centre());
	result.m_TriangleNormal = result.m_normal;
	result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
	result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
	result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
	result.m_distance=w.Length();
	result.m_UserData = shape->m_UserData;
	ray.m_end=contact;
	ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();

	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(sphere.c, 0.025f, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.5f, RGBA(255, 0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(sphere.c, contact, RGBA(255, 255, 0, 255));*/
		
	return true;

}


NOALIAS inline bool CollideSphereRay(const C_Shape* const shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode)
{
	using namespace Maths;

	C_SphereShape* pSphereShape = (C_SphereShape*)shape;
	const HF32 zero(Constants<HF32>::Zero());
	C_Sphere& sphere=pSphereShape->m_Sphere;
	const HVec3 v(ray.m_end - ray.m_start);
	const HVec3 m(ray.m_start - sphere.Centre());
	const HF32 a(v|v);
	const HF32 b(m|v);
	const HF32 c((m|m)-sphere.Radius()*sphere.Radius());
	
	const HF32 discriminant(b*b - a*c);
	
	if(discriminant.LT(zero)) // not intersect
		return false;
	
	const HF32 sqrt(discriminant.Sqrt());
	const HF32 aRecip(a.Reciprocal());
	const HF32 t1((-b-sqrt)*aRecip);
	const HF32 t2((-b+sqrt)*aRecip);
	const Scalar_Bool con(t1.LT(zero));
	const HF32 t(Select(con, t2, t1));
	const HVec3 w(v*t);
	const HVec3 contact(ray.m_start + w);
	C_RayTracingResult result;
	result.m_contact=contact;
	result.m_normal=C_Normal(Normalise(contact-sphere.Centre()));
	result.m_TriangleNormal = result.m_normal;
	result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
	result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
	result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
	result.m_distance=w.Length();
	result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
	result.m_UserData = shape->m_UserData;
	result.m_pNode = pGraphNode;

	ray.StoreResult(result);

	return true;

}

};

#endif

