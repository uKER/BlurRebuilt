#include "stdafx.h"

#include "Dynamic/Particle/Manager.h"
#include "Dynamic/Particle/Force.h"

using namespace DynamicEffect::Particle;

C_Force::C_Force(C_ParticleManager* pRenderer) : m_eType(E_ForceType_Point)
{
	m_pRenderer = pRenderer;
	m_pRenderer->AddForce(this);
}

C_Force::~C_Force()
{
	assert(m_pRenderer);
	m_pRenderer->RemoveForce(this);
}

void C_Force::SetPosition(F32 x, F32 y, F32 z)
{
	m_vPosition = MathsStorage::Float3A16(x, y, z);
}

void C_Force::SetVelocity(F32 x, F32 y, F32 z)
{
	m_vVelocity = MathsStorage::Float3A16(x, y, z);
}

void C_Force::SetRadius(F32 radius)
{
	m_vRadius = MathsStorage::Float4A16(radius, radius, radius, radius);
	m_vRadiusRcp = MathsStorage::Float4A16(1.f/radius, 1.f/radius, 1.f/radius, 1.f/radius);
	m_vRadiusSq = MathsStorage::Float4A16(radius * radius, radius * radius, radius * radius, radius * radius);

	m_vMin = MathsStorage::Float4A16(m_vPosition.x - radius, m_vPosition.y - radius, m_vPosition.z - radius, 1.0f);
	m_vMax = MathsStorage::Float4A16(m_vPosition.x + radius, m_vPosition.y + radius, m_vPosition.z + radius, 1.0f);
}

void C_Force::SetStrength(F32 strength)
{
	m_vStrength = MathsStorage::Float4A16(strength, strength, strength, strength);
}



