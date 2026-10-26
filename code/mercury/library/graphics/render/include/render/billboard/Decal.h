#ifndef RENDER_BILLBOARD_DECAL_H
#define RENDER_BILLBOARD_DECAL_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Render
{
namespace Billboard
{



/*
 * Decal
 *
 * Primitive type:		Display::E_PrimitiveType_TriangleList
 * Primitive count:		2
 * Vertex count:		6
 *
 * Triangles:
 *  0, 1, 2
 *  0, 2, 3
 */

class Decal
{

public:

	Decal();
	~Decal();

	void Build(const Maths::HVec3 &vPosition, const Maths::HMat33 &mRotate, const Maths::HVec3 &vSize);

public:

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



INLINE Decal::Decal()
{
}

INLINE Decal::~Decal()
{
}

INLINE void Decal::Build(const Maths::HVec3 &vPosition, const Maths::HMat33 &mRotate, const Maths::HVec3 &vSize)
{
	Maths::HVec3 vVertex0(-1.0f, 0.0f, -1.0f);
	Maths::HVec3 vVertex1(1.0f, 0.0f, -1.0f);
	Maths::HVec3 vVertex2(1.0f, 0.0f, 1.0f);
	Maths::HVec3 vVertex3(-1.0f, 0.0f, 1.0f);

	m_vPosition0 = (vVertex0 * vSize) * mRotate + vPosition;
	m_vPosition1 = (vVertex1 * vSize) * mRotate + vPosition;
	m_vPosition2 = (vVertex2 * vSize) * mRotate + vPosition;
	m_vPosition3 = (vVertex3 * vSize) * mRotate + vPosition;
}



}; // namespace Billboard
}; // namespace Render



#endif // RENDER_BILLBOARD_DECAL_H
