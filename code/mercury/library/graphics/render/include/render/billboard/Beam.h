#ifndef RENDER_BILLBOARD_BEAM_H
#define RENDER_BILLBOARD_BEAM_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Render
{
namespace Billboard
{



/*
 * Beam
 *
 * Primitive type:		Display::E_PrimitiveType_TriangleList
 * Primitive count:		2
 * Vertex count:		6
 *
 * Triangles:
 *  0, 1, 2
 *  0, 2, 3
 */

class Beam
{

public:

	Beam();
	~Beam();

	void Build(const Maths::HVec3 &vBegin, const Maths::HVec3 &vEnd, const Maths::HF32 &fWidth);

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



INLINE Beam::Beam()
{
	const Maths::HMat43 &matCamera = Display::Device()->GetViewSettings3D().GetCameraToWorld();

	m_vViewPosition = Maths::HVec3(matCamera.AW());
}

INLINE Beam::~Beam()
{
}

INLINE void Beam::Build(const Maths::HVec3 &vBegin, const Maths::HVec3 &vEnd, const Maths::HF32 &fWidth)
{
	Maths::HVec3	vEyeToBegin(vBegin - m_vViewPosition);
	Maths::HVec3	vEyeToEnd(vEnd - m_vViewPosition);
	Maths::C_Normal	vRight(vEyeToBegin ^ vEyeToEnd);

	m_vPosition0 = vBegin - vRight * fWidth;
	m_vPosition1 = vBegin + vRight * fWidth;
	m_vPosition2 = vEnd + vRight * fWidth;
	m_vPosition3 = vEnd - vRight * fWidth;
}



}; // namespace Billboard
}; // namespace Render



#endif
