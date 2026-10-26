#ifndef ARCHIVE_VECTOR_H
#define ARCHIVE_VECTOR_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/vector.h"

namespace Arch
{

template <class T, class P>
struct SerialisationInfo<Btl::Vector<T, P> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::Vector";}
};

template <class Archive, class T, class P>
void Deserialise(Archive& ar, Btl::Vector<T, P> &t)
{
	S32 n;
	ar >> n;
	t.Resize(n);
	for (int i = 0; i < n; i++)
		ar >> t[i];
}

template <class Archive, class P>
void Deserialise(Archive& ar, Btl::Vector<U8, P> &t)
{
	S32 n;
	ar >> n;
	t.Resize(n);
	if (n > 0) ar.Read(&t[0], n);
}

template<typename Archive, class T, class P>
void Serialise(Archive& ar, Btl::Vector<T, P> &t)
{
	S32 n = t.Size();
	ar << MakeNvp("Size", n);
	for (int i = 0; i < n; i++)
		ar << t[i];
}

template <class Archive, class T, int N>
void Deserialise(Archive& ar, Btl::Vector<T, Btl::VectorParameters_Array<N> > &t)
{
	int n = t.Size();
	for (int i = 0; i < n; i++)
		ar >> t[i];
}

template <class Archive, class T, int N>
void Serialise(Archive& ar, Btl::Vector<T, Btl::VectorParameters_Array<N> > &t)
{
	int n = t.Size();
	for (int i = 0; i < n; i++)
		ar << t[i];
}

template <class Archive, class T, class A>
void Deserialise(Archive& ar, Btl::Vector<T, Btl::VectorParameters_FixedSize<A> > &t)
{
	S32 n;
	ar >> n;
	t.InitialiseSize(n);
	for (int i = 0; i < n; i++)
		ar >> t[i];
}

template <class Archive, class A>
void Deserialise(Archive& ar, Btl::Vector<U8, Btl::VectorParameters_FixedSize<A> > &t)
{
	S32 n;
	ar >> n;
	t.InitialiseSize(n);
	if (n > 0) ar.Read(&t[0], n);
}

}

#endif
