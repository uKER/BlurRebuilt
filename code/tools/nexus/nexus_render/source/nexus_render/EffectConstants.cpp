#include <nexus_render/stdafx.h>
#include <nexus_render/EffectConstants.h>
#include <nexus_render/D3dShaderManager.h>

using namespace NexusRender;

EffectConstants::EffectConstants()
{
}

EffectConstants::EffectConstants(const EffectConstants& rhs)
{
	*this = rhs;
}

EffectConstants::~EffectConstants()
{
}

const EffectConstants& EffectConstants::operator=(const EffectConstants& rhs)
{
	return *this;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif