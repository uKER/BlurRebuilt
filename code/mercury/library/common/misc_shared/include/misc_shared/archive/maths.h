#ifndef SYSTEM_ARCHIVE_MATHS_H
#define SYSTEM_ARCHIVE_MATHS_H
#ifdef _MSC_VER
#pragma once
#endif

#include "core/maths.h"

namespace Arch
{

template<typename Archive>
void Serialise(Archive &ar, Maths::RGBA& t)
{
	ar << (U8) t.R();
	ar << (U8) t.G();
	ar << (U8) t.B();
	ar << (U8) t.A();
}

template <class Archive>
void Deserialise(Archive &ar, Maths::RGBA& t)
{
	U8 r, g, b, a;
	ar >> r >> g >> b >> a;
	t = Maths::RGBA(r, g, b, a);
}

template <typename Archive>
inline void Serialise(Archive& ar, MathsStorage::Matrix44& t)
{
	for (int i = 0; i < 16; i++)
		ar & t.v[i];
}

AUTO_DESERIALISE_FREE(MathsStorage::Matrix44)

}

SERIALISATION_PRIMITIVE(Maths::RGBA, "Maths::RGBA")

SERIALISATION_PRIMITIVE(Maths::HF32, "Maths::HF32")
SERIALISATION_PRIMITIVE(Maths::HVec4, "Maths::HVec4")
SERIALISATION_PRIMITIVE(Maths::HVec3, "Maths::HVec3")
SERIALISATION_PRIMITIVE(Maths::HVec2, "Maths::HVec2")
SERIALISATION_PRIMITIVE(Maths::C_Vertex, "Maths::C_Vertex")
SERIALISATION_PRIMITIVE(Maths::C_Vector, "Maths::C_Vector")
SERIALISATION_PRIMITIVE(Maths::C_Normal, "Maths::C_Normal")
SERIALISATION_PRIMITIVE(Maths::HMat33, "Maths::HMat33")
SERIALISATION_PRIMITIVE(Maths::HMat43, "Maths::HMat43")

SERIALISATION_PRIMITIVE(MathsStorage::Float2, "MathsStorage::Float2")
SERIALISATION_PRIMITIVE(MathsStorage::Float3, "MathsStorage::Float3")
SERIALISATION_PRIMITIVE(MathsStorage::Float4, "MathsStorage::Float4")
SERIALISATION_PRIMITIVE(MathsStorage::Matrix44, "MathsStorage::Matrix44")

#endif
