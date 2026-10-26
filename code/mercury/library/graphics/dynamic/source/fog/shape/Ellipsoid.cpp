#include "stdafx.h"
#include "Dynamic/Fog/Shape/Ellipsoid.h"
#include "Dynamic/Fog/Enums.h"
#include "Dynamic/Fog/Equations.h"
#include "Dynamic/Shaders/Constants.h"
#include "Render/State/RenderState.h"

namespace DynamicEffect
{
namespace Fog
{

Debug::Int g_debugEllipsoidRings("Render.Fog.Ellipsoid.Rings", 16, Debug::Range(2, 32));
Debug::Int g_debugEllipsoidSegments("Render.Fog.Ellipsoid.Segments", 16, Debug::Range(2, 32));

Ellipsoid::Ellipsoid()
{
	m_eShape = E_Shape_Ellipsoid;
}

bool Ellipsoid::Create()
{
	m_VertexCount	= (g_debugEllipsoidRings + 1) * (g_debugEllipsoidSegments + 1);
	m_IndexCount	= (g_debugEllipsoidRings * (g_debugEllipsoidSegments + 2)) * 2;
	m_PrimitiveType	= m_PrimitiveType;

	if(!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XYZ), 0, &m_VertexBuffer))
		return false;
	if(!Display::Device()->CreateIndexBuffer(m_IndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		return false;

	Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)m_VertexBuffer.Lock();
	assert(pVertex);

	U16* pIndex = (U16*)m_IndexBuffer.Lock();
	assert(pIndex);
	
	U16 vertexIndex = 0;
    F32 deltaRingAngle = Maths::Pi / g_debugEllipsoidRings;
	F32 deltaSegmentAngle = 2.f * Maths::Pi / g_debugEllipsoidSegments;
	F32 multiplier = 1.0f / (cosf(deltaRingAngle) * cosf(deltaSegmentAngle)); // half segment angle?

	for (S32 ringIndex=0; ringIndex < g_debugEllipsoidRings + 1; ++ringIndex)
	{
		F32 r = sinf(ringIndex * deltaRingAngle) * multiplier;
		F32 y = cosf(ringIndex * deltaRingAngle) * multiplier;
		for (S32 segmentIndex=0; segmentIndex < g_debugEllipsoidSegments + 1; ++segmentIndex)
		{
			pVertex->x = r * sinf(segmentIndex * deltaSegmentAngle);
			pVertex->y = y;
			pVertex->z = r * cosf(segmentIndex * deltaSegmentAngle);
			++pVertex;
			if (ringIndex != g_debugEllipsoidRings)
			{
				*pIndex++ = vertexIndex;
				*pIndex++ = vertexIndex + (U16)(g_debugEllipsoidSegments + 1);
				if (segmentIndex == g_debugEllipsoidSegments)
				{
					*pIndex++ = vertexIndex + (U16)(g_debugEllipsoidSegments + 1);
					*pIndex++ = vertexIndex + (U16)(g_debugEllipsoidSegments + 2);
				}
				++vertexIndex;
			}
		}
	}

	m_VertexBuffer.Unlock();
	m_IndexBuffer.Unlock();

	return true;
}

void Ellipsoid::Destroy()
{
	Display::Device()->Destroy(&m_IndexBuffer);
	Display::Device()->Destroy(&m_VertexBuffer);
}

bool Ellipsoid::IsInside(const Maths::HVec3& vPoint, const Maths::HMat43& mObject, const Maths::HF32& vEpsilon) const
{
	const Maths::HMat44 mWorld = Maths::HMat44(mObject).Inverse();
	const Maths::HVec3 vPosition = Maths::HVec3(mWorld.AW());
	const Maths::C_Normal vDirection (Normalise(vPoint - vPosition));

	Maths::HVec3 vObject = Maths::HVec3(vPoint + vDirection * vEpsilon) * mObject;
	return (bool) vObject.Length().LE(Maths::Constants<Maths::HF32>::One() + vEpsilon);
}

Maths::HF32 Ellipsoid::EvaluateDensityAt(const Maths::HVec3& vPoint, const Maths::HMat43& mObject, const Maths::HVec2& vSettings) const
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

		const Maths::HF32 vC2 = vObjectPosition | vObjectPosition;
		const Maths::HF32 vM2 = vObjectViewVector | vObjectViewVector;
		const Maths::HF32 vCDotM = vObjectPosition | vObjectViewVector;
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
