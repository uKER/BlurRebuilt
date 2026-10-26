#pragma once

class Frustum
{
public:
	enum FrustumPlane
	{
		NEAR_PLANE,
		FAR_PLANE,
		LEFT_PLANE,
		RIGHT_PLANE,
		TOP_PLANE,
		BOTTOM_PLANE
	};

	void ExtractPlanes(const HMat44& projectionMatrix);
	bool IntersectBox(box3f &bbox);

private:
	plane3f m_planes[6];
};
