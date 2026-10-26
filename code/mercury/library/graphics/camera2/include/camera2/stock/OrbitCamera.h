
#include <camera2/Camera.h>

#pragma warning (disable:4481)

namespace Cam2
{

class C_OrbitCamera : public Cam2::C_Camera
{
public:
	C_OrbitCamera();
	~C_OrbitCamera();
	void OnUpdate() OVERRIDE;

private:
	F32 m_Angle;
};

} // End of namespace
