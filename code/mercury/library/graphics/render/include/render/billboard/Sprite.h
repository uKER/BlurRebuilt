#ifndef RENDER_BILLBOARD_SPRITE_H
#define RENDER_BILLBOARD_SPRITE_H

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

class Sprite
{

public:

	Sprite();
	~Sprite();

	void Build(const Maths::HVec3 &vPosition, const Maths::HF32 &fSize);

public:

	// per batch
	Maths::HVec3	m_vTopLeft;
	Maths::HVec3	m_vTopRight;
	Maths::HVec3	m_vBottomLeft;
	Maths::HVec3	m_vBottomRight;

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



INLINE Sprite::Sprite()
{
	const Maths::HMat43 &matCamera = Display::Device()->GetViewSettings3D().GetCameraToWorld();

	Maths::HVec3 vUp = Maths::HVec3(matCamera.AY());
	Maths::HVec3 vRight = Maths::HVec3(matCamera.AX());

	m_vTopLeft = vUp - vRight;
	m_vTopRight = vUp + vRight;
	m_vBottomRight = -m_vTopLeft;
	m_vBottomLeft = -m_vTopRight;
}

INLINE Sprite::~Sprite()
{
}

INLINE void Sprite::Build(const Maths::HVec3 &vPosition, const Maths::HF32 &fSize)
{
	m_vPosition0 = m_vBottomLeft * fSize + vPosition;
	m_vPosition1 = m_vBottomRight * fSize + vPosition;
	m_vPosition2 = m_vTopRight * fSize + vPosition;
	m_vPosition3 = m_vTopLeft * fSize + vPosition;
}



}; // namespace Billboard
}; // namespace Render



#endif // RENDER_BILLBOARD_SPRITE_H
