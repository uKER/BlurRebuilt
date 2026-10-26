#include <nexus_render/stdafx.h>

#include <nexus_render/D3dImageLoader.h>
#include <nexus_render/D3dTextureManager.h>

#include <nexus_render/EffectConstants.h>
#include <nexus_render/MeshBasicEffectConstants.h>
#include <core/utility/StringProc.h>

using namespace NexusRender;

MeshBasicEffectConstants::MeshBasicEffectConstants() : EffectConstants()
{
	OneTimeInit();
}

MeshBasicEffectConstants::MeshBasicEffectConstants(const MeshBasicEffectConstants& rhs)
{
	*this = rhs;
}

void MeshBasicEffectConstants::OneTimeInit()
{
}

MeshBasicEffectConstants::~MeshBasicEffectConstants()
{
}

const MeshBasicEffectConstants& MeshBasicEffectConstants::operator=(const MeshBasicEffectConstants& rhs)
{
	EffectConstants::operator =(rhs);

	return *this;
}

Tu::Status MeshBasicEffectConstants::Build(const Aftk::C_MaterialNode* pAftkMaterialNode)
{
	return Tu::Status::SUCCESS;
}

U32 MeshBasicEffectConstants::GetHash() const
{
	return 0;
}

bool MeshBasicEffectConstants::operator==(const EffectConstants& effectConstants) const
{
	if (GetEffectType() != effectConstants.GetEffectType())
		return false;

	return true;
}


