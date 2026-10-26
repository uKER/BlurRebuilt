#ifndef RENDER_DYNAMICEFFECT_NODE_H
#define RENDER_DYNAMICEFFECT_NODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Defines.h"

namespace DynamicEffect
{

class C_Node
{
public:
	C_Node() : m_vPosition(0.0f, 0.0f, 0.0f, 1.0f), m_vRotation(0.0f, 0.0f, 0.0f, 1.0f) {}
	C_Node(const Maths::C_QuatTrans& quatTrans);
	C_Node(const MathsStorage::Float4A16& vPosition, const MathsStorage::Float4A16& vRotation) : m_vPosition(vPosition), m_vRotation(vRotation) {}
	virtual ~C_Node() {};

	void					SetRotation(const MathsStorage::Float4A16& vRotation);
	void					SetPosition(const MathsStorage::Float4A16& vPosition);
	void					SetPosition(const MathsStorage::Float3& vPosition);
	
	void					SetRotation(const Maths::C_Quaternion& vRotation);
	void					SetPosition(const Maths::HVec3& vPosition);
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
	void					SetRotation(const XMVECTOR vRotation);
	void					SetPosition(const XMVECTOR vPosition);
#endif

	const MathsStorage::Float4A16&	GetPosition() const { return m_vPosition; }
	const MathsStorage::Float4A16&	GetRotation() const { return m_vRotation; }

	// HVec QuatTrans cast
	operator                const Maths::C_QuatTrans () const;

	MathsStorage::Float4A16	m_vPosition;
	MathsStorage::Float4A16	m_vRotation;
};

FORCE_INLINE void C_Node::SetPosition(const Maths::HVec3& vPosition)
{
	Maths::Store(vPosition, &m_vPosition);
}

FORCE_INLINE void C_Node::SetRotation(const Maths::C_Quaternion& vRotation)
{
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
	Maths::Store(!vRotation, &m_vRotation);
#else
	Maths::Store(vRotation, &m_vRotation);
#endif
}

FORCE_INLINE void C_Node::SetPosition(const MathsStorage::Float4A16& vPosition)
{
	m_vPosition = vPosition;
}

FORCE_INLINE void C_Node::SetPosition(const MathsStorage::Float3& vPosition)
{
	m_vPosition.x = vPosition.x;
	m_vPosition.y = vPosition.y;
	m_vPosition.z = vPosition.z;
	m_vPosition.w = 1.0f;
}

FORCE_INLINE void C_Node::SetRotation(const MathsStorage::Float4A16& vRotation)
{
	m_vRotation = vRotation;
}

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
FORCE_INLINE void C_Node::SetPosition(const XMVECTOR vPosition)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vPosition, vPosition);
}

FORCE_INLINE void C_Node::SetRotation(const XMVECTOR vRotation)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vRotation, vRotation);
}
#endif

FORCE_INLINE C_Node::C_Node(const Maths::C_QuatTrans& quatTrans)
{
	SetPosition(quatTrans.Translation());
	SetRotation(quatTrans.Quaternion());
}

FORCE_INLINE C_Node::operator const Maths::C_QuatTrans () const
{
	Maths::C_Quaternion qRotation(Maths::Load<Maths::C_Quaternion>(&m_vRotation));
	Maths::HVec3 vTranslation(Maths::Load<Maths::HVec3>(&m_vPosition));
	// undo conjugation from XMVECTOR
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
	Maths::C_QuatTrans transform(!qRotation, vTranslation);
#else
	Maths::C_QuatTrans transform(qRotation, vTranslation);
#endif

	return transform;
}

}

#endif // RENDER_DYNAMICEFFECT_NODE_H
