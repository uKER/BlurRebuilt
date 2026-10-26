#ifndef __VEHICLEFEED_H__
#define __VEHICLEFEED_H__

#include <camera2/Feed.h>
#pragma warning (push, 3)
#include <actor_system/ActorDeclare.h>
#pragma warning (pop)

#include <btl/containers/vector.h>

#include "VFCamera.h"

// Forward declarations:
namespace Cam2 { class C_BlendCamera; }
namespace Game
{
	class C_GameProfile;
	namespace Actors { class C_VehicleActor; }
}
namespace Kanzen { class C_HollywoodVehicleCamera; }
namespace Input { class C_GameInput; }

namespace AmaxCam
{
using namespace Maths;

class C_AmaxChaseCamera;
class C_BoltedCamera;
class C_ShutterCurve;
class C_VehicleFeedParam;

class C_VehicleFeed : public Cam2::I_Feed
{
public:
	C_VehicleFeed();
	~C_VehicleFeed();
	void Init();
	void Exit();
	void CameraChangeUpdate(F32 period);
	void Update(F32 period) OVERRIDE;
	void Update2(F32 period);
	const Cam2::C_Camera& Live() const OVERRIDE;
	void DebugRender() const;
	void SetHandle(const Actor::C_ActorHandle& handle);
	void Reset();
	void AddOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov);
	S32 GetAudioMapping() const;
	void SetFuture(bool future) { m_Future = future; }
	void ChangeCamera(S32 index, const F32 blendPeriod = 0.0f); // Used by automation
	void SetMotionBlur(const HVec3& motion);
	E_VFCamera CurrentType() const { return (E_VFCamera)m_Choice; }
	void SetPadIndex(S32 index) { m_PadIndex = index;; }
	S32 PadIndex() const { return m_PadIndex; }
	
	// This function is called by last gasp to disable changing the camera while last gasp is active
	// Changing camera is only allowed if the vehicle feed is shown and this is true:
	void SetChangeViewAllowed(bool allowed) { m_ChangeViewAllowed = allowed; }

	Cam2::C_Camera* GetCamera(E_VFCamera p_Type);

	F32 GetLookAngle() const { return m_LookAngle; }
	const Cam2::C_Camera* ChaseCamera() const;
	HMat43 LastKnownSafeMatrix() const;
	HVec3 VehicleVelocity() const;
	void SetShown(bool shown);
	bool Shown() const { return m_Shown; }

private:
	Actor::C_ActorHandle m_Handle;
	Cam2::C_BlendCamera* m_BlendCamera;
	Btl::Vector<Cam2::C_Camera*> m_Cameras;
	C_AmaxChaseCamera* m_Chase;
	C_BoltedCamera *m_Bumper;
	Kanzen::C_HollywoodVehicleCamera *m_pHollywoodCamera;
	S32 m_Choice;
	E_VFCamera m_CurrentVehicleCamera;
	F32 m_LookX, m_LookY, m_LookAngle;
	bool m_Future;
	S32 m_PadIndex;
	const C_VehicleFeedParam* m_VehicleParam;
	HVec3 m_smoothAcceleration;
	HVec3 m_smoothVelocity;
	HVec3 m_smoothAngularVelocity;
	bool m_ChangeViewAllowed;
	bool m_Shown;

	void UpdateCameraObject(const Game::Actors::C_VehicleActor* vehicleActor);
	void UpdateBlendCamera(F32 period);
	void UpdateParam();
	Input::C_GameInput* GameInput();
	Game::C_GameProfile* GetGameProfile();
	void LerpShutterAngle(Cam2::C_Camera* camera, const C_ShutterCurve* shutterCurve);
};

} // End of namespace

#endif
