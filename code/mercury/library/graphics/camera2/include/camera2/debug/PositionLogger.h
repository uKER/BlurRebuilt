
#ifndef CAMERA2_POSITIONLOGGER_H
#define CAMERA2_POSITIONLOGGER_H

namespace Cam2
{

class C_PosFrame
{
public:
	Maths::HVec3 m_position;
	Maths::C_Quaternion m_orient;
	F32 m_fov;
	
	C_PosFrame() { m_fov = 0.f; }
};

class C_Camera;
class C_PositionLogger
{
public:
	C_PositionLogger();
	void Reset();
	void AddFrame(const C_Camera& camera);
	bool WriteAfterEffects();
	bool WriteMaya();

private:	
	Ut::Vector<C_PosFrame> m_frames;
};

} // End of namespace Cam2

#endif
