#include "stdafx.h"
#include "Display/View/ViewFrustum.h"
#include "Display/Window/WindowBase.h"

namespace Display
{

C_ViewFrustum::C_ViewFrustum() :
	m_PlaneCount(0)
{
}

C_ViewFrustum::~C_ViewFrustum()
{
}

void C_ViewFrustum::Setup(const U32 planeCount, const Maths::C_Plane* pPlanes)
{
	assert(planeCount <= m_MaxPlaneCount);

	m_PlaneCount = planeCount;
	for(U32 i=0; i<m_PlaneCount;i++)
	{
		Maths::Store(pPlanes[i].m_V, &m_ClipPlanes[i]);
	}
	for(U32 i=m_PlaneCount; i<m_MaxPlaneCount;i++)
	{
		m_ClipPlanes[i] = MathsStorage::Float4A16(0.0f, 1.0f, 0.0f, -10000.0f);
	}

	SetupClipPlaneAbs();
}

void C_ViewFrustum::Setup(const Maths::HMat44& cameraToDevice, const Maths::HMat44& worldToDevice, const Maths::HF32& viewWidth)
{
	Maths::HVec4 clipPlanesAbs[6];
	Maths::HVec4 clipPlanes[6];

	// get a transposed view proj matrix so the rows can be extracted
	const Maths::HMat44& worldToDeviceTransposed = worldToDevice.Transposed();

	// TODO: move this out of this class
	// get a scaling factor based on the view width (1.0f for fullscreen) and the _11 element of the projection matrix
	Maths::HF32 forwardMultiplier = 1.0f / (viewWidth * Maths::Abs(cameraToDevice.AX().X()));
	// multiply 4th row of projection matrix to get sizeTransform - fov adjusted distance culling
	Maths::HVec4 forward = worldToDeviceTransposed.AW() * forwardMultiplier;
	Maths::HVec4 forwardAbs(Maths::Abs(forward.X()), Maths::Abs(forward.Y()), Maths::Abs(forward.Z()), Maths::Abs(forward.W()));
	Maths::Store(forward, &m_Forward);
	Maths::Store(forwardAbs, &m_ForwardAbs);
	m_ForwardMultiplier = forwardMultiplier;
	// this is used in the form sizeTransform | position to get a distance scaled by view frustum fov (for use with distance culling)
	
	// left and right
	clipPlanes[0] = worldToDeviceTransposed.AW() + worldToDeviceTransposed.AX();
	clipPlanes[1] = worldToDeviceTransposed.AW() - worldToDeviceTransposed.AX();

	// bottom and top
	clipPlanes[2] = worldToDeviceTransposed.AW() + worldToDeviceTransposed.AY();
	clipPlanes[3] = worldToDeviceTransposed.AW() - worldToDeviceTransposed.AY();

	// near and far
	clipPlanes[4] = worldToDeviceTransposed.AZ();
	clipPlanes[5] = worldToDeviceTransposed.AW() - worldToDeviceTransposed.AZ();

	// normalise the planes
	for(U32 i = 0; i < 6; i++)
	{
		Maths::HF32 lengthSq = Maths::LengthSquared(Maths::HVec3(clipPlanes[i]));
		clipPlanes[i] = clipPlanes[i] * lengthSq.RcpSqrt();
	}

	// store planes
	for(U32 i = 0; i < 6; ++i)
	{
		Maths::Store(clipPlanes[i], &m_ClipPlanes[i]);
	}

	// calculate points of the frustum
	for(U32 i = 0; i < 8; ++i)
	{
		const Maths::C_Plane &p0 = (i & 1) ? Maths::C_Plane(clipPlanes[5]) : Maths::C_Plane(clipPlanes[4]);
		const Maths::C_Plane &p1 = (i & 2) ? Maths::C_Plane(clipPlanes[2]) : Maths::C_Plane(clipPlanes[3]);
		const Maths::C_Plane &p2 = (i & 4) ? Maths::C_Plane(clipPlanes[1]) : Maths::C_Plane(clipPlanes[0]);
	
		Maths::C_Vertex point;
		Maths::IntersectPlanes(p0, p1, p2, point);

		Maths::Store(Maths::HVec4(point), &m_Points[i]);
	}

	m_PlaneCount = 6;

	SetupClipPlaneAbs();
}

void C_ViewFrustum::SetupClipPlaneAbs()
{
	// abs clip planes used for projecting box size for AABB culling
	for(U32 i = 0; i < m_PlaneCount; i++)
	{
		Maths::HVec4 clipPlane = Maths::Load<Maths::HVec4>(&m_ClipPlanes[i]);
		Maths::HVec4 clipPlaneAbs(-Maths::Abs(clipPlane.X()), -Maths::Abs(clipPlane.Y()), -Maths::Abs(clipPlane.Z()), Maths::Constants<Maths::HF32>::Zero());
		Maths::Store(clipPlaneAbs, &m_ClipPlanesAbs[i]);
	}
}

void C_ViewFrustum::SetupTilingPlanes(const Maths::HMat44& worldToDevice, const C_TileSetup& tileSetup)
{
}

}; // namespace Display
