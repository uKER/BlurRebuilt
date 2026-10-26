
#ifndef CAMERA2_CHARACTERCAMERA_H
#define CAMERA2_CHARACTERCAMERA_H

#include <camera2/Camera.h>
#include <camera2/IPhysics.h>

namespace Cam2
{

class C_CharacterCamera : public C_Camera
{
public:
	C_CharacterCamera();
	void OnUpdate() OVERRIDE;
	void SetLookAround(F32 x, F32 y);
	void SetPhysics(I_Physics* physics) { m_physics = physics; }

private:
	bool m_reset;
	F32 m_lookX, m_lookY;
	Maths::HVec3 m_trailPos; // The camera trail position
	I_Physics* m_physics;
	F32 m_distance;

	void ApplyLookAround();
	void ClipToCone();
	Maths::HVec3 FindClippedPosition(const Maths::HVec3& safe, const Maths::HVec3& want);
};

} // End of namespace Cam2

#endif
