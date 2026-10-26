#ifndef _HOLLYWOODCAMERA_H
#define _HOLLYWOODCAMERA_H

#include <camera2/Camera.h>
#include <camera2/IPhysics.h>
#include <camera2/stock/ChaseParam.h>
#include <actor_system/ActorDeclare.h>

namespace Kanzen
{
	class C_HollywoodCamera : public Cam2::C_Camera
	{
	public:
		C_HollywoodCamera();
		void OnUpdate() OVERRIDE;

		void SetParam(const Cam2::C_ChaseParam& param) { m_param = param; }
		const Cam2::C_ChaseParam& GetParam() { return m_param; }

		void SetLookAround(F32 angle);
		void SetLookAround(F32 sx, F32 sy);
		void Reset() { m_reset = true; }
		void SetPhysics(const Cam2::I_Physics* physics) { m_physics = physics; }
		void SetSafeHeight(F32 safeHeight) { m_SafeHeight = safeHeight; }
		void SetOffset(const Maths::HVec3& offset) { m_Offset = offset; }

		void VehicleReset();
		void SetHandle(const Actor::C_ActorHandle& handle) { m_Handle = handle; }
		void SetOffset(const Maths::HVec3& offset, F32 fov);
		F32 GetLookAngle() { return m_lookAngle; }
		bool GetReset() { return m_reset; }
		const Cam2::I_Physics* GetPhysics() { return m_physics; }
		F32 GetSafeHeight() { return m_SafeHeight; }
		const Maths::HVec3& GetOffset() { return m_Offset; }
		const Maths::HVec3& GetTrailPos() { return m_trailPos; }
		const Maths::C_Quaternion& GetLookOrient() { return m_lookOrient; }
		const Cam2::C_CamObj& GetPrevObject() { return m_prevObject; }

	private:
		Cam2::C_ChaseParam m_param;
		Cam2::C_CamObj m_prevObject;
		Maths::HVec3 m_trailPos;
		F32 m_lookAngle;
		Maths::C_Quaternion m_lookOrient;
		bool m_reset;
		const Cam2::I_Physics* m_physics;
		F32 m_SafeHeight;
		Maths::HVec3 m_Offset;

		Actor::C_ActorHandle m_Handle;

		Cam2::C_ChaseParam CurrentParam();
		F32 GetAngularSpeed();
	};

} // End of namespace Cam2

#endif  //_HOLLYWOODCAMERA_H
