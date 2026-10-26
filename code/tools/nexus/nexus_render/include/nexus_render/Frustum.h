#pragma once

namespace NexusRender
{
	class Frustum
	{
	public:
		void ExtractPlanes(const Maths::HMat44& projectionMatrix);

		bool Contains(const Sphere& sphere) const;

		bool Contains(const Maths::HVec3& point) const;

	private:
		Plane m_nearPlane;
		Plane m_farPlane;
		Plane m_leftPlane;
		Plane m_rightPlane;
		Plane m_topPlane;
		Plane m_bottomPlane;
	};
}
