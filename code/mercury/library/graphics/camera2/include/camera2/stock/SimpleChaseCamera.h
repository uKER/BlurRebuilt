
#ifndef CAMERA2_SIMPLECHASECAMERA_H
#define CAMERA2_SIMPLECHASECAMERA_H

#include <camera2/Camera.h>
#include <camera2/IPhysics.h>
#include <camera2/stock/ChaseParam.h>

namespace Cam2
{

class C_SimpleChaseCamera : public C_Camera
{
public:
	C_SimpleChaseCamera();
	void OnUpdate() OVERRIDE;

	void SetParam(const C_ChaseParam& param) { m_param = param; }
	void SetLookAround(F32 angle);
	void SetLookAround(F32 sx, F32 sy);
	void SetPhysics(I_Physics* physics) { m_physics = physics; }

private:
	F32 m_lookAngle;
	C_ChaseParam m_param;
	Maths::C_Quaternion m_chaseOrient, m_lookOrient;
	bool m_reset;
	
	I_Physics* m_physics;
};

} // End of namespace Cam2

#endif
