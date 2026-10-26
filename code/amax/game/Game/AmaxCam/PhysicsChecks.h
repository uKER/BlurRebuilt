#ifndef __PHYSICSCHECKS_H__
#define __PHYSICSCHECKS_H__

#include <camera2/camera.h>
#include <camera2/IPhysics.h>

#pragma warning (disable:4481)

namespace Gravity { class C_Scene; };

namespace AmaxCam
{
using namespace Maths;

class C_PhysicsChecks : public Cam2::I_Physics
{
public:
	C_PhysicsChecks(Gravity::C_Scene* physicsScene, S32 groupId);
	~C_PhysicsChecks();
	F32 RayTrace(const HVec3& start, const HVec3& end, F32 radius, const Collision::C_CollisionFilter* filter = NULL ) const OVERRIDE;
	F32 RayTraceNormal(const HVec3& start, const HVec3& end, F32 radius, HVec3* normal, bool meshOnly = false, const Collision::C_CollisionFilter* filter = NULL) const;
	S32 DeferredTrace(const HVec3& start, const HVec3& end, F32 radius) OVERRIDE;
	F32 DeferredResult(S32 rayIndex) OVERRIDE;
	Gravity::C_Scene* GetScene() { return m_PhysicsScene; }

private:
	Gravity::C_Scene* m_PhysicsScene;
	S32 m_GroupId;

};

} // End of namespace

#endif
