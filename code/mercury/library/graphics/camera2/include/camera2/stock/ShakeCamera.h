
#ifndef CAMERA2_SHAKECAMERA_H
#define CAMERA2_SHAKECAMERA_H

#include <camera2/Camera.h>

namespace Cam2
{

F32 SoftNoise(int x);

class C_ShakeCamera : public C_Camera
{
public:
	C_ShakeCamera();
	void OnUpdate() OVERRIDE;
	void SetParam(F32 amount, F32 freq);
	void SetParamXYZ(F32 ax, F32 ay, F32 az, F32 freq);
	
protected:
	S32 m_shakePos;
	// Shake amount in degrees:
	F32 m_shakeAmountX, m_shakeAmountY, m_shakeAmountZ;
	F32 m_shakeFreq; // Frequency in hertz of shake
};

} // End of namespace Cam2

#endif
