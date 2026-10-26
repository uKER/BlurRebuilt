#ifndef __PHYSICS_CHECKSS_H__
#define __PHYSICS_CHECKSS_H__

#include <camera2/IPhysics.h>

namespace Gravity { class C_Scene; };

namespace MGCam
{

class C_PhysicsChecks : public Cam2::I_Physics
{
public:
	C_PhysicsChecks(Gravity::C_Scene* physicsScene, S32 groupId);
	virtual F32 RayTrace(const Maths::HVec3& start, const Maths::HVec3& end, F32 radius) const OVERRIDE;
	
private:
	Gravity::C_Scene* m_PhysicsScene;
	S32 m_GroupId;
};

} // End of namespace

#endif
