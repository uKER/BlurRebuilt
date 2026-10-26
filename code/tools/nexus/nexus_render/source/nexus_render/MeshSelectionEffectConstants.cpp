#include <nexus_render/stdafx.h>

#include <nexus_render/D3dImageLoader.h>
#include <nexus_render/D3dTextureManager.h>

#include <nexus_render/EffectConstants.h>
#include <nexus_render/MeshSelectionEffectConstants.h>
#include <core/utility/StringProc.h>

using namespace NexusRender;

static const U32 COLOUR_TYPE = 0;
static const U32 LIGHTMAP_TYPE = 1;

MeshSelectionEffectConstants::MeshSelectionEffectConstants() : EffectConstants()
{
	OneTimeInit();
}

MeshSelectionEffectConstants::MeshSelectionEffectConstants(const MeshSelectionEffectConstants& rhs)
{
	*this = rhs;
}

void MeshSelectionEffectConstants::OneTimeInit()
{
}

MeshSelectionEffectConstants::~MeshSelectionEffectConstants()
{
}

const MeshSelectionEffectConstants& MeshSelectionEffectConstants::operator=(const MeshSelectionEffectConstants& rhs)
{
	EffectConstants::operator =(rhs);
	return *this;
}

Tu::Status MeshSelectionEffectConstants::Build()
{
	return Tu::Status::SUCCESS;
}

U32 MeshSelectionEffectConstants::GetHash() const
{
	return 0;
}

bool MeshSelectionEffectConstants::operator==(const EffectConstants& effectConstants) const
{
	if (GetEffectType() != effectConstants.GetEffectType())
		return false;

	return true;
}