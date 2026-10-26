#ifndef RD_EFFECT_H
#define RD_EFFECT_H
#ifdef _MSC_VER
#pragma once
#endif

#include "maths.h"
#include "material.h"
#include "resourceblock.h"

namespace RenderingData
{
	// Defines parameters for materials.
	class Effect
	{
	public:
		Effect();

		// The name should uniquely identify the effect
		// so that the game can use it to set the right shaders.
		// (does not break the exporter if not unique)
		void SetName(const Btl::String& s);
		const Btl::String& GetName() const;

		FRIEND_SERIALISE(Effect);

	protected:
		Btl::String m_name;
	};

	#include "noninterface/effect.inl"
}

#endif

