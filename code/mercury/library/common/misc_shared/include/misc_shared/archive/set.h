#ifndef ARCHIVE_SET_H
#define ARCHIVE_SET_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/set.h"
#include "btl/containers/multiset.h"
#include "btl/containers/chainedhashset.h"
#include "btl/containers/openhashset.h"

namespace Arch
{

template <typename Key, typename Allocator>
struct SerialisationInfo<Btl::Set<Key, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::Set";}
};

template<typename Archive, typename Key, typename Allocator>
void Serialise(Archive &ar, Btl::Set<Key, Allocator> &t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::Set<Key, Allocator>::ConstIterator i = t.Begin(); i != t.End(); ++i)
		ar << i->GetKey();
}

template <class Archive, typename Key, typename Allocator>
void Deserialise(Archive &ar, Btl::Set<Key, Allocator> &t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		Key key;
		ar >> key;
		t.Insert(key);
	}
}

///////////////////////////////////////////////////////////////////////

template <typename Key, typename Allocator>
struct SerialisationInfo<Btl::MultiSet<Key, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::MultiSet";}
};

template<typename Archive, typename Key, typename Allocator>
void Serialise(Archive &ar, Btl::MultiSet<Key, Allocator> &t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::MultiSet<Key, Allocator>::ConstIterator i = t.Begin(); i != t.End(); ++i)
		ar << i->GetKey();
}

template <class Archive, typename Key, typename Allocator>
void Deserialise(Archive &ar, Btl::MultiSet<Key, Allocator> &t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		Key data;
		ar >> data;
		t.Insert(data);
	}
}

////////////////////////////////////////////////////////////////////////

template <typename T, typename HashKeyOp, typename Allocator>
struct SerialisationInfo<Btl::ChainedHashSet<T, HashKeyOp, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::ChainedHashSet";}
};

template <typename Archive, typename T, typename HashKeyOp, typename Allocator>
void Deserialise(Archive& ar, Btl::ChainedHashSet<T, HashKeyOp, Allocator>& t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		T data;
		ar >> data;
		t.Insert(data);
	}
}

template <typename Archive, typename T, typename HashKeyOp, typename Allocator>
void Serialise(Archive& ar, Btl::ChainedHashSet<T, HashKeyOp, Allocator>& t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::ChainedHashSet<T, HashKeyOp, Allocator>::ConstIterator i = t.Begin(); i != t.End(); ++i)
		ar << i->GetKey();
}

////////////////////////////////////////////////////////////////////////

template <typename T, typename HashKeyOp, typename Allocator>
struct SerialisationInfo<Btl::ChainedHashMultiSet<T, HashKeyOp, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::ChainedHashMultiSet";}
};

template <typename Archive, typename T, typename HashKeyOp, typename Allocator>
void Serialise(Archive& ar, Btl::ChainedHashMultiSet<T, HashKeyOp, Allocator>& t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::ChainedHashMultiSet<T, HashKeyOp, Allocator>::ConstIterator i = t.Begin(); i != t.End(); ++i)
		ar << i->GetKey();
}

template <typename Archive, typename T, typename HashKeyOp, typename Allocator>
void Deserialise(Archive& ar, Btl::ChainedHashMultiSet<T, HashKeyOp, Allocator>& t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		T data;
		ar >> data;
		t.Insert(data);
	}
}

//////////////////////////////////////////////////////////////////////////////

template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
struct SerialisationInfo<Btl::OpenHashSet<Key, HashKeyOp, Probe, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::OpenHashSet";}
};

template <typename Archive, typename Key, typename HashKeyOp, typename Probe, typename Allocator>
void Serialise(Archive& ar, Btl::OpenHashSet<Key, HashKeyOp, Probe, Allocator>& t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::OpenHashSet<Key, HashKeyOp, Probe, Allocator>::ConstIterator i = t.Begin(); i != t.End(); ++i)
		ar << i->GetKey();
}

template <typename Archive, typename Key, typename HashKeyOp, typename Probe, typename Allocator>
void Deserialise(Archive& ar, Btl::OpenHashSet<Key, HashKeyOp, Probe, Allocator>& t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		Key data;
		ar >> data;
		t.Insert(data);
	}
}

}

#endif
