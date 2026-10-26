
#include "stdafx.h"
#include "PhysicsChecks.h"

#include <gravity/sdk/PhysicsSDK.h>
#include <Game/GameDef/Polyflags.h>
#include <system/debug/I_DebugRender.h>
#include "gravity/narrowphase/Ray.h"

static Debug::Bool ShowRaytraces("Camera2.Chase2.ShowRaytraces", false);

namespace MGCam
{

C_PhysicsChecks::C_PhysicsChecks(Gravity::C_Scene* physicsScene, S32 groupId)
{
	m_PhysicsScene = physicsScene;
	m_GroupId = groupId;
}

F32 C_PhysicsChecks::RayTrace(const Maths::HVec3& start, const Maths::HVec3& end, F32 radius) const
{
	F32 length = (end - start).Length();
	if (length < 1e-6f)
		return 1.f;

	Maths::HVec3 direction = (end - start) / length;

	F32 further = length;
	if (radius > 0.f)
		further *= 1.5f; // Ray trace 50% further to get an accurate capsule trace
	
	Collision::C_Ray ray;
	Collision::C_CollisionFilter filter(0xffffffff, Game::PolyFlags::kCameraSolid, m_GroupId, 0, 0);

	ray.InitialiseRay(start, Maths::C_Normal(direction), further, &filter, radius);
	
	m_PhysicsScene->DoRayTrace(ray, m_PhysicsScene->m_Stack);

	if (ShowRaytraces)
	{
		DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
		debugRender->Draw3dLine(start, end, Maths::RGBA(0xffffff00));
		debugRender->Draw3dSphere(ray.m_result.HitPoint(), 0.05f, Maths::RGBA(0xffff0000));
	}
	
	F32 frac = 1.f;
	if (ray.m_result.m_pShape)
	{
		frac = (F32)ray.m_result.m_distance / length;
		if (frac > 1.f)
			frac = 1.f;
	}

	return frac;
}

} // End of namespace
