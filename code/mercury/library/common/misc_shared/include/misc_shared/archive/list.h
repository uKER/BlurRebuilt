#ifndef ARCHIVE_LIST_H
#define ARCHIVE_LIST_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/list.h"

namespace Arch
{

template <class T, class P>
struct SerialisationInfo<Btl::List<T, P> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::List";}
};

template<typename Archive, class T, class P>
void Serialise(Archive& ar, Btl::List<T, P>& t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::List<T, P>::ConstIterator i = t.Begin(); i != t.End(); ++i)
		ar << *i;
}

template <class Archive, class T, class P>
void Deserialise(Archive& ar, Btl::List<T, P>& t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i=0; i<n; i++)
	{
		t.PushBack();
		ar >> t.Back();
	}
}

}

#endif
