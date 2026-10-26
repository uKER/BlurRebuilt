
#ifndef CAMERA2_SHAKEWORLD_H
#define CAMERA2_SHAKEWORLD_H

namespace Cam2
{

class C_ShakeCamera;
class C_ShakeEvent;

class C_ShakeWorld
{
public:
	static C_ShakeWorld& Instance();

	C_ShakeWorld();
	~C_ShakeWorld();
	void Reset();
	void AddEvent(const Maths::HVec3& worldPos, F32 frequency = 20.f, F32 duration = 1.f, F32 magnitude = 5.f, F32 range = 100.f, F32 fovMagnitude = 0.f);
	void Update(F32 period);
	void GetCameraShake(const Maths::HVec3& position, Maths::C_Quaternion* pRetQuat, F32 *ptotalAmp = 0, F32* pRetFovShake = 0) const;

	C_ShakeCamera* m_pGlobalShake;

private:	
	Ut::Vector<C_ShakeEvent*> m_ShakeList;
};

}

#endif
