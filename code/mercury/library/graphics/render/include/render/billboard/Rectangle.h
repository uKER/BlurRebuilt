#ifndef RENDER_BILLBOARD_RECTANGLE_H
#define RENDER_BILLBOARD_RECTANGLE_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Render
{
namespace Billboard
{



/*
 * Sprite
 *
 * Primitive type:		Display::E_PrimitiveType_TriangleList
 * Primitive count:		2
 * Vertex count:		6
 *
 * Triangles:
 *  0, 1, 2
 *  0, 2, 3
 */

class Rectangle
{

public:

	Rectangle();
	~Rectangle();

	void Build(const Maths::HVec3 &vPosition, const Maths::HVec2 &vSize);

public:

	// per batch
	Maths::HVec3	m_vUp;
	Maths::HVec3	m_vRight;

	// per particle
	Maths::HVec3	m_vPosition0;
	Maths::HVec3	m_vPosition1;
	Maths::HVec3	m_vPosition2;
	Maths::HVec3	m_vPosition3;

public:

	// per shape
	static const U32	m_iTexCoord0 = 0;
	static const U32	m_iTexCoord1 = 1;
	static const U32	m_iTexCoord2 = 2;
	static const U32	m_iTexCoord3 = 3;

};



INLINE Rectangle::Rectangle()
{
	const Maths::HMat43 &matCamera = Display::Device()->GetViewSettings3D().GetCameraToWorld();

	m_vUp = Maths::HVec3(matCamera.AY());
	m_vRight = Maths::HVec3(matCamera.AX());
}

INLINE Rectangle::~Rectangle()
{
}

INLINE void Rectangle::Build(const Maths::HVec3 &vPosition, const Maths::HVec2 &vSize)
{
	Maths::HVec3 vUp = m_vUp * vSize.Y();
	Maths::HVec3 vRight = m_vRight * vSize.X();

	m_vPosition0 = -vUp - vRight + vPosition;
	m_vPosition1 = -vUp + vRight + vPosition;
	m_vPosition2 = vUp + vRight + vPosition;
	m_vPosition3 = vUp - vRight + vPosition;
}



}; // namespace Billboard
}; // namespace Render



#endif // RENDER_BILLBOARD_RECTANGLE_H
