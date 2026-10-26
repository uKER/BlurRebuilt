#ifndef RENDER_BILLBOARD_SPRITEDIRECTION_H
#define RENDER_BILLBOARD_SPRITEDIRECTION_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Render
{
namespace Billboard
{



/*
 * SpriteDirection
 *
 * Primitive type:		Display::E_PrimitiveType_TriangleList
 * Primitive count:		2
 * Vertex count:		6
 *
 * Triangles:
 *  0, 1, 2
 *  0, 2, 3
 */

class SpriteDirection
{

public:

	SpriteDirection();
	~SpriteDirection();

	void Build(const Maths::HVec3 &vPosition, const Maths::HVec3 &vDirection, const Maths::HVec2 &vSize);

public:

	// per batch
	Maths::HVec3	m_vViewPosition;

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



INLINE SpriteDirection::SpriteDirection()
{
	const Maths::HMat43 &matCamera = Display::Device()->GetViewSettings3D().GetCameraToWorld();

	m_vViewPosition = Maths::HVec3(matCamera.AW());
}

INLINE SpriteDirection::~SpriteDirection()
{
}

INLINE void SpriteDirection::Build(const Maths::HVec3 &vPosition, const Maths::HVec3 &vDirection, const Maths::HVec2 &vSize)
{
	Maths::HVec3 vToView(m_vViewPosition - vPosition);
	
	Maths::HVec3 vHeight(vDirection);
	Maths::HVec3 vWidth(vToView ^ vDirection);
	vHeight.Normalise();
	vWidth.Normalise();
	vHeight *= vSize.Y();
	vWidth *= vSize.X();
	Maths::HVec3 vHeightNeg(-vHeight);
	Maths::HVec3 vWidthNeg(-vWidth);

	Maths::HVec3 vFront(vPosition + vHeight);
	Maths::HVec3 vBack(vPosition + vHeightNeg);
	
	m_vPosition0 = vBack + vWidth;
	m_vPosition1 = vBack + vWidthNeg;
	m_vPosition2 = vFront + vWidthNeg;
	m_vPosition3 = vFront + vWidth;
}



}; // namespace Billboard
}; // namespace Render



#endif
