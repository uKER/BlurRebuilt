#ifndef ARCHIVE_BITVECTOR_H
#define ARCHIVE_BITVECTOR_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/bitvector.h"

namespace Arch
{

template <typename Base>
struct SerialisationInfo<Btl::BitVectorTemplate<Base> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::BitVector";}
};

template <class Archive, typename Base>
void Serialise(Archive& ar, Btl::BitVectorTemplate<Base>& t)
{
	ar << (S32) t.Size();
	ar << t.GetBuffer();
}

template<typename Archive, typename Base>
void Deserialise(Archive& ar, Btl::BitVectorTemplate<Base>& t)
{
	S32 size;
	ar >> size;
	ar >> t.GetBuffer();
	t.SetNumBits(size);
}

}

#endif
