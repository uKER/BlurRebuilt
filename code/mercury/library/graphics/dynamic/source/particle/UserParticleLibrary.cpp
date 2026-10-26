#include "stdafx.h"

#include <core/utility/endian.h>

#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
//lint -e766
#include <system/serialise/C_FileBlock.h>

#include "Dynamic/Particle/Tools_Shared/UserShape3DData.h"
#include "Dynamic/Particle/Tools_Shared/UserShape3DVariants.h"
#include "Dynamic/Particle/Tools_Shared/UserParticleContainer.h"
#include "Dynamic/Particle/UserParticleLibrary.h"

namespace DynamicEffect
{
namespace Particle
{
const char* kGlobalParticleFilePath = "particles/particles_global.particleMesh";
}
}

using namespace DynamicEffect::Particle;

bool C_UserParticleLibrary::Initialise()
{
	if (!m_initialised)
	{
		m_initialised = true;
	}
	
	return true;
}

void C_UserParticleLibrary::Shutdown()
{
	if (m_initialised)
	{
		Reset();
		m_initialised = false;
	}
}

bool C_UserParticleLibrary::Open(const char* pFilePath)
{
	bool success = true;
	Sys::I_File* pFile = Display::gFileSystem->Open(
		pFilePath, 
		Sys::E_FileAccess_Read);
	if (!pFile)
	{
		success = false;
		goto exit;
	}

	success = m_userParticleContainer.Deserialise(*pFile);
	if (!success) goto exit;

exit:
	delete pFile;
	return success;
}

void C_UserParticleLibrary::Reset()
{
	m_userParticleContainer.Reset();
}

const C_UserShape3DVariants& C_UserParticleLibrary::GetShape3DVariants(U32 index) const
{
	return m_userParticleContainer.GetShape3DVariantsArray()[index];
}

U32 C_UserParticleLibrary::GetNumShape3DVariants() const
{
	return m_userParticleContainer.GetShape3DVariantsArray().Size();
}

S32 C_UserParticleLibrary::GetShape3DVariantsIndex(const Util::C_String& variantName) const
{
	for (U32 i = 0; i < (U32)m_userParticleContainer.GetShape3DVariantsArray().Size(); i++)
	{
		const C_UserShape3DVariants& variants = m_userParticleContainer.GetShape3DVariantsArray()[i];
		if (variants.GetName().Lower() == variantName.Lower())
			return i;
	}

	return -1;
}
