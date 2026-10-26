#ifndef RENDER_DYNAMICEFFECT_PARTICLE_USERSHAPE3DVARIANTS_H
#define RENDER_DYNAMICEFFECT_PARTICLE_USERSHAPE3DVARIANTS_H

#ifdef _MSC_VER
#pragma once
#endif


namespace DynamicEffect
{
namespace Particle
{
	class ALIGN16 C_UserShape3DVariants
	{
	public:
		typedef Btl::Vector<C_UserShape3DData>		Shape3DArray;

		INLINE C_UserShape3DVariants();

		INLINE const Util::C_String& GetName() const;

		INLINE U32 GetNumVertices() const;

		INLINE U32 GetNumPrimitives() const;

		INLINE F32 GetScale() const;
	
		INLINE const Shape3DArray& GetShape3DArray() const;

		INLINE Shape3DArray& GetShape3DArray();

		INLINE void SetName(const Util::C_String& name);

		INLINE void SetNumVertices(U32 numVertices);
	
	#ifdef PLATFORM_PC
		INLINE void SetScale(F32 scale);
	#endif

		template<typename S>
		INLINE void Serialise(S& s);

		template<typename S>
		INLINE void Deserialise(S& s);

	private:
		Util::C_String m_name;
		U32 m_numVertices;

		MathsStorage::Float4A16 m_scale;

		Shape3DArray	m_userShape3DData;
	};

	#include "UserShape3DVariants.inl"
}
}

#endif
