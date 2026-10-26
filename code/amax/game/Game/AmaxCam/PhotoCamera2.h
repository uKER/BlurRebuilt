#ifndef PHOTOCAMERA2_H
#define PHOTOCAMERA2_H

#include <camera2/Camera.h>

namespace Collision { class C_Shape; }
namespace Gravity
{
	class C_CollisionPhantom;
	class C_Scene;
}
namespace Input { class C_GameInput; }
namespace SharedPhysics { class C_ContactStream; }

namespace AmaxCam
{
using namespace Maths;

class C_PhysicsChecks;
class C_VehicleFeed;

class C_PhotoCamera2 : public Cam2::C_Camera
{
public:
	C_PhotoCamera2();
	~C_PhotoCamera2();
	void Init();
	void Exit();
	void CopyFromVehicleFeed(const C_VehicleFeed* vehicleFeed);
	void OnUpdate() OVERRIDE;
	void SetMotionVector(const HVec3& vector);
	void EnableControls(bool enable) { m_ControlsEnabled = enable; }
	void SetPadIndex(S32 pad) { m_PadIndex = pad; }
	void AutoFocusUpdate();
	HVec3 MotionBlur() const { return m_MotionBlur; }

private:
	class C_Input
	{
	public:
		HVec3 move;
		F32 yaw, pitch, roll, zoom, up;
		bool rollReset, zoomReset;
		C_Input() { yaw = pitch = roll = zoom = up = 0.f; rollReset = zoomReset = false; }
	};

	class C_Contact
	{
	public:
		HVec3 point, normal;
		F32 penetration;
		F32 lambda;
		C_Contact() { penetration = lambda = 0.f; }
	};

	S32 m_PadIndex;
	Input::C_GameInput* m_GameInput;
	bool m_ControlsEnabled;
	bool m_InvertY, m_RightStick;
	F32 m_Radius;
	C_Input m_Input;
	HVec3 m_Velocity;
	S32 m_SchedulerGroup, m_SchedulerPolicy;
	Gravity::C_Scene* m_Scene;
	C_PhysicsChecks* m_PhysicsChecks;
	Collision::C_Shape* m_Shape;
	Gravity::C_CollisionPhantom* m_Phantom;
	U8* m_StackData;
	SharedPhysics::C_ContactStream* m_ContactStream;
	Btl::Vector<C_Contact> m_Contacts;
	F32 m_GroundHeight;
	HVec3 m_GroundNormal;
	HVec3 m_RangeCentre;
	HVec3 m_MotionBlur;

	void TimeStep(F32 period);
	void GetGameInput();
	void GetControls();
	void UpdatePhantom();
	void GetContacts();
	void HeightConstraint();
	void RangeConstraint();
	void PlaneConstraint(F32 height, F32 facing);
	void RenderContacts();
	void ProcessContacts();
	void ReadProfileSettings();
	S32 CalcAddr(S32 x, S32 y, S32 pitch);
	F32 GroundBelow(HVec3 position, HVec3* normal);
	void AutoFocus();
};

} // End of namespace

#endif
