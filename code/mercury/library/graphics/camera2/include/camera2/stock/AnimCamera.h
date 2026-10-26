
#ifndef CAMERA2_ANIMCAMERA_H
#define CAMERA2_ANIMCAMERA_H

#include <camera2/Camera.h>
#pragma warning (disable:4481) // nonstandard extension used: override

namespace Cam2
{
class C_AnimData;

// An AnimCamera shows a scripted animation (for example an animation exported from Maya)
class C_AnimCamera : public C_Camera
{
public:
	C_AnimCamera();
	virtual ~C_AnimCamera();
	virtual void OnUpdate() OVERRIDE;
	
	void SetAnimData(const C_AnimData* anim);
	void SetCounter(F32 c) { m_counter = c; }
	void DebugRender(Maths::RGBA colour, bool renderFrustum) const;
	void DrawPath(Maths::RGBA colour) const;

	const Util::C_String& Name() const;

private:
	const C_AnimData* m_animData;
	F32 m_length, m_counter;
	Cam2::C_Camera m_evalCamera; // Evaluated animation data

	C_Camera Evaulate(F32 ctr) const;
	void StandardOrientFov();
};

} // End of namespace Cam2

#endif
