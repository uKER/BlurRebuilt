#ifndef RENDER_DYNAMICEFFECT_PARTICLE_USER_PARTICLE_CONTAINER_H
#define RENDER_DYNAMICEFFECT_PARTICLE_USER_PARTICLE_CONTAINER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Dynamic/Particle/Tools_shared/UserShape3DVariants.h>

namespace DynamicEffect
{
namespace Particle
{
	class C_UserParticleContainer
	{
	public:
		typedef Btl::Vector<DynamicEffect::Particle::C_UserShape3DVariants>	Shape3DVariantsArray;

		INLINE C_UserParticleContainer();

		INLINE Shape3DVariantsArray& GetShape3DVariantsArray();

		INLINE const Shape3DVariantsArray& GetShape3DVariantsArray() const;

		INLINE bool Serialise(Sys::I_File& file) const;

		INLINE bool Deserialise(Sys::I_File& file);

		INLINE void Reset();

	private:
		Shape3DVariantsArray m_userShape3DVariants;

		const char* m_particlePackageBlock;
		const char* m_shape3DVariantsBlock;

		static const U32 m_currentFileVersion = 102;
	};

	#include "UserParticleContainer.inl"
}
}

#endif
