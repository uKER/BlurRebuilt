
#include <camera2/Camera.h>

class C_MayaCameraCopy
{
private:
	Cam2::C_Camera m_camera;
	F32 m_angle[3];

public:
	C_MayaCameraCopy();
	void SetCamera(const Cam2::C_Camera& cam);
	MDoubleArray GetResult() const;
};
