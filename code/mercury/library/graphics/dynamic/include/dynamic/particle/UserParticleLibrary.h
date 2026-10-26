#ifndef RENDER_DYNAMICEFFECT_PARTICLE_USERPARTICLELIBRARY_H
#define RENDER_DYNAMICEFFECT_PARTICLE_USERPARTICLELIBRARY_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <core\utility\Singleton.h>

#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>
//User particle mesh
#include <dynamic/particle/tools_shared/UserShape3DData.h>
#include <dynamic/particle/tools_shared/UserShape3DVariants.h>
#include <dynamic/particle/tools_shared/UserParticleContainer.h>

namespace DynamicEffect
{
namespace Particle
{

extern const char* kGlobalParticleFilePath;

class C_UserParticleLibrary : public Ut::Singleton<C_UserParticleLibrary, Ut::CreateStatic>
{
public:
	bool	Initialise();
	void	Shutdown();
	bool	Open(const char* pFilePath);
	void	Reset();

	S32		GetShape3DVariantsIndex(const Util::C_String& variantName) const;
	const C_UserShape3DVariants& GetShape3DVariants(U32 index) const;
	U32		GetNumShape3DVariants() const;

private:
	bool						m_initialised;
	C_UserParticleContainer		m_userParticleContainer;
};

}
}

#endif
