
#ifndef CAMERA2_IPHYSICS_H
#define CAMERA2_IPHYSICS_H

// Forward declarations
namespace Collision { class C_Ray; };
namespace Collision { class C_CollisionFilter; }

namespace Cam2
{

class I_Physics
{
public:
	virtual F32 RayTrace(const Maths::HVec3& start, const Maths::HVec3& end, F32 radius = 0.f, const Collision::C_CollisionFilter* filter = NULL) const = 0;
	virtual S32 DeferredTrace(const Maths::HVec3&, const Maths::HVec3&, F32) { return -1; }
	virtual F32 DeferredResult(S32) { return 0.f; }
};

} // End of namespace Cam2

#endif
