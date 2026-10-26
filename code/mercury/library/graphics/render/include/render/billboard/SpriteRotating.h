#ifndef RENDER_BILLBOARD_SPRITEROTATING_H
#define RENDER_BILLBOARD_SPRITEROTATING_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Render
{
namespace Billboard
{



/*
 * SpriteRotating
 *
 * Primitive type:		Display::E_PrimitiveType_TriangleList
 * Primitive count:		2
 * Vertex count:		6
 *
 * Triangles:
 *  0, 1, 2
 *  0, 2, 3
 */

class SpriteRotating
{

public:

	SpriteRotating();
	~SpriteRotating();

	void Build(const Maths::HVec3 &vPosition, const Maths::HF32 &fRotation, const Maths::HF32 &fSize);

public:

	// per batch
	Maths::HVec3	m_vForward;
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



INLINE SpriteRotating::SpriteRotating()
{
	const Maths::HMat43 &matCamera = Display::Device()->GetViewSettings3D().GetCameraToWorld();

	Maths::HVec3 vUp = matCamera.AY();
	Maths::HVec3 vRight = matCamera.AX();

	m_vForward = matCamera.AZ();

	m_vTopLeft = vUp - vRight;
	m_vTopRight = vRight + vRight;
	m_vBottomRight = -m_vTopLeft;
	m_vBottomLeft = -m_vTopRight;
}

INLINE SpriteRotating::~SpriteRotating()
{
}

INLINE void SpriteRotating::Build(const Maths::HVec3 &vPosition, const Maths::HF32 &fRotation, const Maths::HF32 &fSize)
{
	Maths::C_Quaternion qRotate(m_vForward, fRotation);
	Maths::HMat33 mRotate(qRotate);

	Maths::HVec3 vBottomLeft(m_vBottomLeft * mRotate);
	Maths::HVec3 vBottomRight(m_vBottomRight * mRotate);
	Maths::HVec3 vTopLeft(m_vTopLeft * mRotate);
	Maths::HVec3 vTopRight(m_vTopRight * mRotate);

	m_vPosition0 = vBottomLeft * fSize + vPosition;
	m_vPosition1 = vBottomRight * fSize + vPosition;
	m_vPosition2 = vTopRight * fSize + vPosition;
	m_vPosition3 = vTopLeft * fSize + vPosition;
}



}; // namespace Billboard
}; // namespace Render



#endif
