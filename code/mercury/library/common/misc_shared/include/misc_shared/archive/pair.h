#ifndef ARCHIVE_PAIR_H
#define ARCHIVE_PAIR_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/utility/pair.h"

namespace Arch
{

template <typename First, typename Second>
struct SerialisationInfo<Btl::Pair<First, Second> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_PRIMITIVE;}
	static const char* GetGUID() {return "Btl::Pair";}
};

template <typename Archive, typename First, typename Second>
void Serialise(Archive& ar, Btl::Pair<First, Second>& t)
{
	ar << t.first;
	ar << t.second;
}

template <typename Archive, typename First, typename Second>
void Deserialise(Archive& ar, Btl::Pair<First, Second>& t)
{
	ar >> t.first;
	ar >> t.second;
}

}

#endif
