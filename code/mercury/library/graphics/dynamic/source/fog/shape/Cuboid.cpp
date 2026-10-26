#include "stdafx.h"
#include "Dynamic/Fog/Shape/Cuboid.h"
#include "Dynamic/Fog/Enums.h"
#include "Dynamic/Fog/Equations.h"
#include "Dynamic/Shaders/Constants.h"
#include "Render/State/RenderState.h"

namespace DynamicEffect
{
namespace Fog
{

Cuboid::Cuboid()
{
	m_eShape = E_Shape_Cuboid;	
}

bool Cuboid::Create()
{
	m_VertexCount	= 8;
	m_IndexCount	= 36;
	m_PrimitiveType	= Display::E_PrimitiveType_TriangleList;
	
	if(!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XYZ), 0, &m_VertexBuffer))
		return false;
	if(!Display::Device()->CreateIndexBuffer(m_IndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		return false;

	Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)m_VertexBuffer.Lock();
	assert(pVertex);

	pVertex->x = -1.0f; pVertex->y = -1.0f; pVertex->z = -1.0f; ++pVertex;
	pVertex->x = -1.0f; pVertex->y = -1.0f; pVertex->z = +1.0f; ++pVertex;
	pVertex->x = -1.0f; pVertex->y = +1.0f; pVertex->z = -1.0f; ++pVertex;
	pVertex->x = -1.0f; pVertex->y = +1.0f; pVertex->z = +1.0f; ++pVertex;
	pVertex->x = +1.0f; pVertex->y = -1.0f; pVertex->z = -1.0f; ++pVertex;
	pVertex->x = +1.0f; pVertex->y = -1.0f; pVertex->z = +1.0f; ++pVertex;
	pVertex->x = +1.0f; pVertex->y = +1.0f; pVertex->z = -1.0f; ++pVertex;
	pVertex->x = +1.0f; pVertex->y = +1.0f; pVertex->z = +1.0f; ++pVertex;

	m_VertexBuffer.Unlock();

	U16* pIndex = (U16*)m_IndexBuffer.Lock();
	assert(pIndex);

	// top
	*pIndex++ = 2; *pIndex++ = 3; *pIndex++ = 7;
	*pIndex++ = 2; *pIndex++ = 7; *pIndex++ = 6;
	// bottom
	*pIndex++ = 0; *pIndex++ = 5; *pIndex++ = 1;
	*pIndex++ = 0; *pIndex++ = 4; *pIndex++ = 5;
	// left
	*pIndex++ = 1; *pIndex++ = 3; *pIndex++ = 2;
	*pIndex++ = 1; *pIndex++ = 2; *pIndex++ = 0;
	// right
	*pIndex++ = 4; *pIndex++ = 6; *pIndex++ = 5;
	*pIndex++ = 5; *pIndex++ = 6; *pIndex++ = 7;
	// front
	*pIndex++ = 2; *pIndex++ = 6; *pIndex++ = 0;
	*pIndex++ = 0; *pIndex++ = 6; *pIndex++ = 4;
	// back
	*pIndex++ = 1; *pIndex++ = 7; *pIndex++ = 3;
	*pIndex++ = 1; *pIndex++ = 5; *pIndex++ = 7;

	m_IndexBuffer.Unlock();

	return true;
}

void Cuboid::Destroy()
{
	Display::Device()->Destroy(&m_IndexBuffer);
	Display::Device()->Destroy(&m_VertexBuffer);
}

bool Cuboid::IsInside(const Maths::HVec3& vPoint, const Maths::HMat43& mObject, const Maths::HF32& vEpsilon) const
{
	const Maths::HF32  vBoundary	= Maths::Constants<Maths::HF32>::One();
	const Maths::HVec3 vObject		= vPoint * mObject;

	const Maths::Scalar_Bool vX		= vObject.X().LE(vBoundary) && vObject.X().GE(-vBoundary);
	const Maths::Scalar_Bool vY		= vObject.Y().LE(vBoundary) && vObject.Y().GE(-vBoundary);
	const Maths::Scalar_Bool vZ		= vObject.Z().LE(vBoundary) && vObject.Z().GE(-vBoundary);
	
	return (bool) (vX && vY && vZ);
}

Maths::HF32 Cuboid::EvaluateDensityAt(const Maths::HVec3& vPoint, const Maths::HMat43& mObject, const Maths::HVec2& vSettings) const
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
		const Maths::HVec3 vObjectViewVectorRcp(vObjectViewVector.X().Rcp(), vObjectViewVector.Y().Rcp(), vObjectViewVector.Z().Rcp());

		const Maths::HVec3 vPlanes(vObjectViewVector.X().Sign(), vObjectViewVector.Y().Sign(), vObjectViewVector.Z().Sign());
		const Maths::HVec3 vLambdas((vPlanes - vObjectPosition) * vObjectViewVectorRcp);
		const Maths::HF32 vLambda = Maths::Min(vLambdas.X(), Maths::Min(vLambdas.Y(), vLambdas.Z()));
		
		vEntryPosition = vPoint + vLambda * vWorldViewVector;
	}

	const Maths::HVec3 vViewVector = vPoint - vEntryPosition;
	const Maths::HVec3 vNewSettings(vSettings.X(), vSettings.Y(), expf(-vSettings.Y() * vEntryPosition.Y()));

	return GetVolumetricFog(vViewVector, vNewSettings);
}

}; // namespace Fog
}; // namespace DynamicEffect
