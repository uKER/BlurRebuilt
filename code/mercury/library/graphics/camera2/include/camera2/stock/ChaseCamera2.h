
#ifndef CAMERA2_CHASECAMERA2_H
#define CAMERA2_CHASECAMERA2_H

#include <camera2/Camera.h>
#include <camera2/IPhysics.h>
#include <camera2/stock/ChaseParam.h>

namespace Cam2
{
using namespace Maths;

class C_ChaseCamera2 : public C_Camera
{
public:
	C_ChaseCamera2();
	void OnUpdate() OVERRIDE;
	void Update2();
	void SetParam(const C_ChaseParam& param) { m_param = param; }
	void SetLookAround(F32 angle);
	void SetLookAround(F32 sx, F32 sy);
	void Reset() { m_reset = true; }
	void SetPhysics(I_Physics* physics) { m_physics = physics; }
	void SetSafeHeight(F32 safeHeight) { m_SafeHeight = safeHeight; }
	void SetOffset(const Maths::HVec3& offset) { m_Offset = offset; }

private:
	C_ChaseParam m_param;
	C_CamObj m_prevObject;
	HVec3 m_PrevPivot;
	HVec3 m_trailPos;
	F32 m_lookAngle;
	C_Quaternion m_lookOrient;
	bool m_reset;
	I_Physics* m_physics;
	F32 m_SafeHeight;
	HVec3 m_Offset;
	S32 m_RayHandle;
	HVec3 m_Safe, m_Want;

	C_ChaseParam CurrentParam();
	F32 GetAngularSpeed();
};

} // End of namespace Cam2

#endif
