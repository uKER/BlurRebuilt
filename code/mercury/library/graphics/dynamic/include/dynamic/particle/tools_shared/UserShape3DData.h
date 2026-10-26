#ifndef RENDER_DYNAMICEFFECT_PARTICLE_USERSHAPE3DDATA_H
#define RENDER_DYNAMICEFFECT_PARTICLE_USERSHAPE3DDATA_H

#ifdef _MSC_VER
#pragma once
#endif


namespace DynamicEffect
{
namespace Particle
{
	class ALIGN16 C_UserShape3DData
	{
	public:
		typedef MathsStorage::Float2 Vector2F32;
		typedef MathsStorage::Float3 Vector3F32;

		struct C_Vertex
		{
			Vector3F32 position;
			Vector3F32 normal;
			Vector2F32 uv0;
			Vector3F32 tan0;
			Vector3F32 bin0;

			template<typename S>
			INLINE void Serialise(S& s);
		};

		typedef Btl::Vector<C_Vertex> VertexArray;

		INLINE void SetNumVertices(U32 numVertices);

		INLINE VertexArray& GetVertexArray();

		INLINE const VertexArray& GetVertexArray() const;

		INLINE void Normalise();

		template<typename S>
		INLINE void Serialise(S& s);

		template<typename S>
		INLINE void Deserialise(S& s);

	private:
		VertexArray		m_vertices;
	};

	#include "UserShape3DData.inl"
}
}

#endif
