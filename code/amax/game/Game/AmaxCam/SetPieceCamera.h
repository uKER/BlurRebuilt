#ifndef __SETPIECECAMERA_H__
#define __SETPIECECAMERA_H__

#include <camera2/camera.h>
#pragma warning (push, 3)
#include <actor_system/ActorDeclare.h>
#pragma warning (pop)
#pragma warning (disable:4481) // nonstandard extension used: override

namespace AmaxCam
{
using namespace Maths;

class C_SetPieceCamera : public Cam2::C_Camera
{
public:
	C_SetPieceCamera();
	~C_SetPieceCamera();
	void OnUpdate() OVERRIDE;
	void Start(const Actor::C_ActorHandle& handle, Maths::HVec3 p_Position, Maths::HVec3 p_LookAt, float p_Roll, bool p_TrackObject);
	void GetObject();

private:
	Actor::C_ActorHandle	m_Handle;
	Maths::HVec3			m_LookAt;
	bool					m_TrackObject;
	float					m_Fov;
};

} // End of namespace

#endif
