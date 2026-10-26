#ifndef NIGHTCAM_CUBE_CAPTURE_H
#define NIGHTCAM_CUBE_CAPTURE_H

#ifndef CFG_GOLD
#include <camera2/camera.h>

namespace AmaxCam
{

class C_CubeCapture
{
public:
	C_CubeCapture();
	~C_CubeCapture();
	
	// This update is called all the time:
	void RealUpdate();

private:
	int m_Counter;
	Cam2::C_Camera m_OldCamera;

	void Begin();
	void End();
	void PointInDirection(int dir);
};

}
#endif

#endif
