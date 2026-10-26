#ifndef __FATALCAMERA_H__
#define __FATALCAMERA_H__

#include <camera2/camera.h>
#pragma warning (push, 3)
#include <actor_system/ActorDeclare.h>
#pragma warning (pop)
#pragma warning (disable:4481) // nonstandard extension used: override

namespace AmaxCam
{
using namespace Maths;

class C_FatalCamera : public Cam2::C_Camera
{
public:
	C_FatalCamera();
	~C_FatalCamera();
	void OnUpdate() OVERRIDE;
	void Start(const Actor::C_ActorHandle& handle, const Cam2::C_Camera& startFrom);
	void GetObject();

private:
	Actor::C_ActorHandle m_Handle;
	Cam2::C_Camera m_Start, m_End;
	F32 m_Anim;
};

} // End of namespace

#endif
