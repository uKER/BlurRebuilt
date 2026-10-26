#ifndef RENDER_BILLBOARD_BEAMCAPPED_H
#define RENDER_BILLBOARD_BEAMCAPPED_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Render
{
namespace Billboard
{



/*
 * BeamCapped
 *
 * Primitive type:		Display::E_PrimitiveType_TriangleList
 * Primitive count:		4
 * Vertex count:		12
 *
 * Triangles:
 *  0, Back, 1
 *  0, 1, 2
 *  2, 3, 0
 *  2, Front, 3
 */

class BeamCapped
{

public:

	BeamCapped();
	~BeamCapped();

	void Build(const Maths::HVec3 &vBegin, const Maths::HVec3 &vEnd, const Maths::HF32 &fSize);

public:

	// per batch
	Maths::HVec3	m_vViewPosition;
	Maths::HVec3	m_vViewDirectionUp;
	Maths::HVec3	m_vViewDirectionRight;

	// per particle
	Maths::HVec3	m_vPosition0;
	Maths::HVec3	m_vPosition1;
	Maths::HVec3	m_vPosition2;
	Maths::HVec3	m_vPosition3;
	Maths::HVec3	m_vPositionFront;
	Maths::HVec3	m_vPositionBack;

public:

	// per shape
	static const U32 m_iTexCoord0 = 0;
	static const U32 m_iTexCoord1 = 2;
	static const U32 m_iTexCoord2 = 2;
	static const U32 m_iTexCoord3 = 0;
	static const U32 m_iTexCoordFront = 1;
	static const U32 m_iTexCoordBack = 3;

};



INLINE BeamCapped::BeamCapped()
{
	const Maths::HMat43 &matCamera = Display::Device()->GetViewSettings3D().GetCameraToWorld();

	m_vViewPosition = Maths::HVec3(matCamera.AW());
	m_vViewDirectionUp = Maths::HVec3(matCamera.AY());
	m_vViewDirectionRight = Maths::HVec3(matCamera.AX());
}

INLINE BeamCapped::~BeamCapped()
{
}

INLINE void BeamCapped::Build(const Maths::HVec3 &vBegin, const Maths::HVec3 &vEnd, const Maths::HF32 &fSize)
{
	Maths::HVec3	vCentre(vBegin.Lerp(vEnd, Maths::Constants<Maths::HF32>::Half()));
	Maths::C_Normal	vDirection(Normalise(vEnd - vBegin));
	Maths::C_Normal	vToView(Normalise(m_vViewPosition - vCentre));
	Maths::C_Normal	vCross(Normalise(vToView ^ vDirection));
	Maths::HVec3	vSize(vCross * fSize);
	
	m_vPosition0 = vBegin + vSize;
	m_vPosition1 = vBegin - vSize;
	m_vPosition2 = vEnd - vSize;
	m_vPosition3 = vEnd + vSize;

	Maths::HVec3 vPosition4((m_vPosition0 + m_vPosition1) * Maths::Constants<Maths::HF32>::Half());
	Maths::HVec3 vPosition5((m_vPosition2 + m_vPosition3) * Maths::Constants<Maths::HF32>::Half());
	Maths::HVec3 vVectorFront(vPosition5 - m_vPosition2);
	Maths::HVec3 vVectorBack(vPosition4 - m_vPosition0);

	Maths::HVec3 vFrontDotRight(m_vViewDirectionRight | vVectorFront);
	Maths::HVec3 vFrontDotUp(m_vViewDirectionUp | vVectorFront);
	Maths::HVec3 vBackDotRight(m_vViewDirectionRight | vVectorBack);
	Maths::HVec3 vBackDotUp(m_vViewDirectionUp | vVectorBack);
	
	m_vPositionFront = vPosition5;
	m_vPositionBack = vPosition4;
	m_vPositionFront += m_vViewDirectionRight * -vFrontDotUp;
	m_vPositionBack += m_vViewDirectionRight * -vBackDotUp;
	m_vPositionFront += m_vViewDirectionUp * vFrontDotRight;
	m_vPositionBack += m_vViewDirectionUp * vBackDotRight;
}



}; // namespace Billboard
}; // namespace Render



#endif
