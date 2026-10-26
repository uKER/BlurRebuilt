#include "stdafx.h"
#include "Render/Geometry/InfinitePlane.h"
#include "Display/Buffer/VertexDeclarations.h"

namespace Render
{
namespace Geometry
{

C_InfinitePlane::C_InfinitePlane() : m_vPlane(Maths::HVec4(0.0f, -1.0f, 0.0f, 0.0f))
{
}

C_InfinitePlane::~C_InfinitePlane()
{
}

bool C_InfinitePlane::Create()
{
	return false;
}

void C_InfinitePlane::Destroy()
{
}

void C_InfinitePlane::Render()
{
	const Maths::HMat43& mCamera = Display::Device()->GetViewSettings3D().GetCameraToWorld();
	const Maths::HMat43& mView = Display::Device()->GetViewSettings3D().GetWorldToCamera();
	const Maths::C_Plane& vViewPlane = m_vPlane * mView;

	const Display::C_ViewFrustum& viewFrustum = Display::Device()->GetViewFrustum();
	
	const U32 iStart[8] = { 0, 4, 1, 5, 2, 7, 3, 4 };
	const U32 iEnd[8] = { 4, 5, 5, 6, 6, 6, 7, 7 };

	Maths::HVec3 vWorldCoords[8];
	U32 iNumIntersections = 0;
	for (U32 i = 0; i < 8; ++i)
	{
		const Maths::C_Vertex vStart = viewFrustum.GetPoint(iStart[i]);
		const Maths::C_Vertex vEnd = viewFrustum.GetPoint(iEnd[i]);
		const Maths::HF32 vT = vViewPlane.Intersect(Maths::C_Line(vStart, vEnd));
		if (Maths::Constants<Maths::HF32>::Zero() <= vT && vT <= Maths::Constants<Maths::HF32>::One())
		{
			vWorldCoords[iNumIntersections++] = Maths::Lerp(vStart, vEnd, vT) * mCamera;
		}
	}

	Display::Vertex_XYZ* pVertices = (Display::Vertex_XYZ*) Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleFan, iNumIntersections, sizeof(Display::Vertex_XYZ), Display::Vertex_XYZ::m_pDeclaration);
	if (pVertices)
	{
		for (U32 i = 0; i < iNumIntersections; ++i)
		{
			Maths::Store(vWorldCoords[i], (MathsStorage::Float3*)pVertices);
			++pVertices;
		}
		Display::Device()->EndVertices();
	}
}

}; // namespace Geometry
}; // namespace Render
