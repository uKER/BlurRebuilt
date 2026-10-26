#ifndef ARCHIVE_UTMATHS_H
#define ARCHIVE_UTMATHS_H
#ifdef _MSC_VER
#pragma once
#endif

#include "core/maths/utmaths/maths.h"

namespace Arch
{

template <typename Vector>
struct SerialisationInfo<UtMaths::MVector<Vector> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_PRIMITIVE;}
	static const char* GetGUID() {return "UtMaths::MVector";}
};

template<typename Stream, typename Vector>
void Serialise(Stream& out, UtMaths::MVector<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		out & vector[i];
}

template<typename Stream, typename Vector>
void Deserialise(Stream& out, UtMaths::MVector<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		out & vector[i];
}

template <typename Vector>
struct SerialisationInfo<UtMaths::MNormal<Vector> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_PRIMITIVE;}
	static const char* GetGUID() {return "UtMaths::MNormal";}
};

template<typename Stream, typename Vector>
void Serialise(Stream& out, UtMaths::MNormal<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		out & vector[i];
}

template<typename Stream, typename Vector>
void Deserialise(Stream& out, UtMaths::MNormal<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		out & vector[i];
}

template <typename Vector>
struct SerialisationInfo<UtMaths::MPoint<Vector> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_PRIMITIVE;}
	static const char* GetGUID() {return "UtMaths::MPoint";}
};

template<typename Stream, typename Vector>
void Serialise(Stream& out, UtMaths::MPoint<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		out & vector[i];
}

template<typename Stream, typename Vector>
void Deserialise(Stream& out, UtMaths::MPoint<Vector>& vector)
{
	for (int i = 0; i < Vector::SIZE; i++)
		out & vector[i];
}

}

#endif

