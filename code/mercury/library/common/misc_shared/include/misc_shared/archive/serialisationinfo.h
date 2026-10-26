#ifndef ARCHIVE_SERIALISATION_INFO_H
#define ARCHIVE_SERIALISATION_INFO_H
#ifdef _MSC_VER
#pragma once
#endif

#include "serialise.h"
#include "serialisationkey.h"

namespace Arch
{

///////////////////////////////////////////////////////////////////////////

enum SerialisationType
{
	SERIALISATION_TYPE_UNKNOWN, // only used when the type is not known
	SERIALISATION_TYPE_PRIMITIVE, // no version number
	SERIALISATION_TYPE_VERSION, // version number only (default)
	SERIALISATION_TYPE_POLYMORPHIC // version number and type information
};

// default serialisation info
template<typename T>
struct SerialisationInfo
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return 0;}
};

// macro to create a SerialisationInfo class
#define SERIALISATION_INFO(type_, version_, serialisation_type_, name_)					\
namespace Arch {																		\
	template<>																			\
	struct SerialisationInfo<type_>														\
	{																					\
		static U8 GetVersion() {return version_;}										\
		static SerialisationType GetSerialisationType() {return serialisation_type_;}	\
		static const char* GetGUID() {return name_;}									\
	};																					\
}

template <typename T>
const SerialisationKey FindKey()
{
	return SerialisationIdManager::FindKey(Arch::SerialisationInfo<T>::GetGUID());
}

/////////////////////////////////////////////////////////////////////////////////////
// Macros for serialisation methods.
// Note that these must be used outside of any namespace.

// Store a version number with the object.
#define SERIALISATION_VERSION(type_, version_, guid_) \
	SERIALISATION_INFO(type_, version_, Arch::SERIALISATION_TYPE_VERSION, guid_)

// No version number.
#define SERIALISATION_PRIMITIVE(type_, guid_) \
	SERIALISATION_INFO(type_, 0, Arch::SERIALISATION_TYPE_PRIMITIVE, guid_)

// Supports loading of derived types through a pointer to a base type.
#define SERIALISATION_POLYMORPHIC(type_, version_, guid_) \
	SERIALISATION_INFO(type_, version_, Arch::SERIALISATION_TYPE_POLYMORPHIC, guid_) \
	namespace Arch {template <> inline const Arch::SerialisationKey GetSerialisationKey<type_>(type_* t) {return t->GetSerialisationKey();}}

// The default is equivalent to SERIALISATION_VERSION(type, 0)
//
// The only change of serialisation method which maintains backward compatibility 
// is SERIALISATION_VERSION <-> SERIALISATION_POLYMORPHIC.
// i.e. types using SERIALISATION_PRIMITIVE must stay as SERIALISATION_PRIMITIVE,
// types not using SERIALISATION_PRIMITIVE cannot use it in the future.
/////////////////////////////////////////////////////////////////////////////////////////////

// Name-Value pair.
// A special type that is understood by the archive.
// XML archives use this to write the name of the object.
// Binary archives ignore the name and just write the value.
template <typename T>
struct Nvp
{
	typedef typename Btl::StripConst<T>::Result T_NC;

	Nvp(const char* n, T& v) : name(n), value(const_cast<T_NC&>(v)) {}

	const char* name;
	T_NC& value;
};

template <typename T>
Nvp<T> MakeNvp(const char* name, T& value)
{
	return Nvp<T>(name, value);
}

}

SERIALISATION_PRIMITIVE(Arch::SerialisationType, "SerialisationType")

SERIALISATION_PRIMITIVE(U8, "U8")
SERIALISATION_PRIMITIVE(S8, "S8")
SERIALISATION_PRIMITIVE(char, "char")
SERIALISATION_PRIMITIVE(U16, "U16")
SERIALISATION_PRIMITIVE(S16, "S16")
SERIALISATION_PRIMITIVE(U32, "U32")
SERIALISATION_PRIMITIVE(S32, "S32")
SERIALISATION_PRIMITIVE(F32, "F32")
SERIALISATION_PRIMITIVE(F64, "F64")
SERIALISATION_PRIMITIVE(U64, "U64")
SERIALISATION_PRIMITIVE(S64, "S64")

#endif

