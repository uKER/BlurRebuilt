#ifndef PASSING_SHAKE_H
#define PASSING_SHAKE_H

namespace AmaxCam
{
using namespace Maths;

class C_PassingShake
{
public:
	C_PassingShake();
	~C_PassingShake();
	void Update();

private:
	float m_VelocityInDir[20]; // Velocity in the direction of the camera
};

// C_PassingShake Singleton:
C_PassingShake& PassingShake();

} // End of namespace

#endif
