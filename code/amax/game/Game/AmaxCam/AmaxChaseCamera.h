#ifndef __VEHICLECAMERA_H__
#define __VEHICLECAMERA_H__

#include <camera2/CamObj.h>
#include <camera2/stock/ChaseCamera2.h>
#pragma warning (push, 3)
#include <actor_system/ActorDeclare.h>
#pragma warning (pop)

#include "AmaxChaseParam.h"

namespace AmaxCam
{
using namespace Maths;

class C_AmaxChaseParam;

class C_AmaxChaseCamera : public Cam2::C_ChaseCamera2
{
public:
	C_AmaxChaseCamera();
	virtual ~C_AmaxChaseCamera();
	void VehicleReset();
	void OnUpdate() OVERRIDE;
	void Update2();
	void SetHandle(const Actor::C_ActorHandle& handle) { m_Handle = handle; }
	void AddOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov);
	void SetAmaxParam(const C_AmaxChaseParam& param) { m_AmaxParam = param; }
	void SetRaytrace(bool trace) { m_Raytrace = trace; }
	void SetMotionBlur(const HVec3& motion);

private:
	Actor::C_ActorHandle m_Handle;
	bool m_AmaxReset;
	F32 m_PrevSpeed, m_SoftAccel;
	Maths::HVec3 m_AccelOffset, m_EffectOffset, m_EffectOffsetRot; // The offset to the camera from acceleration, and special effects
	F32 m_AccelFov, m_EffectFov;
	Maths::C_Quaternion m_PlainOrient;
	C_AmaxChaseParam m_AmaxParam;
	bool m_Raytrace;

	void UpdateParams();
	void BlankParams();
};

} // End of namespace

#endif
