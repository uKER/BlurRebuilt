
#ifndef BOXCAR_H
#define BOXCAR_H

namespace Framework
{

class C_BoxCar
{
public:
	C_BoxCar();
	void Update(F32 period);
	void Render() const;
	void ReadControls();

	S32 m_activeControls;
	F32 m_right, m_forw, m_steering, m_rise;

	Maths::HVec3 m_position;
	Maths::C_Quaternion m_orient;
};

}

#endif //BOXCAR_H
