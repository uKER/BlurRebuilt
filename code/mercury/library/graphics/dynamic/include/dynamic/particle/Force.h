#ifndef RENDER_DYNAMICEFFECT_PARTICLE_FORCE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_FORCE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{
namespace Particle
{

enum E_ForceType
{
	E_ForceType_Point,
	E_ForceType_Vector,
};

class C_ParticleManager;

class C_Force
{
public:
	C_Force() : m_eType(E_ForceType_Point), m_pRenderer(NULL) {};
	C_Force(C_ParticleManager* pRenderer);
	virtual ~C_Force();

	void			Display() {};
	void			SetPosition(F32 x, F32 y, F32 z);
	void			SetVelocity(F32 x, F32 y, F32 z);
	void			SetRadius(F32 radius);
	void			SetStrength(F32 strength);
	void			SetType(E_ForceType eType) { m_eType = eType; }

	MathsStorage::Float3A16	m_vPosition;
	MathsStorage::Float3A16	m_vVelocity;
	MathsStorage::Float4A16	m_vRadius;
	MathsStorage::Float4A16	m_vRadiusRcp;			
	MathsStorage::Float4A16	m_vRadiusSq;
	MathsStorage::Float4A16	m_vMin;
	MathsStorage::Float4A16	m_vMax;
	MathsStorage::Float4A16	m_vStrength;

	E_ForceType				m_eType;
	friend class			C_ParticleManager;

private:
	
	C_ParticleManager*		m_pRenderer;
};

typedef Ut::UtSet<C_Force*>									Forces;
typedef	Ut::UtVector<Ut::DynamicVector<const C_Force*> >	ConstForces;

} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_FORCE_H
