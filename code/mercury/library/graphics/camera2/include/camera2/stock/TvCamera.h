
#ifndef CAMERA2_TVCAMERA_H
#define CAMERA2_TVCAMERA_H

#include <camera2/Camera.h>

namespace Cam2
{

class C_TvCamera : public C_Camera
{
public:
	C_TvCamera();
	void OnUpdate() OVERRIDE;
	
	void SetViewRange(F32 range);
	
private:
	F32 m_viewRange;
};

} // End of namespace Cam2

#endif
