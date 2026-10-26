#pragma once

namespace Bounds
{

class C_Bounds
{
public:
	C_Bounds();
	~C_Bounds();

	void					AddPoint(const Maths::HVec3& point);
	void					AddSphere(const Maths::C_Sphere& sphere);

	// Not currently the smallest bounding sphere but an approximate one
	const Maths::C_Sphere	CalculateSphere();
	const Maths::C_Sphere	CalculateSphere2();
	const Maths::C_Sphere   CalculateSphere3();
	const Maths::C_Sphere	CalculateSphere4();
	const Maths::C_Sphere	CalculateSphere5();
	const Maths::C_AxisBox  CalculateBox();

public:

	Ut::Vector<Maths::HVec3>		m_PointList;
	Ut::Vector<Maths::C_Sphere>		m_SphereList;

	bool							m_SphereValid;
	bool							m_BoxValid;

	Maths::C_Sphere     			m_Sphere;
	Maths::C_AxisBox				m_Box;
};

} // namespace Bounds
