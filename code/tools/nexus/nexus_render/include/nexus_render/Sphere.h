#pragma once

namespace NexusRender
{
	class Sphere
	{
	public:
		Sphere();
		Sphere(const Maths::HVec3& position, F32 radiusSq);
		void SetPosition(const Maths::HVec3& position);
		void SetRadiusSq(F32 radius);

		F32 RadiusSq() const;
		const Maths::HVec3& Position() const;

	private:
		Maths::HVec3	m_position;
		F32				m_radiusSq;
	};
}