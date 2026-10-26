#include "stdafx.h"

#include "ParticleEmitter.h"

namespace RecordGraph
{

C_ParticleEmitterNode::C_ParticleEmitterNode(LfStruct* pStruct) : C_StructNode(pStruct)
{
	m_Colour = wxColour(128, 128, 255);
	m_Size = wxSize(64, 64);
}

} // RecordGraph
