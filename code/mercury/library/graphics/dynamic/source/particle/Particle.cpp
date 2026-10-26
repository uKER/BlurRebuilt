#include "stdafx.h"

#include "Dynamic/Particle/Particle.h"

namespace DynamicEffect
{
namespace Particle
{

void C_ParticleFormat::AddElement(E_ParticleElement type)
{
	U32 offset = 0;

	if(m_Elements.Size() > 0)
		offset = m_Elements[m_Elements.Size() - 1].offset +  ParticleElementSize(m_Elements[m_Elements.Size() - 1].type);

	m_Elements.PushBack();

	ParticleElement& element = m_Elements.Back();

	element.type = type;
	element.offset = offset;

	m_Size = offset + ParticleElementSize(type);
}

S32 C_ParticleFormat::FindElement(E_ParticleElement element)
{
	for(S32 i = 0; i < m_Elements.Size(); ++i)
	{
		if(m_Elements[i].type == element)
			return i;
	}

	return -1;
}



C_ParticleFormat	C_Particle3D::format;
C_ParticleFormat	C_Particle3D_R1D::format;
C_ParticleFormat	C_Particle3D_R1D_CP::format;
C_ParticleFormat	C_Particle3D_R3D::format;
C_ParticleFormat	C_Particle3D_RV3D::format;
C_ParticleFormat	C_Particle3D_RV3D_CP::format;
C_ParticleFormat	C_Particle3D_R1D_P::format;
C_ParticleFormat	C_Particle3D_P::format;
C_ParticleFormat	C_Particle3D_CP::format;

} // Particle
} // DynamicEffect
