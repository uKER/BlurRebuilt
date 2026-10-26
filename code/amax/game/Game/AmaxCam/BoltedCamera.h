#pragma once	// [Blur2PC] header had no guard; the unity unit includes it twice

#include <camera2/camera.h>
#include "BoltedParam.h"
#include <actor_system/ActorDeclare.h>

#pragma warning (disable:4481) // Override

namespace AmaxCam
{
using namespace Maths;

class C_BoltedCamera : public Cam2::C_Camera
{
public:
	C_BoltedCamera();
	void OnUpdate() OVERRIDE;
	void SetParam(const C_BoltedParam& param) { m_Param = param; }
	void SetLookAngle(F32 angle) { m_LookAngle = angle; }
	void SetOffset(MathsStorage::Float3 p_Offset) { m_Offset = p_Offset; }
	void AddOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov);
	void SetHandle(const Actor::C_ActorHandle& handle);

private:
	C_BoltedParam m_Param;
	F32 m_LookAngle;
	S32 m_PrevLookDir;
	MathsStorage::Float3 m_Offset;
	Maths::HVec3 m_EffectOffset;		
	Maths::HVec3 m_EffectOffsetRot;
	F32			 m_EffectFov;	
	Actor::C_ActorHandle m_Handle;

};

} // End of namespace
