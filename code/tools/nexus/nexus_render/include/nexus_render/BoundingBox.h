#pragma once

namespace NexusRender
{
	class BoundingBox
	{
	public:
		BoundingBox();

		~BoundingBox();

		void SetBounds(const Aftk::C_Bounds& bounds);

		void SetBounds(const Maths::HVec3& min, const Maths::HVec3& max);

		void Draw();

	private:
		PrimitiveRenderer m_pr;
	};
}

