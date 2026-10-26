#include "stdafx.h"
#include "Dynamic/Fog/Shape/Cylinder.h"
#include "Dynamic/Fog/Enums.h"
#include "Dynamic/Fog/Equations.h"
#include "Dynamic/Shaders/Constants.h"
#include "Render/State/RenderState.h"

namespace DynamicEffect
{
namespace Fog
{

Debug::Int g_debugCylinderSegments("Render.Fog.Cylinder.Segments", 16, Debug::Range(2, 32));

Cylinder::Cylinder()
{
	m_eShape = E_Shape_Cylinder;
}

bool Cylinder::Create()
{
	m_VertexCount	= 2 * g_debugCylinderSegments;
	m_IndexCount	= 2 * (g_debugCylinderSegments + 2 * ((g_debugCylinderSegments + 1) / 2) + 3);
	m_PrimitiveType	= Display::E_PrimitiveType_TriangleStrip;

	if(!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XYZ), 0, &m_VertexBuffer))
		return false;
	if(!Display::Device()->CreateIndexBuffer(m_IndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		return false;

	Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)m_VertexBuffer.Lock();
	assert(pVertex);

	F32 deltaSegmentAngle = 2.0f * Maths::Pi / g_debugCylinderSegments;
	for (S32 segmentNo = 0; segmentNo < g_debugCylinderSegments; ++segmentNo)
	{
		pVertex->x = sinf(segmentNo * deltaSegmentAngle);
		pVertex->y = +1.0f;
		pVertex->z = cosf(segmentNo * deltaSegmentAngle);
		++pVertex;
	}
	for (S32 segmentNo = 0; segmentNo < g_debugCylinderSegments; ++segmentNo)
	{
		pVertex->x = sinf(segmentNo * deltaSegmentAngle);
		pVertex->y = -1.0f;
		pVertex->z = cosf(segmentNo * deltaSegmentAngle);
		++pVertex;
	}

	m_VertexBuffer.Unlock();

	U16* pIndex = (U16*)m_IndexBuffer.Lock();
	assert(pIndex);

	// cylinder
	for (S32 segmentNo = 0; segmentNo < g_debugCylinderSegments; ++segmentNo)
	{
		*pIndex++ = segmentNo;
		*pIndex++ = segmentNo + g_debugCylinderSegments;
	}
	*pIndex++ = 0;
	*pIndex++ = g_debugCylinderSegments;
	*pIndex++ = g_debugCylinderSegments; // degenerate
	*pIndex++ = g_debugCylinderSegments - 1; // degenerate
	// bottom
	for (S32 segmentNo = 0; segmentNo < (g_debugCylinderSegments + 1) / 2; ++segmentNo)
	{
		*pIndex++ = g_debugCylinderSegments - segmentNo - 1;
		*pIndex++ = segmentNo;
	}
	*pIndex++ = (g_debugCylinderSegments + 1) / 2 - 1; // degenerate
	*pIndex++ = g_debugCylinderSegments; // degenerate
	// top
	for (S32 segmentNo = 0; segmentNo < (g_debugCylinderSegments + 1) / 2; ++segmentNo)
	{
		*pIndex++ = segmentNo + g_debugCylinderSegments;
		*pIndex++ = g_debugCylinderSegments + g_debugCylinderSegments - segmentNo - 1;
	}

	m_IndexBuffer.Unlock();

	return true;
}

void Cylinder::Destroy()
{
	Display::Device()->Destroy(&m_IndexBuffer);
	Display::Device()->Destroy(&m_VertexBuffer);
}

bool Cylinder::IsInside(const Maths::HVec3& vPoint, const Maths::HMat43& mObject, const Maths::HF32& vEpsilon) const
{
	const Maths::HVec3 vMask(Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::One());

	const Maths::HMat44 mWorld = Maths::HMat44(mObject).Inverse();
	const Maths::HVec3 vPosition = Maths::HVec3(mWorld.AW());
	const Maths::C_Normal vDirection (Normalise((vPoint - vPosition) * vMask));

	Maths::HVec3 vObject = Maths::HVec3(vPoint + vDirection * vEpsilon) * mObject;
	vObject = vObject * vMask;

	return (bool) vObject.Length().LE(Maths::Constants<Maths::HF32>::One() + vEpsilon);
}

Maths::HF32 Cylinder::EvaluateDensityAt(const Maths::HVec3& vPoint, const Maths::HMat43& mObject, const Maths::HVec2& vSettings) const
{
	const Maths::HF32 vZero = Maths::Constants<Maths::HF32>::Zero();
	const Maths::HF32 vOne = Maths::Constants<Maths::HF32>::One();

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	const Maths::HVec3 vViewPosition(viewSettings3D.GetCameraPosition());

	Maths::HVec3 vEntryPosition = vViewPosition;
	if (!IsInside(vViewPosition, mObject))
	{
		const Maths::HVec3 vWorldViewVector(vViewPosition - vPoint);
		const Maths::HVec3 vObjectViewVector(vWorldViewVector * mObject.Rotation());
		const Maths::HVec3 vObjectPosition(vPoint * mObject);
		const Maths::HVec2 vC(vObjectPosition.X(), vObjectPosition.Z());
		const Maths::HVec2 vM(vObjectViewVector.X(), vObjectViewVector.Z());

		const Maths::HF32 vC2 = vC | vC;
		const Maths::HF32 vM2 = vM | vM;
		const Maths::HF32 vCDotM = vC | vM;
		const Maths::HF32 vDeterminant = vCDotM * vCDotM - vM2 * (vC2 - vOne);
		const Maths::HF32 vLambda = (-vCDotM + Maths::Sqrt(vDeterminant)) * vM2.Rcp();
		
		vEntryPosition = vPoint + vLambda * vWorldViewVector;
	}

	const Maths::HVec3 vViewVector = vPoint - vEntryPosition;
	const Maths::HVec3 vNewSettings(vSettings.X(), vSettings.Y(), expf(-vSettings.Y() * vEntryPosition.Y()));

	return GetVolumetricFog(vViewVector, vNewSettings);
}

}; // namespace Fog
}; // namespace DynamicEffect
