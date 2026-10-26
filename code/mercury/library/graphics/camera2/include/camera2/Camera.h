
#ifndef CAMERA2_CAMERA_H
#define CAMERA2_CAMERA_H

#include "Feed.h"

// Forward declarations
namespace Display
{
	class C_ViewSetup3D;
}

namespace DbUtil
{
	class I_DebugRender;
}

namespace Cam2
{

class C_EffectInfo;
class C_ShakeWorld;
class C_ShakeCamera;
// Base class of camera system
// Any camera can be cast up to this to retieve position / orientation / fov etc
class C_Camera : public I_Feed
{
public:
	C_Camera();
	virtual ~C_Camera();

	virtual const C_Camera& Live() const { return *this; }

	void SetPosition(const Maths::HVec3& position) { m_Position = position; }
	Maths::HVec3 Position() const { return m_Position; }
	
	void SetOrient(const Maths::C_Quaternion& orient) { m_Orient = m_Shaken = orient; }
	Maths::C_Quaternion Orient() const { return m_Orient; }

	void SetShaken(const Maths::C_Quaternion& shaken) { m_Shaken = shaken; }
	Maths::C_Quaternion Shaken() const { return m_Shaken; } // But not stirred

	void SetFov(F32 fov) { m_Fov = fov; }
	F32 Fov() const { return m_Fov + m_FovShake; }

	void SetFarClip(F32 farClip) { m_FarClip = farClip; }
	F32 FarClip() const { return m_FarClip; }

	void SetNearClip(F32 nearClip) { m_NearClip = nearClip; }
	F32 NearClip() const { return m_NearClip; }

	void SetShutterAngle(F32 angle);
	F32 ShutterAngle() const;

	void Update(F32 period); // Calls OnUpdate and then updates m_shaken

	void StartUpdate(F32 period); // If doing a manual update of the camera 
	void EndUpdate();

	virtual void CopyFrom(const C_Camera& other);

	static Maths::C_Quaternion GetLookQuat(const Maths::HVec3& direction);
	void LookAt(const Maths::HVec3& point);

	virtual void DebugRender(Maths::RGBA colour = Maths::RGBA(255, 255, 255, 255), bool renderFrustum = false) const;

	void GetEuler(F32* pitch, F32* yaw, F32* roll) const;
	
	Maths::HVec3 InterPosition(F32 timeVal) const;
	Maths::C_Quaternion InterShaken(F32 timeVal) const;
	Maths::HMat43 InterMat43(F32 timeVal) const;
	F32 InterFov(F32 timeVal) const;

	void SetShakeWorld(C_ShakeWorld* world);
	void SetShake(F32 amount, F32 frequency = 15.f);
	void SetShake(const HVec3& amount, F32 frequency = 15.f);
	void SetShakeXYZ(F32 amountX, F32 amountY, F32 amountZ, F32 frequency = 15.f);

	void GetPrevPosition(); // todo - change this method name to 'StorePrevPosition'
	void SetIdepth(F32 idepth);
	F32 Idepth() const;
	void SetAperture(F32 aperture);
	F32 Aperture() const;
	F32 Period() const { return m_Period; }
	Maths::HVec3 Velocity() const;
	C_Camera Lerp(const C_Camera& target, F32 amount) const;
	HVec3 ToCameraSpace(const HVec3& wspos) const;
	void SetPrevPosition(const HVec3& pos);
	void SetPrevShaken(const C_Quaternion& orient);
	void SetName(const char* name);
	const char* Name() const;
	void SetFovByFocalLength(F32 focalLength);
	static F32 FovToFocalLength(F32 fov);
	F32 ImagingLength() const;
	F32 CalculateBokeh(F32 invObjectDistance) const;
	F32 FindBokehLimit(F32 diameter) const;
	Util::C_String InfoString(bool extraInfo = false) const;
	void SetEffectInfo(const C_EffectInfo& effect);
	const C_EffectInfo* EffectInfo() const;
	bool ProjectPoint(const HVec3& worldPos, F32& sx, F32& sy, F32 aspect = 16.f / 9.f) const;
	bool ProjectPointToUI(const HVec3& worldPos, F32& sx, F32& sy, F32 aspect = 16.f / 9.f) const;

private:
	Maths::HVec3 m_Position;
	Maths::C_Quaternion m_Orient, m_Shaken; // With camera shake added on
	F32 m_Fov, m_NearClip, m_FarClip;
	F32 m_ShutterAngle;
	F32 m_Idepth, m_Aperture;

	Maths::HVec3 m_PrevPosition;
	Maths::C_Quaternion m_PrevShaken;
	F32 m_PrevFov, m_FovShake;
public:
	U32 m_Blur2PCTick;	// [Blur2PC] update tick of the last GetPrevPosition (render interpolation)
	// [Blur2PC] pose at the previous update, for render interpolation. m_PrevPosition/m_PrevShaken are
	// the motion-blur inputs, which the chase and photo cameras overwrite with an artificial blur vector.
	Maths::HVec3 m_Blur2PCPrevPosition;
	Maths::C_Quaternion m_Blur2PCPrevShaken;
	F32 m_Blur2PCPrevFov;
	C_ShakeWorld* m_pShakeWorld; // The shake events which this camera reacts to, or 0 for none
	C_ShakeCamera* m_pShakeCamera; // Extra shake
	F32 m_Period;
#ifndef CFG_GOLD
	const char* m_Name;
#endif
	C_EffectInfo* m_EffectInfo;

	virtual void OnUpdate(); // Specific camera update
	void GetCameraShake(Maths::C_Quaternion& rotate, F32& fov) const;
};

bool InvertYAxis();
void SetInvertYAxis(bool invert);
void QuatToEuler(const Maths::C_Quaternion& quat, F32* pitch, F32* yaw, F32* roll);
F32 LerpValue(F32 period, F32 lag);

extern C_Camera NullCamera; // Global null camera

// In CameraDisplay.cpp:
extern bool g_RenderDeferred;
DbUtil::I_DebugRender* GetDebugRender();

} // End of namespace Cam2

#endif
