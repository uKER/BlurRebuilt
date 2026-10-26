#ifndef RENDER_DYNAMICEFFECT_LIGHT_H
#define RENDER_DYNAMICEFFECT_LIGHT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{

// [TODO SCENEGRAPH] kill and merge
class C_Light
{
public:
	C_Light() {}
	virtual ~C_Light() {};

	void					SetPosition(const MathsStorage::Float4A16& vPosition);
	void					SetColour(const MathsStorage::Float4A16& vColour);
	void					SetRadius(const MathsStorage::Float4A16& vRadius);
	void					SetIntensity(const MathsStorage::Float4A16& vIntensity);

	void					SetPriority(const U32 iProirity);
	
	void					SetPosition(const Maths::HVec3& vPosition);
	void					SetColour(const Maths::C_RGBA_F32& vColour);
	void					SetRadius(const Maths::HF32& fRadius);
	void					SetIntensity(const Maths::HF32& fIntensity);
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
	void					SetPosition(const XMVECTOR vPosition);
	void					SetColour(const XMVECTOR vColour);
	void					SetRadius(const XMVECTOR vRadius);
	void					SetIntensity(const XMVECTOR vIntensity);
#endif

	const MathsStorage::Float4A16&	GetPosition() const { return m_vPosition; }
	const MathsStorage::Float4A16&	GetColour() const { return m_vColour; }
	const MathsStorage::Float4A16&	GetRadius() const { return m_vRadius; }
	const MathsStorage::Float4A16&	GetIntensity() const { return m_vIntensity; }
	const U32						GetPriority() const { return m_iPriority; }

	Maths::C_Sphere					GetBoundingSphere() const;
	
protected:
	MathsStorage::Float4A16	m_vPosition;
	MathsStorage::Float4A16 m_vColour;
	MathsStorage::Float4A16	m_vRadius;
	MathsStorage::Float4A16	m_vIntensity;
	U32						m_iPriority;
};

FORCE_INLINE void C_Light::SetPosition(const Maths::HVec3& vPosition)
{
	Maths::Store(vPosition, &m_vPosition);
}

FORCE_INLINE void C_Light::SetColour(const Maths::C_RGBA_F32& vColour)
{
	Maths::Store(vColour, &m_vColour);
}

FORCE_INLINE void C_Light::SetRadius(const Maths::HF32& fRadius)
{
	m_vRadius.x = fRadius;
}

FORCE_INLINE void C_Light::SetIntensity(const Maths::HF32& fIntensity)
{
	m_vIntensity.x = fIntensity;
}

FORCE_INLINE void C_Light::SetPosition(const MathsStorage::Float4A16& vPosition)
{
	m_vPosition = vPosition;
}

FORCE_INLINE void C_Light::SetColour(const MathsStorage::Float4A16& vColour)
{
	m_vColour = vColour;
}

FORCE_INLINE void C_Light::SetRadius(const MathsStorage::Float4A16& vRadius)
{
	m_vRadius = vRadius;
}

FORCE_INLINE void C_Light::SetIntensity(const MathsStorage::Float4A16& vIntensity)
{
	m_vIntensity = vIntensity;
}

FORCE_INLINE void C_Light::SetPriority(const U32 iPriority)
{
	m_iPriority = iPriority;
}

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
FORCE_INLINE void C_Light::SetPosition(const XMVECTOR vPosition)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vPosition, vPosition);
}

FORCE_INLINE void C_Light::SetColour(const XMVECTOR vColour)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vColour, vColour);
}

FORCE_INLINE void C_Light::SetRadius(const XMVECTOR vRadius)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vRadius, vRadius);
}

FORCE_INLINE void C_Light::SetIntensity(const XMVECTOR vIntensity)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vIntensity, vIntensity);
}
#endif

FORCE_INLINE Maths::C_Sphere C_Light::GetBoundingSphere() const
{
	Maths::HVec3 vCentre = Maths::Load<Maths::HVec3>(&m_vPosition);
	Maths::HF32 fRadius = m_vRadius.x;

	return Maths::C_Sphere(vCentre, fRadius);
}


}

#endif // RENDER_DYNAMICEFFECT_NODE_H
