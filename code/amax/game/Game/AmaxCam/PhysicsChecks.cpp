
#include "stdafx.h"
#include "PhysicsChecks.h"

#include <gravity/sdk/Scene.h>
#include <gravity/narrowphase/Ray.h>
#include <system/debug/I_DebugRender.h>

#include "Game/GameDef/Polyflags.h"
#include "RayGroup.h"
#include "SchedulerConfig.h"
namespace AmaxCam
{

using namespace Maths;

static Debug::Bool ShowRaytraces("Camera2.Chase2.ShowRaytraces", false);

C_PhysicsChecks::C_PhysicsChecks(Gravity::C_Scene* physicsScene, S32 groupId)
{
	m_PhysicsScene = physicsScene;
	m_GroupId = groupId;
}

C_PhysicsChecks::~C_PhysicsChecks()
{
}

F32 C_PhysicsChecks::RayTrace(const HVec3& start, const HVec3& end, F32 radius, const Collision::C_CollisionFilter* filter) const
{
	return RayTraceNormal(start, end, radius, 0, false, filter);
}

F32 C_PhysicsChecks::RayTraceNormal(const HVec3& start, const HVec3& end, F32 radius, HVec3* normal, bool meshOnly, const Collision::C_CollisionFilter* filter) const
{
	F32 length = (end - start).Length();
	if (length < 1e-6f)
		return 1.f;

	HVec3 direction = (end - start) / length;

	F32 further = length ;
	if (radius > 0.f)
		further *= 1.5f; // Ray trace 50% further to get an accurate capsule trace
	
	Collision::C_Ray ray;
	Collision::C_CollisionFilter defaultCollisionFilter(0xffffffff, Game::PolyFlags::kCameraSolid , m_GroupId, 0, 0);

	defaultCollisionFilter.SetBodyMask(defaultCollisionFilter.GetBodyMask() & (~Game::PolyFlags::kbPerk));

	ray.InitialiseRay(start, C_Normal(direction), further, filter?filter:&defaultCollisionFilter, radius);
	U8 data[1024 * sizeof(void*)];
	SharedPhysics::C_SafeStack<true> stack(data, 1024);

	if (meshOnly)
		m_PhysicsScene->DoRayTraceMesh(ray, stack);
	else
		m_PhysicsScene->DoRayTrace(ray, stack);
	

	if (ShowRaytraces)
	{
		DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
		debugRender->Draw3dLine(start, start + direction * further, RGBA(0xffffff00));

		if (ray.m_result.m_pShape)
		{
			debugRender->Draw3dSphere(ray.m_result.HitPoint(), 0.05f, RGBA(0xff40c0ff));
			
			Util::C_String text;
			text.Printf("type=%d", ray.m_result.m_pShape->m_ShapeType);
			debugRender->Draw3dText(ray.m_result.HitPoint(), text);
		}
	}

	F32 frac = 1.f;
	if (ray.m_result.m_pShape)
	{
		frac = (F32)ray.m_result.m_distance / length;
		if (frac > 1.f)
			frac = 1.f;

		if (normal)
			*normal = ray.m_result.Normal();
	}
		
	return frac;
}

S32 C_PhysicsChecks::DeferredTrace(const HVec3& start, const HVec3& end, F32 radius)
{
	F32 length = (end - start).Length();
	if (length < 1e-6f)
		return -1;

	HVec3 direction = (end - start) / length;

	S32 rayIndex = g_RayGroup.NextRay();
	
	Collision::C_Ray* ray = g_RayGroup.GetRay(rayIndex);
	if (!ray)
		return -1;

	Collision::C_CollisionFilter filter(0xffffffff, Game::PolyFlags::kCameraSolid, m_GroupId, 0, 0);
	ray->InitialiseRay(start, C_Normal(direction), length, &filter, radius);
	
	S32 group = g_RayGroup.SchedulerGroup();
	S32 policy = Game::C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy();
	m_PhysicsScene->DoDeferredRayTrace(ray, 0, group, policy);
	
	return rayIndex;
}

F32 C_PhysicsChecks::DeferredResult(S32 rayIndex)
{
	Collision::C_Ray* ray = g_RayGroup.GetRay(rayIndex);
	if (!ray)
		return 0.f;
	
	F32 frac = 1.f;
	if (ray->m_result.m_pShape)
	{
		frac = ray->m_result.m_distance / ray->m_maxDistance;
		if (frac > 1.f)
			frac = 1.f;
	}

	return frac;
}

} // End of namespace
